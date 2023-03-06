#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifndef ITER
#define ITER 10000
#endif

#ifndef MAXSIZE
#define MAXSIZE 1048576
#endif

#define NSEC_PER_SEC (1000000000)

static intmax_t difftimespec(struct timespec end, struct timespec begin);
pid_t pid;
static pid_t getpid_dummy() { return pid; }

int
main()
{
  unsigned char buf[MAXSIZE] = {0};

  struct timespec start = {0}, stop = {0};

  freopen("/dev/zero", "r", stdin);
  setvbuf(stdin, 0, _IOFBF, MAXSIZE);

  {
    volatile pid_t x;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITER; ++i) {
      x = getpid();
    }
    clock_gettime(CLOCK_MONOTONIC, &stop);
    intmax_t diff = difftimespec(stop, start);
    printf("getpid\t%jd\n", diff);
  }
  pid = getpid();
  {
    volatile pid_t x;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITER; ++i) {
      x = getpid_dummy();
    }
    clock_gettime(CLOCK_MONOTONIC, &stop);
    intmax_t diff = difftimespec(stop, start);
    printf("getpid_dummy\t%jd\n", diff);
  }
  putchar('\n');
  puts("size\tsyscall\tstdio");

  for (int size = 1; size <= MAXSIZE; size *= 2) {
    printf("%d", size);
    {
      clock_gettime(CLOCK_MONOTONIC, &start);
      for (int i = 0; i < ITER; ++i) {
        read(0, buf, size);
      }
      clock_gettime(CLOCK_MONOTONIC, &stop);
      intmax_t diff = difftimespec(stop, start);
      printf("\t%jd", ((uintmax_t)NSEC_PER_SEC * (uintmax_t)size * (uintmax_t)ITER * (uintmax_t)CHAR_BIT) /
                          ((uintmax_t)diff * (uintmax_t)1024 * (uintmax_t)1024));
    }
    {
      clock_gettime(CLOCK_MONOTONIC, &start);
      for (int i = 0; i < ITER; ++i) {
        fread(buf, 1, size, stdin);
      }
      clock_gettime(CLOCK_MONOTONIC, &stop);
      intmax_t diff = difftimespec(stop, start);
      printf("\t%jd", ((uintmax_t)NSEC_PER_SEC * (uintmax_t)size * (uintmax_t)ITER * (uintmax_t)CHAR_BIT) /
                          ((uintmax_t)diff * (uintmax_t)1024 * (uintmax_t) 1024));
    }
    putchar('\n');
  }
}

static intmax_t
difftimespec(struct timespec end, struct timespec begin)
{
  intmax_t result = ((intmax_t)end.tv_sec * (intmax_t)NSEC_PER_SEC + (intmax_t)end.tv_nsec) -
                    (((intmax_t)begin.tv_sec) * (intmax_t)NSEC_PER_SEC + (intmax_t)begin.tv_nsec);
  return result;
}

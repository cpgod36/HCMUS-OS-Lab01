#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXNUM 280

void primes(int left) __attribute__((noreturn));

// One stage of the pipeline sieve.
// The first number read from `left` is always a prime p. Every later
// number that is not a multiple of p is forwarded to the next stage,
// which is created only when the first such number shows up.
void
primes(int left)
{
  int p, n;
  int right[2];
  int has_next = 0;

  if(read(left, &p, sizeof(p)) != sizeof(p)){
    // left neighbour closed without sending anything: end of pipeline
    close(left);
    exit(0);
  }
  printf("prime %d\n", p);

  while(read(left, &n, sizeof(n)) == sizeof(n)){
    if(n % p == 0)
      continue;
    if(!has_next){
      if(pipe(right) < 0){
        fprintf(2, "primes: pipe failed\n");
        exit(1);
      }
      int pid = fork();
      if(pid < 0){
        fprintf(2, "primes: fork failed\n");
        exit(1);
      }
      if(pid == 0){
        // next stage only needs the read end of `right`
        close(left);
        close(right[1]);
        primes(right[0]);
      }
      close(right[0]);
      has_next = 1;
    }
    write(right[1], &n, sizeof(n));
  }

  // left neighbour is done (read returned 0)
  close(left);
  if(has_next){
    close(right[1]);   // lets the next stage see EOF
    wait(0);           // exit only after the rest of the pipeline exits
  }
  exit(0);
}

int
main(int argc, char *argv[])
{
  int p[2];
  int i, pid;

  if(pipe(p) < 0){
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }
  pid = fork();
  if(pid < 0){
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }
  if(pid == 0){
    close(p[1]);
    primes(p[0]);
  }

  // generator: feed 2..MAXNUM as raw 4-byte ints
  close(p[0]);
  for(i = 2; i <= MAXNUM; i++)
    write(p[1], &i, sizeof(i));
  close(p[1]);
  wait(0);
  exit(0);
}

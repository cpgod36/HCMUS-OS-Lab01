#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Parent sends one byte to child over p2c; child prints "ping" and
// sends the byte back over c2p; parent then prints "pong".
int
main(int argc, char *argv[])
{
  int p2c[2], c2p[2];   // [0] = read end, [1] = write end
  char b = 'x';
  int pid;

  if(pipe(p2c) < 0 || pipe(c2p) < 0){
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }

  pid = fork();
  if(pid < 0){
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    // child: reads from p2c, writes to c2p
    close(p2c[1]);
    close(c2p[0]);
    if(read(p2c[0], &b, 1) != 1){
      fprintf(2, "pingpong: child read failed\n");
      exit(1);
    }
    printf("%d: received ping\n", getpid());
    if(write(c2p[1], &b, 1) != 1){
      fprintf(2, "pingpong: child write failed\n");
      exit(1);
    }
    close(p2c[0]);
    close(c2p[1]);
    exit(0);
  }

  // parent: writes to p2c, reads from c2p
  close(p2c[0]);
  close(c2p[1]);
  if(write(p2c[1], &b, 1) != 1){
    fprintf(2, "pingpong: parent write failed\n");
    exit(1);
  }
  // blocks until the child has printed "ping" and replied
  if(read(c2p[0], &b, 1) != 1){
    fprintf(2, "pingpong: parent read failed\n");
    exit(1);
  }
  printf("%d: received pong\n", getpid());
  close(p2c[1]);
  close(c2p[0]);
  wait(0);
  exit(0);
}

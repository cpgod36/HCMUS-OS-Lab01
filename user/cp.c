#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char buf[512];

int
main(int argc, char *argv[])
{
  int src_fd, dst_fd;
  int n;

  if(argc != 3){
    fprintf(2, "usage: cp src dst\n");
    exit(1);
  }

  if((src_fd = open(argv[1], O_RDONLY)) < 0){
    fprintf(2, "cp: cannot open %s\n", argv[1]);
    exit(1);
  }

  if((dst_fd = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC)) < 0){
    fprintf(2, "cp: cannot open %s\n", argv[2]);
    close(src_fd);
    exit(1);
  }

  while((n = read(src_fd, buf, sizeof(buf))) > 0){
    if(write(dst_fd, buf, n) != n){
      fprintf(2, "cp: write error\n");
      close(src_fd);
      close(dst_fd);
      exit(1);
    }
  }

  if(n < 0){
    fprintf(2, "cp: read error\n");
    close(src_fd);
    close(dst_fd);
    exit(1);
  }

  close(src_fd);
  close(dst_fd);
  exit(0);
}

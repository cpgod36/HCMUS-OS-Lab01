#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char buf1[512];
char buf2[512];

int
readline(int fd, char *buf, int max)
{
  int i = 0;
  int n = 0;
  char c = 0;

  while(i < max - 1){
    n = read(fd, &c, 1);
    if(n < 0)
      return -1;
    if(n == 0){
      if(i == 0)
        return 0;
      break;
    }
    if(c == '\n')
      break;
    buf[i++] = c;
  }
  buf[i] = '\0';

  /* Neu dong dai hon max - 1, doc xa bo phan con lai den '\n' de khong lech so dong */
  if(c != '\n' && n > 0){
    while((n = read(fd, &c, 1)) > 0 && c != '\n')
      ;
    if(n < 0)
      return -1;
  }

  return 1;
}

int
main(int argc, char *argv[])
{
  int quiet = 0;
  char *file1 = 0;
  char *file2 = 0;
  int fd1, fd2;
  int line = 1;
  int r1, r2;
  int i;

  for(i = 1; i < argc; i++){
    if(strcmp(argv[i], "-q") == 0){
      quiet = 1;
    } else if(file1 == 0){
      file1 = argv[i];
    } else if(file2 == 0){
      file2 = argv[i];
    } else {
      fprintf(2, "usage: diff file1 file2 [-q]\n");
      exit(1);
    }
  }

  if(file1 == 0 || file2 == 0){
    fprintf(2, "usage: diff file1 file2 [-q]\n");
    exit(1);
  }

  if((fd1 = open(file1, O_RDONLY)) < 0){
    fprintf(2, "diff: cannot open %s\n", file1);
    exit(1);
  }

  if((fd2 = open(file2, O_RDONLY)) < 0){
    fprintf(2, "diff: cannot open %s\n", file2);
    close(fd1);
    exit(1);
  }

  while(1){
    r1 = readline(fd1, buf1, sizeof(buf1));
    r2 = readline(fd2, buf2, sizeof(buf2));

    if(r1 < 0 || r2 < 0){
      fprintf(2, "diff: read error\n");
      close(fd1);
      close(fd2);
      exit(1);
    }

    if(r1 == 0 && r2 == 0)
      break;

    if(r1 > 0 && r2 > 0){
      if(strcmp(buf1, buf2) != 0){
        if(quiet){
          printf("diff: files differ\n");
          close(fd1);
          close(fd2);
          exit(0);
        }
        printf("%s:%d: < %s\n", file1, line, buf1);
        printf("%s:%d: > %s\n", file2, line, buf2);
      }
    } else if(r1 == 0 && r2 > 0){
      if(quiet){
        printf("diff: files differ\n");
        close(fd1);
        close(fd2);
        exit(0);
      }
      printf("%s:%d: < EOF\n", file1, line);
      printf("%s:%d: > %s\n", file2, line, buf2);
    } else if(r1 > 0 && r2 == 0){
      if(quiet){
        printf("diff: files differ\n");
        close(fd1);
        close(fd2);
        exit(0);
      }
      printf("%s:%d: < %s\n", file1, line, buf1);
      printf("%s:%d: > EOF\n", file2, line);
    }

    line++;
  }

  close(fd1);
  close(fd2);
  exit(0);
}

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Return 1 if s is a non-empty string of decimal digits.
static int
isnumber(const char *s)
{
  if(*s == 0)
    return 0;
  for(; *s; s++)
    if(*s < '0' || *s > '9')
      return 0;
  return 1;
}

int
main(int argc, char *argv[])
{
  if(argc != 2){
    fprintf(2, "usage: sleep ticks\n");
    exit(1);
  }
  if(!isnumber(argv[1])){
    fprintf(2, "sleep: invalid number of ticks: %s\n", argv[1]);
    exit(1);
  }

  sleep(atoi(argv[1]));
  exit(0);
}

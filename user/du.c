#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

#define MAX_ENTRIES 128

// Stores basic metadata for each direct child entry in a directory.
struct entry_info {
  char name[DIRSIZ + 1];
  int is_dir;
  uint64 size;
};

// Safely joins a directory prefix and entry name: dst = prefix + "/" + name.
// Returns 0 on success, or -1 if the combined path is too long.
static int
join_path(char *dst, int max_len, const char *prefix, const char *name)
{
  int plen = strlen(prefix);
  int nlen = strlen(name);
  int need_slash = (plen > 0 && prefix[plen - 1] != '/');

  if(plen + need_slash + nlen + 1 > max_len)
    return -1;

  memmove(dst, prefix, plen);
  if(need_slash){
    dst[plen] = '/';
    plen++;
  }
  memmove(dst + plen, name, nlen);
  dst[plen + nlen] = '\0';
  return 0;
}

// Recursively calculates disk usage using post-order traversal (children first, parent last).
// - path: current directory path
// - is_root: 1 if this is the initial path from command line, 0 for subdirectories
// - flag_a: 1 to print all individual files alongside directories
// - flag_s: 1 to summarize and print only the root path
// Returns the total size in bytes of all files contained within this tree.
static uint64
du_dir(char *path, int is_root, int flag_a, int flag_s)
{
  int fd = open(path, O_RDONLY);
  if(fd < 0){
    fprintf(2, "du: cannot open %s\n", path);
    return 0;
  }

  struct stat st;
  if(fstat(fd, &st) < 0){
    fprintf(2, "du: cannot stat %s\n", path);
    close(fd);
    return 0;
  }

  if(st.type != T_DIR){
    close(fd);
    return 0;
  }

  // Allocate on heap to avoid overflowing xv6's 8KB user stack during recursion
  struct entry_info *entries = malloc(sizeof(struct entry_info) * MAX_ENTRIES);
  if(!entries){
    fprintf(2, "du: out of memory\n");
    close(fd);
    return 0;
  }

  int count = 0;
  struct dirent de;
  char subpath[512];

  // Step 1: Read all valid entries in the directory
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
      continue; // Unused or deleted slot

    // Crucial: always skip "." and ".." to avoid infinite loops
    if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
      continue;

    char name[DIRSIZ + 1];
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = '\0';

    if(join_path(subpath, sizeof(subpath), path, name) < 0)
      continue;

    struct stat subst;
    if(stat(subpath, &subst) < 0)
      continue;

    if(count < MAX_ENTRIES){
      strcpy(entries[count].name, name);
      entries[count].is_dir = (subst.type == T_DIR);
      entries[count].size = subst.size;
      count++;
    }
  }

  // Close directory descriptor immediately to stay well below NOFILE (16) limit
  close(fd);

  // Step 2: Post-order traversal (process children first)
  uint64 total_size = 0;

  for(int i = 0; i < count; i++){
    if(join_path(subpath, sizeof(subpath), path, entries[i].name) < 0)
      continue;

    if(entries[i].is_dir){
      // Recurse into subdirectory and aggregate its size
      uint64 child_dir_size = du_dir(subpath, 0, flag_a, flag_s);
      total_size += child_dir_size;
    } else {
      // Regular file: accumulate file size
      total_size += entries[i].size;

      // Print individual file when -a is set (and not in summary mode)
      if(flag_a && !flag_s){
        printf("%ld\t%s\n", entries[i].size, subpath);
      }
    }
  }

  // Step 3: Print current directory size after all children have finished
  // In summary mode (-s), only print the root directory
  if(!flag_s || is_root){
    printf("%ld\t%s\n", total_size, path);
  }

  free(entries);
  return total_size;
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  int flag_a = 0;
  int flag_s = 0;

  // Flexible argument parsing (options and path can be in any position)
  for(int i = 1; i < argc; i++){
    if(strcmp(argv[i], "-a") == 0){
      flag_a = 1;
    } else if(strcmp(argv[i], "-s") == 0){
      flag_s = 1;
    } else if(argv[i][0] == '-'){
      fprintf(2, "du: invalid option: %s\n", argv[i]);
      fprintf(2, "usage: du [path] [-a] [-s]\n");
      exit(1);
    } else {
      path = argv[i];
    }
  }

  struct stat st;
  if(stat(path, &st) < 0){
    fprintf(2, "du: cannot stat %s\n", path);
    exit(1);
  }

  // If the target is a regular file, simply print its size
  if(st.type != T_DIR){
    printf("%ld\t%s\n", st.size, path);
    exit(0);
  }

  // Start calculating from root
  du_dir(path, 1, flag_a, flag_s);

  exit(0);
}

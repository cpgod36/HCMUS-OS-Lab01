#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

#define MAX_ENTRIES 128

// Stores basic metadata for each direct child entry in a directory.
struct entry_info {
  char name[DIRSIZ + 1];
  int is_dir;
  uint64 size;
};

// Global shared path buffer to save stack space.
// xv6 user stack is only 8KB (or 4KB); keeping this global avoids stack overflow on deep trees.
static char path_buf[MAXPATH];

// Temporary buffers used only during the non-recursive directory scan phase.
// Moving them to static storage reduces each recursive stack frame to ~48 bytes.
static struct stat g_st;
static struct stat g_subst;
static struct dirent g_de;
static char g_name[DIRSIZ + 1];

// Global settings to save stack space during recursion.
static int g_flag_a = 0;
static int g_flag_s = 0;

// Helper to safely append a child name to path_buf.
// Returns the previous length on success, or -1 if the path would exceed MAXPATH.
static int
append_path(const char *name)
{
  int old_len = strlen(path_buf);
  int nlen = strlen(name);
  int need_slash = (old_len > 0 && path_buf[old_len - 1] != '/');

  if(old_len + need_slash + nlen + 1 > MAXPATH)
    return -1;

  if(need_slash){
    path_buf[old_len] = '/';
    old_len++;
  }
  memmove(path_buf + old_len, name, nlen);
  path_buf[old_len + nlen] = '\0';
  return old_len - need_slash;
}

// Recursively calculates disk usage using post-order traversal (children first, parent last).
// - is_root: 1 if this is the initial path from command line, 0 for subdirectories
// Returns the total size in bytes of all files contained within this tree.
static uint64
du_dir(int is_root)
{
  int fd = open(path_buf, O_RDONLY);
  if(fd < 0){
    fprintf(2, "du: cannot open %s\n", path_buf);
    return 0;
  }

  if(fstat(fd, &g_st) < 0){
    fprintf(2, "du: cannot stat %s\n", path_buf);
    close(fd);
    return 0;
  }

  if(g_st.type != T_DIR){
    close(fd);
    return 0;
  }

  // Allocate on heap to avoid overflowing xv6's limited user stack during recursion
  struct entry_info *entries = malloc(sizeof(struct entry_info) * MAX_ENTRIES);
  if(!entries){
    fprintf(2, "du: out of memory\n");
    close(fd);
    return 0;
  }

  int count = 0;
  int warned_overflow = 0;

  // Step 1: Read all valid entries in the directory
  while(read(fd, &g_de, sizeof(g_de)) == sizeof(g_de)){
    if(g_de.inum == 0)
      continue; // Unused or deleted slot

    // Crucial: always skip "." and ".." to avoid infinite loops
    if(strcmp(g_de.name, ".") == 0 || strcmp(g_de.name, "..") == 0)
      continue;

    memmove(g_name, g_de.name, DIRSIZ);
    g_name[DIRSIZ] = '\0';

    int old_len = append_path(g_name);
    if(old_len < 0){
      fprintf(2, "du: path too long\n");
      continue;
    }

    int stat_res = stat(path_buf, &g_subst);
    path_buf[old_len] = '\0'; // Restore parent path

    if(stat_res < 0)
      continue;

    if(count < MAX_ENTRIES){
      strcpy(entries[count].name, g_name);
      entries[count].is_dir = (g_subst.type == T_DIR);
      entries[count].size = g_subst.size;
      count++;
    } else if(!warned_overflow){
      fprintf(2, "du: directory has > %d entries, truncating\n", MAX_ENTRIES);
      warned_overflow = 1;
    }
  }

  // Close directory descriptor immediately to stay well below NOFILE (16) limit
  close(fd);

  // Step 2: Post-order traversal (process children first)
  uint64 total_size = 0;

  for(int i = 0; i < count; i++){
    int old_len = append_path(entries[i].name);
    if(old_len < 0)
      continue;

    if(entries[i].is_dir){
      // Recurse into subdirectory and aggregate its size
      uint64 child_dir_size = du_dir(0);
      total_size += child_dir_size;
    } else {
      // Regular file: accumulate file size
      total_size += entries[i].size;

      // Print individual file when -a is set (and not in summary mode)
      if(g_flag_a && !g_flag_s){
        printf("%ld\t%s\n", entries[i].size, path_buf);
      }
    }

    path_buf[old_len] = '\0'; // Restore parent path
  }

  // Step 3: Print current directory size after all children have finished
  // In summary mode (-s), only print the root directory
  if(!g_flag_s || is_root){
    printf("%ld\t%s\n", total_size, path_buf);
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

  if(strlen(path) >= MAXPATH){
    fprintf(2, "du: path too long: %s\n", path);
    exit(1);
  }
  strcpy(path_buf, path);

  struct stat st;
  if(stat(path_buf, &st) < 0){
    fprintf(2, "du: cannot stat %s\n", path_buf);
    exit(1);
  }

  // If the target is a regular file, simply print its size
  if(st.type != T_DIR){
    printf("%ld\t%s\n", st.size, path_buf);
    exit(0);
  }

  // Set global flags and start calculating from root
  g_flag_a = flag_a;
  g_flag_s = flag_s;
  du_dir(1);

  exit(0);
}

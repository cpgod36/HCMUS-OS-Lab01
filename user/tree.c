#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

#define MAX_ENTRIES 128
#define MAX_DEPTH MAXPATH

// Stores basic metadata for each direct child entry in a directory.
struct entry_info {
  char name[DIRSIZ + 1];
  int is_dir;
};

// Global shared path buffer and ancestor status array to save stack space.
// xv6 user stack is only 8KB (or 4KB); keeping these global avoids stack overflow on deep trees.
static char path_buf[MAXPATH];
static int is_last[MAX_DEPTH];

// Temporary buffers used only during the non-recursive directory scan phase.
// Moving them to static storage reduces each recursive stack frame to ~48 bytes.
static struct stat g_st;
static struct stat g_subst;
static struct dirent g_de;
static char g_name[DIRSIZ + 1];

// Global settings to save stack space during recursion.
static int g_max_depth = 999999;
static int g_only_dir = 0;

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

// Recursively walks and displays the directory tree.
// - depth: current depth level (root is 0, direct children are 1, etc.)
static void
tree_walk(int depth)
{
  if(depth >= MAX_DEPTH){
    fprintf(2, "tree: recursion depth limit reached\n");
    return;
  }

  int fd = open(path_buf, O_RDONLY);
  if(fd < 0){
    fprintf(2, "tree: cannot open %s\n", path_buf);
    return;
  }

  if(fstat(fd, &g_st) < 0){
    fprintf(2, "tree: cannot stat %s\n", path_buf);
    close(fd);
    return;
  }

  if(g_st.type != T_DIR){
    close(fd);
    return;
  }

  // Allocate on the heap to avoid overflowing xv6's limited user stack
  struct entry_info *entries = malloc(sizeof(struct entry_info) * MAX_ENTRIES);
  if(!entries){
    fprintf(2, "tree: out of memory\n");
    close(fd);
    return;
  }

  int count = 0;
  int warned_overflow = 0;

  // Step 1: Read all eligible directory entries into our temporary array
  while(read(fd, &g_de, sizeof(g_de)) == sizeof(g_de)){
    if(g_de.inum == 0)
      continue; // Unused or deleted directory slot

    // Crucial: always skip "." and ".." to avoid infinite recursion
    if(strcmp(g_de.name, ".") == 0 || strcmp(g_de.name, "..") == 0)
      continue;

    memmove(g_name, g_de.name, DIRSIZ);
    g_name[DIRSIZ] = '\0';

    int old_len = append_path(g_name);
    if(old_len < 0){
      fprintf(2, "tree: path too long\n");
      continue;
    }

    int stat_res = stat(path_buf, &g_subst);
    path_buf[old_len] = '\0'; // Restore parent path

    if(stat_res < 0)
      continue;

    // When -d is active, ignore non-directory entries
    if(g_only_dir && g_subst.type != T_DIR)
      continue;

    if(count < MAX_ENTRIES){
      strcpy(entries[count].name, g_name);
      entries[count].is_dir = (g_subst.type == T_DIR);
      count++;
    } else if(!warned_overflow){
      fprintf(2, "tree: directory has > %d entries, truncating\n", MAX_ENTRIES);
      warned_overflow = 1;
    }
  }

  // Close directory descriptor right away so we never run out of file descriptors (NOFILE = 16)
  close(fd);

  // Step 2: Print each entry with proper tree branch styling
  for(int i = 0; i < count; i++){
    int is_last_item = (i == count - 1);

    // Print guiding vertical bars or indentation for ancestor levels
    for(int d = 1; d < depth; d++){
      if(is_last[d])
        printf("    ");
      else
        printf("│   ");
    }

    // Print branch symbol: └── for the last item, ├── for the rest
    if(is_last_item){
      printf("└── %s\n", entries[i].name);
      is_last[depth] = 1;
    } else {
      printf("├── %s\n", entries[i].name);
      is_last[depth] = 0;
    }

    // Recurse into subdirectories if we haven't reached the depth limit
    if(entries[i].is_dir && depth < g_max_depth){
      int old_len = append_path(entries[i].name);
      if(old_len >= 0){
        tree_walk(depth + 1);
        path_buf[old_len] = '\0'; // Restore parent path
      }
    }
  }

  free(entries);
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  int max_depth = 999999;
  int only_dir = 0;

  // Flexible argument parsing (options and path can be in any position)
  for(int i = 1; i < argc; i++){
    if(strcmp(argv[i], "-d") == 0){
      only_dir = 1;
    } else if(strcmp(argv[i], "-L") == 0){
      if(i + 1 >= argc){
        fprintf(2, "tree: option requires an argument -- L\n");
        fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
        exit(1);
      }
      i++;
      max_depth = atoi(argv[i]);
      if(max_depth <= 0){
        fprintf(2, "tree: invalid depth: %s\n", argv[i]);
        exit(1);
      }
    } else if(argv[i][0] == '-'){
      fprintf(2, "tree: invalid option: %s\n", argv[i]);
      fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
      exit(1);
    } else {
      path = argv[i];
    }
  }

  if(strlen(path) >= MAXPATH){
    fprintf(2, "tree: path too long: %s\n", path);
    exit(1);
  }
  strcpy(path_buf, path);

  struct stat st;
  if(stat(path_buf, &st) < 0){
    fprintf(2, "tree: cannot stat %s\n", path_buf);
    exit(1);
  }

  // Print the root path name
  printf("%s\n", path_buf);

  // If path is a directory and depth allows, start the recursive walk
  if(st.type == T_DIR && max_depth > 0){
    g_max_depth = max_depth;
    g_only_dir = only_dir;
    memset(is_last, 0, sizeof(is_last));
    tree_walk(1);
  }

  exit(0);
}

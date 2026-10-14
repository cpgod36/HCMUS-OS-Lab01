#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

#define MAX_ENTRIES 128
#define MAX_DEPTH 32

// Stores basic metadata for each direct child entry in a directory.
struct entry_info {
  char name[DIRSIZ + 1];
  int is_dir;
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

// Recursively walks and displays the directory tree.
// - path: current directory path
// - depth: current depth level (root is 0, direct children are 1, etc.)
// - max_depth: depth limit from -L (unlimited if very large)
// - only_dir: 1 if -d flag is set (skip regular files), 0 otherwise
// - is_last: tracks whether ancestors at each depth were the last child
static void
tree_walk(char *path, int depth, int max_depth, int only_dir, int *is_last)
{
  int fd = open(path, O_RDONLY);
  if(fd < 0){
    fprintf(2, "tree: cannot open %s\n", path);
    return;
  }

  struct stat st;
  if(fstat(fd, &st) < 0){
    fprintf(2, "tree: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if(st.type != T_DIR){
    close(fd);
    return;
  }

  // Allocate on the heap to avoid overflowing xv6's limited 8KB user stack
  struct entry_info *entries = malloc(sizeof(struct entry_info) * MAX_ENTRIES);
  if(!entries){
    fprintf(2, "tree: out of memory\n");
    close(fd);
    return;
  }

  int count = 0;
  struct dirent de;
  char subpath[512];

  // Step 1: Read all eligible directory entries into our temporary array
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
      continue; // Unused or deleted directory slot

    // Crucial: always skip "." and ".." to avoid infinite recursion
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

    // When -d is active, ignore non-directory entries
    if(only_dir && subst.type != T_DIR)
      continue;

    if(count < MAX_ENTRIES){
      strcpy(entries[count].name, name);
      entries[count].is_dir = (subst.type == T_DIR);
      count++;
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
    if(entries[i].is_dir && depth < max_depth){
      if(join_path(subpath, sizeof(subpath), path, entries[i].name) == 0){
        tree_walk(subpath, depth + 1, max_depth, only_dir, is_last);
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

  struct stat st;
  if(stat(path, &st) < 0){
    fprintf(2, "tree: cannot stat %s\n", path);
    exit(1);
  }

  // Print the root path name
  printf("%s\n", path);

  // If path is a directory and depth allows, start the recursive walk
  if(st.type == T_DIR && max_depth > 0){
    int is_last[MAX_DEPTH];
    memset(is_last, 0, sizeof(is_last));
    tree_walk(path, 1, max_depth, only_dir, is_last);
  }

  exit(0);
}

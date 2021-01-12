#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "tlpi_hdr.h"

static void usageError(const char* progName){
  fprintf(stderr,"Usage:%s [-cx]name size [octal-perms]\n",progName);
  fprintf(stderr,"    -c    Create shared memory (O_CREAT)\n");
  fprintf(stderr,"    -x    Create exclusively(O_EXCL)\n");
  exit(EXIT_FAILURE);
}

int main(int argc,char* argv[]){
    int flags = O_RDWR;
    int opt;
    while((opt = getopt(argc,argv,"cx")) != -1){
      switch(opt){
        case 'c': 
          flags |= O_CREAT;
          break;
        case 'x':
          flags |= O_EXCL;
          break;
        default:
          usageErr(argv[0]);
      }
    }
    if(optind +1 >= argc)
      usageErr(argv[0]);

    size_t size = getLong(argv[optind]+1,GN_ANY_BASE,"size");
    mode_t perms = (argc <= optind+2)?(S_IRUSR | S_IWUSR) : getLong(argv[optind+2],GN_BASE_8,"octal-perms");

    //create shared memory and set its size 
    int fd = shm_open(argv[optind],flags,perms);
    if(fd == -1)
      errExit("shm_open");
    if(ftruncate(fd,size) == -1)
      errExit("ftruncate");
    //Map it 
    void* addr = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED)
      errExit("mmap");
    exit(EXIT_SUCCESS);
}

#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/procinfo.h"

int main(int argc, char *argv[]) {
  struct procinfo info;
  
  // getpid() trả về id của chính tiến trình proctest này
  if (procinfo(getpid(), &info) == 0) {
    printf("Process: %s\n", info.name);
    printf("PID: %d, PPID: %d\n", info.pid, info.ppid);
    printf("State: %d\n", info.state);
    printf("Memory: %d bytes\n", (int)info.sz); 
  } else {
    printf("procinfo failed!\n");
  }
  
  exit(0);
}

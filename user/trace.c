#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int i;
  char *nargv[MAXARG];

  // Kiểm tra xem user có nhập đủ tham số không (trace <mask> <command>)
  if(argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')){
    fprintf(2, "Usage: %s mask command\n", argv[0]);
    exit(1);
  }

  // Gọi system call sys_trace trong kernel
  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "%s: trace failed\n", argv[0]);
    exit(1);
  }

  // Lấy các tham số của lệnh command phía sau để chạy
  for(i = 2; i < argc && i < MAXARG; i++){
    nargv[i-2] = argv[i];
  }
  nargv[i-2] = 0;

  // Thực thi command
  exec(nargv[0], nargv);
  
  // Nếu exec thất bại (lệnh không tồn tại) thì in ra lỗi
  printf("exec %s failed\n", nargv[0]);
  exit(1);
}
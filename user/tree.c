#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

//tách path lấy mỗi tên file
char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  if(strlen(p) >= DIRSIZ)
    return p;

  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), 0, DIRSIZ - strlen(p));
  return buf;
}

//in khoảng trống dựa trên cấp dir
void indent(int depth){
  for(int i = 0; i < depth; i++)
    printf("  ");
}

void tree(char *path, int depth){
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  //mở file
  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "tree: cannot open %s\n", path);
    return;
  }

  //lấy metadata
  if(fstat(fd, &st) < 0){
    fprintf(2, "tree: cannot stat %s\n", path);
    close(fd);
    return;
  }

  //Từ metadata đã trích xuất, check type
  //case file
  if(st.type == T_FILE){
    indent(depth);
    printf("%s\n", fmtname(path));
  }

  //case dir
  if(st.type == T_DIR){
    indent(depth);
    printf("%s/\n", fmtname(path));

    //Ktra đường dẫn
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("tree: path too long\n");
      close(fd);
      return;
    }

    //tạo prefix
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    //Đọc từng file trong dir
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;

        //skip "." và ".."
      if(strcmp(de.name,".")==0 || strcmp(de.name,"..")==0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      //recursion
      tree(buf, depth+1);
    }
  }

  close(fd);
}

int main(int argc, char *argv[]){
  if(argc < 2)
    tree(".",0);
  else
    tree(argv[1],0);

  exit(0);
}
#include "kernel/types.h"   // Kiểu dữ liệu cơ bản của xv6
#include "kernel/stat.h"    // Cấu trúc stat (metadata file)
#include "user/user.h"      // Thư viện hàm user-space (printf, malloc...)
#include "kernel/fs.h"      // Định nghĩa filesystem (DIRSIZ, dirent...)
#include "kernel/fcntl.h"   // Hằng số open (O_RDONLY...)

// Tách path lấy mỗi tên file
char* fmtname(char *path) {
  char *p;

  // Duyệt ngược chuỗi để tìm dấu '/' cuối
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++; // Lấy phần sau '/'

  return p; // Trả về tên file/thư mục
}

// In khoảng trắng theo độ sâu cây thư mục
void indent(int depth){
  int i;
  for(i = 0; i < depth; i++)
    printf("  "); // Mỗi cấp thụt 2 space
}

void tree(char *path, int depth){
  char *buf = malloc(512);  // Cấp phát buffer chứa path con
  char *p;
  int fd;
  struct dirent de;         // Entry của thư mục
  struct stat st;           // Metadata file

  if((fd = open(path, O_RDONLY)) < 0){ // Mở file/thư mục
    fprintf(2, "tree: cannot open %s\n", path);
    free(buf); // Giải phóng bộ nhớ
    return;
  }

  if(fstat(fd, &st) < 0){ // Lấy metadata
    fprintf(2, "tree: cannot stat %s\n", path);
    close(fd);
    free(buf);
    return;
  }

  if(st.type == T_FILE){ // Nếu là file thường
    indent(depth);
    printf("%s\n", fmtname(path)); // In tên file
  }

  if(st.type == T_DIR){ // Nếu là thư mục
    indent(depth);
    printf("%s/\n", fmtname(path)); // In tên thư mục

    if(strlen(path) + 1 + DIRSIZ + 1 > 512){ // Kiểm tra tràn buffer
      printf("tree: path too long\n");
      close(fd);
      free(buf);
      return;
    }

    strcpy(buf, path); // Copy path cha
    p = buf + strlen(buf); // Con trỏ cuối chuỗi
    *p++ = '/'; // Thêm dấu '/'

    while(read(fd, &de, sizeof(de)) == sizeof(de)){ // Đọc từng entry
      if(de.inum == 0)
        continue; // Bỏ entry rỗng

      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue; // Bỏ . và ..

      memmove(p, de.name, DIRSIZ); // Nối tên file vào path
      p[DIRSIZ] = 0; // Kết thúc chuỗi

      tree(buf, depth + 1); // Đệ quy duyệt thư mục con
    }
  }

  close(fd); // Đóng file descriptor
  free(buf); // Giải phóng bộ nhớ
}

int main(int argc, char *argv[]){
  if(argc < 2)
    tree(".", 0); // Không có argument -> duyệt thư mục hiện tại
  else
    tree(argv[1], 0); // Duyệt thư mục được truyền vào

  exit(0); // Kết thúc chương trình
}
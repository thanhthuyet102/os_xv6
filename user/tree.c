#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Hàm đệ quy duyệt cây thư mục
// path: Đường dẫn đầy đủ để mở file/thư mục (VD: a/aa/b)
// name_to_print: Tên ngắn gọn chỉ để in ra màn hình (VD: b)
// depth: Độ sâu hiện tại để thụt lề (0 là gốc, 1 là con, 2 là cháu...)
void tree(char *path, char *name_to_print, int depth) {
    char buf[512];      // Bộ đệm chứa đường dẫn con sẽ được tạo ra
    char *p;            // Con trỏ dùng để nối chuỗi
    int fd;             // File descriptor
    struct dirent de;   // Cấu trúc lưu thông tin 1 dòng trong thư mục
    struct stat st;     // Cấu trúc lưu trạng thái (loại file, dung lượng...)

    // 1. MỞ ĐƯỜNG DẪN: Lấy fd để thao tác
    if((fd = open(path, 0)) < 0){
        // Yêu cầu: Print error and exit/return nếu không mở được
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    // 2. LẤY THÔNG TIN (stat): Xem fd này là FILE hay THƯ MỤC
    if(fstat(fd, &st) < 0){
        // Yêu cầu: Print error but continue nếu stat lỗi
        fprintf(2, "tree: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // 3. IN LÙI LỀ (Indentation): 2 khoảng trắng cho mỗi level (depth)
    for(int i = 0; i < depth; i++){
        printf("  "); 
    }

    // 4. KIỂM TRA LOẠI FILE VÀ XỬ LÝ
    switch(st.type){
        // TRƯỜNG HỢP A: LÀ FILE THƯỜNG (Điều kiện dừng đệ quy)
        case T_FILE:
            printf("%s\n", name_to_print); // Chỉ in tên file, không có dấu /
            break;

        // TRƯỜNG HỢP B: LÀ THƯ MỤC (Tiếp tục đệ quy)
        case T_DIR:
            printf("%s/\n", name_to_print); // Yêu cầu: Show directories with trailing /

            // Kiểm tra xem đường dẫn sắp nối có vượt quá bộ đệm buf (512 bytes) không
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
                printf("tree: path too long\n");
                break;
            }

            // Chuẩn bị đường dẫn cha vào buf: copy path vào buf, thêm dấu '/'
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/'; // p bây giờ trỏ đến ngay sau dấu '/', sẵn sàng ghép tên con

            // Đọc từng entry (file/thư mục con) bên trong thư mục hiện tại
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                // Bỏ qua các entry trống (đã bị xóa)
                if(de.inum == 0) continue;

                // Yêu cầu: BẮT BUỘC bỏ qua "." và ".." để tránh ĐỆ QUY VÔ HẠN
                if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;

                // Ghép tên file con (de.name) vào sau dấu '/' trong buf
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0; // Thêm ký tự kết thúc chuỗi null-terminator

                // GỌI ĐỆ QUY: 
                // - buf lúc này là đường dẫn hoàn chỉnh (VD: a/aa)
                // - de.name là tên hiển thị (VD: aa)
                // - depth + 1 để con lùi lề sâu hơn cha
                tree(buf, de.name, depth + 1);
            }
            break;
    }

    // 5. ĐÓNG FILE DESCRIPTOR: Rất quan trọng trong đệ quy để không bị tràn fd
    close(fd);
}

int main(int argc, char *argv[]) {
    // Yêu cầu: Nếu không truyền tham số, mặc định dùng thư mục hiện tại "."
    if(argc == 1) {
        tree(".", ".", 0);
    } 
    // Yêu cầu: Nhận 1 tham số là tên thư mục
    else if (argc == 2) {
        tree(argv[1], argv[1], 0);
    } 
    // Nếu truyền sai cú pháp
    else {
        fprintf(2, "Usage: tree [directory]\n");
        exit(1);
    }

    exit(0);
}
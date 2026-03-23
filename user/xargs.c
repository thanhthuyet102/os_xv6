#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/string.h"
#include "kernel/param.h" // Khai báo MAXARG

int main(int argc, char *argv[]) {
    // Kiểm tra đầu vào tối thiểu
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command>...\n");
        exit(1);
    }

    // 1. Khởi tạo danh sách đối số (xargv)
    char *xargv[MAXARG];
    int xargc = 0;
    int start_idx = 1;

    // Logic kiểm tra và bỏ qua cờ "-n 1"
    if (argc >= 3 && strcmp(argv[1], "-n") == 0) {
        start_idx = 3;
    }

    // Đảm bảo có lệnh cần thực thi sau khi bỏ qua cờ
    if (start_idx >= argc) {
        exit(1);
    }

    // Sao chép các đối số từ argv sang xargv
    for (int i = start_idx; i < argc; i++) {
        xargv[xargc] = argv[i];
        xargc++;
    }

    char buf[512];
    int buf_idx = 0;
    char c;

    // 2. Đọc dữ liệu: Sử dụng read(0, &c, 1)
    while (read(0, &c, 1) == 1) {
        if (c == '\n') {
            // 3. Xử lý dòng lệnh: chốt chuỗi và đưa vào xargv
            buf[buf_idx] = '\0';
            
            xargv[xargc] = buf;      // Nối đối số
            xargv[xargc + 1] = 0;    // Thêm null (0) chốt mảng

            // 4. Thực thi lệnh
            int pid = fork();
            if (pid == 0) { 
                exec(xargv[0], xargv);
                // Xử lý lỗi thực thi (Exec failure)
                fprintf(2, "exec %s failed\n", xargv[0]);
                exit(1);
            } else if (pid > 0) { 
                // Vấn đề đồng bộ bộ đệm: Cha đợi con chạy xong mới đọc tiếp
                wait(0);
            } else {
                fprintf(2, "fork failed\n");
                exit(1);
            }

            buf_idx = 0; // Reset bộ đệm cho dòng mới
        } else {
            // Kiểm soát an toàn bộ nhớ: chặn tràn bộ đệm
            if (buf_idx < sizeof(buf) - 1) {
                buf[buf_idx] = c;
                buf_idx++;
            }
        }
    }

    // Dòng cuối cùng không có ký tự ngắt dòng (\n)
    if (buf_idx > 0) {
        buf[buf_idx] = '\0';
        xargv[xargc] = buf;
        xargv[xargc + 1] = 0;

        int pid = fork();
        if (pid == 0) {
            exec(xargv[0], xargv);
            // Xử lý lỗi thực thi (Exec failure)
            fprintf(2, "exec %s failed\n", xargv[0]);
            exit(1);
        } else if (pid > 0) {
            wait(0);
        }
    }

    exit(0);
}
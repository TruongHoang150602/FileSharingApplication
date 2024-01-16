# Author:

- Hoàng Vân Trường
- Nguyễn Trọng Quang
- Nguyễn Giang Nam

# Subject: Xây dựng ứng dụng chia sẻ file

# Programming language: C

# Library use in this program: pthread with some basic library for tcp/ip communication

# Problem:

- Đăng ký tài khoản mới
- Đăng nhập vào tài khoản để sử dụng
- Cho phép người dùng tạo nhóm chia sẻ hoặc xin tham gia một nhóm. Người tạo nhóm có quyền trưởng nhóm
- Mỗi nhóm có một thư mục riêng, chứa các file được chia sẻ trong nhóm đó
- Cho thành viên bất kỳ trong nhóm cũng có thể upload file, tạo thư mục con trong thư mục của nhóm đó
- Chỉ có trưởng nhóm có quyền xóa file, thư mục con, đổi tên file, thư mục
- Upload file.
- Download file.

# Installation:

- Cài đặt thư viện [cjSON](https://github.com/DaveGamble/cJSON.git) theo hướng dẫn
- Clone source code về máy

```bash
git clone https://github.com/TruongHoang150602/FileSharingApplication.git
```

- Chạy lệnh sau để chạy server trên cổng 8080

```bash
cd FileSharingApplication
cd server
make clean
make
./server 8080
```

- Chạy lệnh sau để chạy client

```bash
cd ..
cd client
make clean
make
./client 172.18.0.1 8080
```

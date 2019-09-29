# SimpleShell
Chương trình shell đơn giản cho Linux. Hỗ trợ pipe, IO redirect, thực thi lệnh song song, tự động lưu lại lịch sử

## Cài đặt
  - Để build lại, hãy cài đặt cmake:
  ```bash
  sudo apt install cmake
  ```
  - Sau đó, chuyển đến thư mục build và nhập lệnh:
  ```bash
  cmake ..
  make
  ```
  - Hoặc chạy chương trình đã build sẵn trong thư mục build:
  ```bash
  ./shell.o
  ```
  
## Giới thiệu các tính năng
**1. Lưu lại lịch sử:**
  - Nhập '!!' để thực thi lại câu lệnh vừa mới thực thi:
  ```bash
  !!
  ```
**2. Thực thi lệnh song song:**
  - Thêm '&' ở cuối lệnh để không phải chờ lệnh đó kết thúc:
  ```bash
  ping -i 2 -w 20 www.google.com.vn &
  ```
**3. IO redirect:**
  - Hỗ trợ redirect input, output và error:
  ```bash
  ls -l > output.txt
  cat < output.txt
  %@^&#*&$ 2> error.txt
  ```
**4. Pipe:**
  - Hỗ trợ pipe, nhập '|' giữa hai lệnh để giao tiếp với nhau:
  ```bash
  cat text.txt | less
  ```
  
## Giấy phép
  - Đây là chương trình đơn giản phục vụ học tập
  - Không vì mục đích kính doanh

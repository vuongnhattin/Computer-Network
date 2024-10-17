# Ứng dụng điều khiển máy tính từ xa.
## 1. Giới thiệu
Ứng dụng có chức năng giống **TeamViewer**, có thể dùng một máy để điều khiển màn hình máy khác thông qua chuột, bàn phím.
## 2. Demo video
https://youtu.be/g3SVFKiqwxk
## 3. Công nghệ sử dụng sử dụng
Ứng dụng được viết bằng **C++** với các thư viện sau:
- **SDL2** và **DearImGui** cho giao diện.
- **OpenCV** để xử lý ảnh (nén, giải nén,...).
- **WinSock2** để thiết lập socket phục vụ cho truyền, nhận ảnh.
- Dùng multi-threading để xử lý tín hiệu ảnh, chuột, bàn phím song song.
## 4. Chức năng
- Như trong video demo.

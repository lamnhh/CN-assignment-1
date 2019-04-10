# I. Các thông tin chung

### 1. Môi trường lập trình:

Microsoft Visual Studio Community 2013

### 2. Các framework và thư viện:

- Sử dụng MFC để làm giao diện WinForm.
- Sử dụng winsock.h để thực hiện các thao tác liên quan đến socket.

# II. Cấu trúc của mỗi packet

```
{
  action: char[20],
  content: char[1000]
}
```

Trong đó:

- `action` là tên thao tác cần thực hiện trên packet này.
- `content` là các thông tin cần thiết để có thể thực hiện thao tác đó.

# III. Kịch bản giao tiếp từ phía Client lên Server

## 1. Chat chung giữa tất cả các user.

- Gửi `action = "message-all"`: `content` sẽ lưu nội dung của dòng chat mới.

- Nhận `action = "message-all-new"`: thêm `content` vào list chat hiện tại; hoặc update số tin nhắn chưa đọc.

## 2. Chat riêng giữa 2 user.

- Gửi `message-one`:

  `content` sẽ lưu người nhận và nội dung của dòng chat. Cụ thể hơn, `content` sẽ là dãy char[] biểu diễn của một struct như sau:

  ```
  {
    receiver: char[20],
    message: char[980]
  }
  ```

- Nhận `message-one`: thêm `content` vào list chat, KHÔNG cập nhật số tin nhắn chưa đọc.

- Nhận `message-one-new`: tương tự trên, nhưng có cập nhật số tin nhắn chưa đọc.

## 3. Đăng ký tài khoản và đăng nhập.

- Gửi `action = "login"`: `content` là biểu diễn char[] của struct Auth có dạng như sau:

  ```
  {
    username: char[20],
    password: char[20]
  }
  ```

- Gửi `action = "register"`: `content` lưu dữ liệu dạng tương tự như khi `action="login"`

- Gửi `action = "re-login"`: `content` lưu username ở client. Mục đích của thao tác này là để cập nhật lại socket mới trên server cho user hiện tại. Lý do cần cập nhật là vì khi chuyển từ dialog đăng nhập sang dialog chat thì socket cũ không còn sử dụng được.

- Nhận `action = "login-response"`: `content` lưu "OK" hoặc thông báo lỗi nếu tồn tại.

## 4. Các thao tác liên quan đến lịch sử chat.

- Gửi `action = "request-one"`: `content` lưu username của user mà user hiện tại muốn chat cùng. Thao tác này mục đích là yêu cầu server gửi tất cả lịch sử chat giữa 2 user.
- Gửi `action = "update-latest"`: `content` lưu username của user mà user hiện tại đang chat. Mục đích của thao tác này là để cập nhật số lượng message chưa đọc.

# IV. Kịch bản giao tiếp từ phía Server về Client

Ở phía server sẽ lưu một mảng các phần tử kiểu Client như sau:

```
{
  socket: SOCKET,
  username: char[20]
}
```

Danh sách username trong mảng này sẽ lấy từ database. Phần socket sẽ được gán giá trị khi user tương ứng login vào.

## 1. Chat chung giữa tất cả user.

- Nhận `action = "message-all"`: `content` là dòng chat mới.

- Gửi `action = "message-all-new"`: `content` là dòng chat mới. Thực hiện thao tác này khi nhận được thao tác ở trên.

## 2. Chat riêng giữa hai user.

- Nhận `action = "message-one"`: cấu trúc `content` tương tự phía Client.

- Gửi `action = "message-one-new"`: cấu trúc `content` tương tự, thực hiện khi nhận được thao tác `message-one` ở trên.

## 3. Đăng ký / Đăng nhập.

- Nhận `action = "login"` hoặc `action = "register"`: `content` là biểu diễn char[] của struct kiểu Auth (username, password).

- Gửi `action = "login-response"`: `content` lưu thông báo lỗi nếu có khi đăng nhập/đăng ký; hoặc "OK" nếu đăng nhập/đăng ký thành công.

## 4. Các thao tác liên quan đến lịch sử chat.

- Nhận `action = "update-latest"`: update dòng chat mới nhất user đã đọc ở trong database.

- Gửi `action = "request-one"`: lấy dữ liệu từ trong database. Với mỗi dòng chat, nếu dòng này đã đọc thì gửi về bằng thao tác `message-one`, ngược lại thì dùng thao tác `message-one-new`.

# V. Khởi tạo socket / Kết nối socket:

## 1. Khởi tạo socket ở Server:

```
WSADATA wsaData;
if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
    return false;
}
server = socket(AF_INET, SOCK_STREAM, 0);
serverAddress.sin_family = AF_INET;
serverAddress.sin_port = htons(PORT);
serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
bind(server, (SOCKADDR *)&serverAddress, sizeof(serverAddress));
listen(server, 5);

WSAAsyncSelect(server, m_hWnd, WM_SOCKET, FD_READ | FD_ACCEPT | FD_CLOSE);
```

## 2. Kết nối tới server từ client:

```
WSADATA wsaData;
if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
    return false;
}

client = socket(AF_INET, SOCK_STREAM, 0);
serverAddress.sin_family = AF_INET;
serverAddress.sin_port = htons(PORT);
serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

int err = connect(client, (SOCKADDR*) &serverAddress, sizeof serverAddress);
if (err == SOCKET_ERROR) {
    return false;
}
WSAAsyncSelect(client, m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
```

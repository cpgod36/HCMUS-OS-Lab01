# HỆ ĐIỀU HÀNH — Lab 01: Xv6 User Programs: File System

> Chuyển từ `CQ_Lab01.docx`. Nội dung giữ nguyên, chỉ định dạng lại. Ghi chú trong `[!NOTE]` là của nhóm, không có trong đề.

## Quy tắc chung

- Project được thực hiện theo nhóm: mỗi nhóm có tối đa 3 sinh viên.
- Tất cả các bài tập giống nhau sẽ được chấm 0 điểm trong toàn bộ quá trình thực hành (mặc dù các bài tập và dự án thực hành khác cũng được chấm điểm).
- Môi trường cài đặt: Linux.

## Nộp bài

Nộp bài tập trực tiếp trên trang web của khóa học (MOODLE), không chấp nhận bài nộp qua email hoặc các hình thức khác.

Tên tệp: `StudentID1_StudentID2_StudentID3.zip` (với StudentID1 < StudentID2 < StudentID3)
VD: Nhóm của bạn có 2 sinh viên: 2312001 và 2312002, tên tệp là: `2312001_2312002_2312003.zip`

Bao gồm:

- **`StudentID1_StudentID2_StudentID3_Report.pdf`**: Báo cáo nên ngắn gọn và súc tích. Không cố gắng đưa mã nguồn vào báo cáo. Mục đích của báo cáo là để bạn có cơ hội làm rõ giải pháp của mình, nêu ra những khó khăn gặp phải, và bổ sung thông tin hữu ích cho việc chấm điểm. Nếu bạn gặp vấn đề cụ thể, thử các phương pháp không thành công, hoặc có yêu cầu chưa được thực hiện đầy đủ, thì việc giải thích trong báo cáo có thể giúp chúng tôi đánh giá và chấm điểm một cách công bằng hơn.
- **Release**: Tệp diff (bản vá diff, ví dụ: `$ git diff > <StudentID1_StudentID2_StudentID3>.patch`)
- **Source**: Tệp Zip của xv6 (phiên bản được `make clean`)

**Lưu ý:** Cần thực hiện đúng các yêu cầu trên, nếu không, bài làm sẽ không được chấm.

> [!NOTE]
> Nhóm: `24120026_24120088_24120110.zip`. `git diff` không chứa file mới chưa track, nên dùng `git add -A && git diff --cached > ....patch` (hoặc `git add -N .` trước).

## Đánh giá

Bài làm của nhóm sẽ được chấm điểm dựa trên tính hoàn thiện, tính chính xác, phong cách lập trình, mức độ kiểm tra kỹ lưỡng, giải pháp và sự hiểu biết về mã nguồn.

Khi quản lý khóa học này, chúng tôi cố gắng hết sức để đánh giá công bằng từng cá nhân dựa trên sự đóng góp của mỗi người cho dự án.

---

# Yêu cầu

Bài thực hành này sẽ giúp bạn làm quen với xv6 và các system call của nó.

## 1. Chương trình `sleep`

Viết lệnh `sleep` cho xv6 (giống UNIX): dừng chương trình theo số ticks nhập từ người dùng.

Yêu cầu chính:

- File: `user/sleep.c`
- Nếu thiếu đối số ⇒ in lỗi.
- Đổi chuỗi sang số bằng `atoi`.
- Gọi system call `sleep`.
- Kết thúc bằng `exit(0)`.

Chạy chương trình từ shell xv6:

```
$ make qemu
...
init: starting sh
$ sleep 10
(nothing happens for a little while)
$
```

## 2. Chương trình `pingpong`

Dùng 2 pipe để "ping-pong" 1 byte giữa cha và con.

Luồng đúng:

1. Cha gửi 1 byte → Con đọc → Con in: `"<pid>: received ping"`
2. Con gửi lại 1 byte → Cha đọc → Cha in: `"<pid>: received pong"`
3. Cả hai thoát đúng cách.

Gợi ý syscall: `pipe`, `fork`, `write`, `read`, `getpid`.

Chạy chương trình từ shell xv6 và nó sẽ tạo ra kết quả sau:

```
$ make qemu
...
init: starting sh
$ pingpong
4: received ping
3: received pong
$
```

Chương trình của bạn sẽ trao đổi một byte giữa hai tiến trình và tạo ra đầu ra như hiển thị ở trên. Chạy `make grade` để kiểm tra.

## 3. Chương trình `primes`

Cài "concurrent prime sieve" theo mô hình pipeline bằng `pipe` và `fork` (ý tưởng Doug McIlroy).

Yêu cầu chính:

- Process đầu tạo số từ **2 đến 280** đưa vào pipe.
- Mỗi prime tạo 1 process lọc các bội số và chuyển sang pipe kế.
- Đóng FD đúng cách để không cạn tài nguyên.
- Process chính chỉ thoát sau khi toàn bộ pipeline kết thúc.
- Gợi ý: `read()` trả 0 khi đầu ghi pipe đã đóng.
- Nên truyền số nguyên 4-byte thay vì ASCII.

Một số gợi ý:

- Hãy cẩn thận khi đóng các tập tin mô tả mà một tiến trình không cần, nếu không chương trình của bạn sẽ chạy hết tài nguyên trước khi tiến trình đầu tiên đạt đến 280.
- Khi tiến trình đầu tiên đạt đến 280, nó phải đợi cho đến khi toàn bộ pipe kết thúc, bao gồm tất cả các tiến trình con, cháu, v.v. Do đó, tiến trình primes chính chỉ nên thoát sau khi tất cả đầu ra đã được in và sau khi tất cả các tiến trình primes khác đã thoát.
- Gợi ý: `read` trả về số không khi phía ghi của pipe bị đóng.
- Cách đơn giản nhất là ghi trực tiếp các số nguyên 32 bit (4 byte) vào các pipe, thay vì sử dụng định dạng ASCII I/O.
- Bạn chỉ nên tạo các tiến trình trong pipe khi cần thiết.
- Thêm chương trình vào `UPROGS` trong Makefile.
- Nếu bạn nhận được lỗi đệ quy vô hạn từ trình biên dịch cho hàm `primes`, bạn có thể phải khai báo `void primes(int) __attribute__((noreturn));` để chỉ ra rằng hàm primes không trả về.

Giải pháp của bạn phải triển khai sàng dạng ống và tạo ra kết quả đầu ra sau:

```
$ make qemu
...
init: starting sh
$ primes
prime 2
prime 3
prime 5
prime 7
prime 11
prime 13
prime 17
prime 19
prime 23
prime 29
prime 31
...
$
```

## 4. `cp` — copy file cơ bản

- File: `user/cp.c`
- Mục tiêu: Copy src → dst
- Cú pháp: `cp src dst`

Yêu cầu:

- Thiếu đối số: báo lỗi.
- Mở src read-only; mở dst để ghi (tạo mới/ghi đè).
- Copy theo buffer (vd 512/1024 bytes).
- Báo lỗi nếu mở/đọc/ghi thất bại.

Output mẫu (đề ghi nhầm câu "phải triển khai sàng dạng ống" ở đây — copy từ bài primes):

```
$ echo hello > a
$ cp a b
$ cat b
hello
$

$ cp a
usage: cp src dst
$

$ cp nofile out
cp: cannot open nofile
$
```

## 5. Chương trình `tree`

Mục tiêu: In cây thư mục theo dạng "cây" (thụt lề theo mức), giúp rèn duyệt thư mục + format output.

File bài làm: `user/tree.c`

Cú pháp: `tree [path] [-L depth] [-d]`

- `path`: mặc định `"."`
- `-L depth`: giới hạn độ sâu đệ quy (mặc định không giới hạn hoặc depth rất lớn)
- `-d`: chỉ in thư mục (không in file)

Yêu cầu:

- In cấu trúc cây bằng ký hiệu thụt lề (chọn 1 style cố định): ví dụ dùng `|--` và `` `-- `` hoặc chỉ dùng khoảng trắng.
- Không duyệt `"."` và `".."`.
- Nếu `-L 1` thì chỉ in path và các mục con trực tiếp.
- Với `-d`, bỏ qua file thường.

Gợi ý:

- Dùng `open`/`read` để duyệt thư mục (như `ls`/`find`).
- Dùng `fstat` để biết entry là file hay thư mục.
- Cần hàm đệ quy `tree(path, level, maxDepth, onlyDir)`.

Output mẫu — tạo cây:

```
$ mkdir a
$ echo hi > a/b.txt
$ mkdir a/aa
$ echo ok > a/aa/c.txt
$ mkdir z
```

1) Mặc định: `tree .`

```
$ tree .
.
├── a
│   ├── b.txt
│   └── aa
│       └── c.txt
└── z
$
```

2) Giới hạn độ sâu: `tree . -L 2`

```
$ tree . -L 2
.
├── a
│   ├── b.txt
│   └── aa
└── z
$
```

3) Chỉ in thư mục: `tree . -d`

```
$ tree . -d
.
├── a
│   └── aa
└── z
$
```

## 6. Chương trình `du`

Mục tiêu: Tính dung lượng thư mục theo dạng cộng dồn (giống `du` tối giản), sử dụng `fstat` và đệ quy tổng hợp.

File bài làm: `user/du.c`

Cú pháp: `du [path] [-a] [-s]`

- `path`: mặc định `"."`
- `-s` (summary): chỉ in tổng dung lượng của path
- `-a`: in cả file lẫn thư mục (mặc định chỉ in thư mục)

Yêu cầu:

- Duyệt đệ quy từ path.
- Lấy size từ `st.size` (bytes).
- Tổng dung lượng thư mục = tổng dung lượng các file/thư mục con (bạn có thể bỏ qua size "bản thân thư mục", miễn nhất quán và ghi rõ).
- Không duyệt `"."` và `".."`.

Gợi ý:

- Tách 2 phần:
  - hàm `long du(path)` trả về tổng bytes
  - phần in theo option `-a` hoặc `-s`
- Nếu `-a`: in từng file và từng thư mục kèm số bytes.

Output mẫu — giả sử cấu trúc thư mục (tạo dữ liệu minh hoạ):

```
$ mkdir a
$ echo 1234567890 > a/f1.txt
$ mkdir a/aa
$ echo hello > a/aa/f2.txt
$ echo world > a/aa/f3.txt
$ mkdir z
```

1) Mặc định: `du .` (chỉ in thư mục). Format đề xuất: `<bytes>\t<path>`

```
$ du .
64      ./a/aa
96      ./a
0       ./z
96      .
$
```

Giải thích:

- `./a/aa` = tổng bytes các file trong aa
- `./a` = tổng bytes trong a (gồm f1.txt và thư mục aa)
- `./z` rỗng nên 0
- `.` = tổng toàn cây

2) Summary: `du . -s` (chỉ in tổng)

```
$ du . -s
96      .
$
```

3) In cả file lẫn thư mục: `du . -a`

```
$ du . -a
16      ./a/f1.txt
32      ./a/aa/f2.txt
32      ./a/aa/f3.txt
64      ./a/aa
96      ./a
0       ./z
96      .
$
```

> [!NOTE]
> Số liệu mẫu không khớp `st.size` thật (`1234567890\n` = 11 byte, `hello\n` = 6 byte), chỉ mang tính minh hoạ. Nhóm cần chọn quy ước và ghi rõ trong report.

## 7. Chương trình `diff`

Mục tiêu: So sánh 2 file theo dòng, in ra các dòng khác nhau, sử dụng đọc dòng + xử lý chuỗi + case file dài/ngắn.

File bài làm: `user/diff.c`

Cú pháp: `diff file1 file2 [-q]`

- `-q`: chỉ báo "khác nhau" thay vì in chi tiết.

Yêu cầu (mức cơ bản – bắt buộc):

- Đọc 2 file theo từng dòng.
- Nếu giống hệt: không in gì (hoặc in một dòng "identical" nếu bạn muốn – nhưng phải ghi rõ).
- Nếu khác:
  - In các dòng chỉ có ở file1 với tiền tố `< `
  - In các dòng chỉ có ở file2 với tiền tố `> `
- Với `-q`: nếu khác thì chỉ in: `diff: files differ`

Gợi ý:

- Viết hàm `readline(fd, buf, max)` đọc đến `\n` hoặc EOF.
- So sánh từng dòng bằng `strcmp`.
- Nếu một file EOF trước file kia, phần còn lại xem như "chỉ có ở file kia".

Output mẫu — giả sử tạo 2 file minh hoạ:

```
$ echo -e "apple\nbanana\ncarrot" > f1
$ echo -e "apple\nblueberry\ncarrot\ndate" > f2
```

> [!NOTE]
> `echo` của xv6 không hỗ trợ `-e`/`\n` — cần cách khác để tạo file test.

1) Mặc định: `diff f1 f2`

Format đề xuất (chuẩn hoá). Nếu khác nhau tại dòng i:

```
f1:i: < <line_f1>
f2:i: > <line_f2>
```

Nếu file2 có thêm dòng (f1 đã hết): dùng `EOF` cho f1.

```
$ diff f1 f2
f1:2: < banana
f2:2: > blueberry
f1:4: < EOF
f2:4: > date
$
```

Giải thích:

- Dòng 2 khác nhau.
- Dòng 4: f1 hết rồi (EOF), f2 còn date.

2) Chế độ "quiet": `diff -q f1 f2`

```
$ diff -q f1 f2
diff: files differ
$
```

> [!NOTE]
> Cú pháp ghi `-q` ở cuối nhưng ví dụ đặt `-q` ở đầu → nên parse option ở vị trí bất kỳ.

3) Trường hợp giống hệt

```
$ diff f1 f1
$
```

---

## Thang điểm

| STT | Yêu cầu  | Điểm |
|-----|----------|------|
| 1   | pingpong | 1    |
| 2   | primes   | 1.5  |
| 3   | cp       | 1.5  |
| 4   | tree     | 1.5  |
| 5   | du       | 1.5  |
| 6   | diff     | 1.5  |
| 7   | report   | 1.5  |

(Tổng 10 điểm; `sleep` không có trong bảng điểm.)

## Tài liệu tham khảo

- https://pdos.csail.mit.edu/6.1810/2024/labs/util.html

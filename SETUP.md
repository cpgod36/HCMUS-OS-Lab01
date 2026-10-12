# SETUP — Lab01 xv6 (Hệ điều hành 24_22, HCMUS)

> Tài liệu cho thành viên nhóm **và AI agent của họ**. Đọc hết trước khi làm.
> Đề bài đầy đủ: [docs/CQ_Lab01.md](docs/CQ_Lab01.md). Hướng dẫn xv6: [docs/HuongDan_XV6.md](docs/HuongDan_XV6.md). Sách xv6: [docs/xv6-book/](docs/xv6-book/) (chỉ cần ch01 và ch08).

## 0. Nhóm và phân công

| Thành viên | MSSV | GitHub | Nhánh git | Bài phụ trách | File được tạo/sửa |
|---|---|---|---|---|---|
| Phan Chí Cao (**nhóm trưởng**) | 24120026 | `cpgod36` | `cao` | sleep, pingpong, primes + gộp code, nộp bài, report | `user/sleep.c`, `user/pingpong.c`, `user/primes.c` |
| Võ Phùng Nhật Long | 24120088 | `nhlcngdz` | `long` | **cp, diff** | `user/cp.c`, `user/diff.c` |
| Nguyễn Hoàng Nhật | 24120110 | `nh996` | `nhat` | **tree, du** | `user/tree.c`, `user/du.c` |

Ngoài file của mình, mỗi người chỉ được sửa **đúng dòng `UPROGS` của mình trong `Makefile`** (xem mục 4.2).

- Repo: https://github.com/cpgod36/HCMUS-OS-Lab01 (**private**, không được public: đề phạt 0 điểm nếu bài giống nhóm khác).
- Nhánh chính: `util`. Tag `base` = commit gốc của MIT (dùng để tạo patch nộp bài).

---

## 1. Cài môi trường (làm 1 lần)

Mục tiêu: Ubuntu 22.04 trên WSL2, toolchain RISC-V, **QEMU ≥ 7** (QEMU 6.2 mặc định của Ubuntu 22.04 bị treo khi boot xv6-labs-2024).

### 1.1. Cài Ubuntu 22.04 (PowerShell trên Windows)

```powershell
wsl -l -v                         # xem đã có Ubuntu-22.04 chưa
wsl --install -d Ubuntu-22.04     # nếu chưa có; sau đó tạo username/password khi được hỏi
```

Nếu `wsl` báo chưa bật WSL: mở PowerShell **Run as administrator**, chạy `wsl --install`, khởi động lại máy, rồi chạy lại lệnh trên.

Mọi lệnh bên dưới chạy **trong Ubuntu** (`wsl -d Ubuntu-22.04`).

### 1.2. Toolchain

```bash
sudo apt-get update
sudo apt-get install -y git build-essential gdb-multiarch qemu-system-misc \
    gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu software-properties-common
```

### 1.3. Nâng cấp QEMU (bắt buộc trên Ubuntu 22.04)

```bash
qemu-system-riscv64 --version          # nếu là 6.x thì làm tiếp
sudo apt remove -y qemu-system-misc qemu-system-common
sudo add-apt-repository -y ppa:canonical-server/server-backports
sudo apt update
sudo apt install -y qemu-system-misc
qemu-system-riscv64 --version          # kỳ vọng: 9.x (máy nhóm trưởng: 9.0.2)
```

### 1.4. GitHub CLI + đăng nhập

```bash
(type -p wget >/dev/null || sudo apt install -y wget) \
 && sudo mkdir -p -m 755 /etc/apt/keyrings \
 && wget -qO- https://cli.github.com/packages/githubcli-archive-keyring.gpg | sudo tee /etc/apt/keyrings/githubcli-archive-keyring.gpg > /dev/null \
 && sudo chmod go+r /etc/apt/keyrings/githubcli-archive-keyring.gpg \
 && echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
 && sudo apt update && sudo apt install -y gh

gh auth login        # GitHub.com → HTTPS → Yes → Login with a web browser (người dùng tự làm, agent không làm thay)
git config --global user.name  "<github-username>"
git config --global user.email "<email-github>"
```

### 1.5. Nhận lời mời và clone

1. Mở email hoặc https://github.com/cpgod36/HCMUS-OS-Lab01/invitations → **Accept**.
2. Clone **trong WSL, vào thư mục home** (build nhanh hơn `/mnt/c`, `/mnt/d` và tránh lỗi CRLF; **không** clone bằng Git for Windows):

```bash
cd ~
git clone https://github.com/cpgod36/HCMUS-OS-Lab01.git
cd HCMUS-OS-Lab01
git checkout -b long        # Nhật: nhat
```

### 1.6. Kiểm tra môi trường

```bash
make qemu
```

Kỳ vọng thấy `init: starting sh` và dấu nhắc `$`. Gõ `ls` để thấy danh sách file. **Thoát QEMU: `Ctrl+A` rồi bấm `x`.**

Nếu bị treo sau dòng `xv6 kernel is booting` → QEMU chưa được nâng cấp (mục 1.3).

---

## 2. Quy trình làm việc với git

```bash
git checkout long                         # luôn làm trên nhánh của mình
git pull origin util                      # cập nhật thay đổi mới từ util (nên làm trước khi bắt đầu)
# ... code + test ...
git add user/cp.c Makefile
git commit -m "cp: copy file with 512-byte buffer"
git push -u origin long
gh pr create --base util --title "cp + diff" --body "..."   # khi xong bài
```

Quy tắc:
- **Không** push thẳng lên `util`, **không** `git push --force`, **không** merge PR (nhóm trưởng review và merge).
- Commit nhỏ, message rõ ràng. Lịch sử commit là bằng chứng đóng góp cá nhân (đề chấm theo đóng góp từng người).
- Không commit file build (`*.o`, `_*`, `fs.img`... đã có trong `.gitignore`). Kiểm tra `git status` trước khi commit.
- Conflict ở `Makefile` khi pull: giữ **cả hai** dòng `UPROGS`.

---

## 3. Build, chạy, test

| Lệnh | Tác dụng |
|---|---|
| `make qemu` | Build và boot xv6 (thoát: `Ctrl+A`, `x`) |
| `make clean` | Xoá file build. Chạy khi thấy code mới không có hiệu lực |
| `make grade` | Chấm tự động toàn bộ lab util |
| `make GRADEFLAGS=pingpong grade` | Chỉ chạy test có tên chứa `pingpong` |

`make grade` **chỉ** có test cho sleep, pingpong, primes. Các test `find`, `xargs`, `time` là của MIT, **không thuộc đề của lớp → fail là bình thường**. Các bài cp, diff, tree, du **phải tự test thủ công** trong QEMU theo output mẫu trong đề, và chụp màn hình để đưa vào report.

`fs.img` được tạo lại mỗi lần `make qemu` khi có thay đổi. File tạo trong lúc chạy xv6 có thể mất sau khi build lại. Cần dựng lại dữ liệu test mỗi lần.

---

## 4. Ghi chú kỹ thuật xv6 (đọc kỹ, đặc biệt là agent)

### 4.1. Khung chương trình user

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"      // struct dirent, DIRSIZ (cần cho tree/du)
#include "kernel/fcntl.h"   // O_RDONLY, O_WRONLY, O_CREATE, O_TRUNC

int
main(int argc, char *argv[])
{
  ...
  exit(0);                  // KHÔNG dùng return từ main
}
```

Style: theo code có sẵn của xv6 (`user/ls.c`, `user/cat.c`, `user/grep.c`), thụt lề 2 space, kiểu trả về nằm trên dòng riêng. **`user/ls.c` là mẫu để duyệt thư mục** (tree/du), **`user/cat.c` là mẫu đọc/ghi file** (cp).

### 4.2. Thêm chương trình vào Makefile

Trong khối `UPROGS=\` đầu tiên của `Makefile`, thêm **sau dòng `$U/_zombie\`**:

```make
	$U/_zombie\
	$U/_cp\
	$U/_diff\
```

(Nhật: `$U/_tree\` và `$U/_du\`.) Dòng phải bắt đầu bằng **TAB**, kết thúc bằng `\`.

### 4.3. Thư viện có sẵn — KHÔNG có libc

Chỉ có các hàm trong `user/user.h`:
- syscall: `fork exit wait pipe write read close kill exec open mknod unlink fstat link mkdir chdir dup getpid sbrk sleep uptime`
- ulib: `stat strcpy memmove strchr strcmp fprintf printf gets strlen memset atoi memcmp memcpy malloc free`

Không có: `strncmp`, `strcat`, `sprintf`, `snprintf`, `fopen`, `fgets`, `opendir`, `strtok`, `isdigit`... Cần thì tự viết hàm `static`.

**`printf` / `fprintf` của xv6 chỉ hỗ trợ:** `%d %ld %lld %u %lu %x %lx %p %s %%`.
- **KHÔNG có `%c`**: in 1 ký tự bằng `write(1, &c, 1)` hoặc `printf("%s", buf)` với buf 2 byte.
- **KHÔNG có width/padding** (`%5d`, `%-14s` không hoạt động).
- In lỗi ra stderr: `fprintf(2, "cp: cannot open %s\n", path);`

### 4.4. Cấu trúc dữ liệu file system

```c
// kernel/stat.h
#define T_DIR 1  /  T_FILE 2  /  T_DEVICE 3
struct stat { int dev; uint ino; short type; short nlink; uint64 size; };

// kernel/fs.h
#define DIRSIZ 14
struct dirent { ushort inum; char name[DIRSIZ]; };   // name KHÔNG chắc có '\0' nếu dài đúng 14
```

Duyệt thư mục: `open(path, O_RDONLY)` → `fstat` kiểm tra `T_DIR` → lặp `read(fd, &de, sizeof(de)) == sizeof(de)`:
- bỏ entry có `de.inum == 0` (slot trống)
- bỏ `"."` và `".."`
- ghép đường dẫn `path + "/" + name` (copy name tối đa `DIRSIZ` byte, tự thêm `'\0'`)
- `stat(fullpath, &st)` để biết là file hay thư mục

Thứ tự entry = thứ tự tạo (không sắp xếp). Thư mục có `st.size` là bội của 16 (= số dirent × 16).

### 4.5. Giới hạn hệ thống

| Hằng | Giá trị | Ảnh hưởng |
|---|---|---|
| `NOFILE` | 16 fd / process | Phải `close` fd không dùng (primes, đệ quy trong tree/du: đóng fd thư mục trước hoặc ngay sau khi đệ quy) |
| `NPROC` | 64 process | primes (2..280) cần khoảng 60 process, vừa đủ nếu đóng/đợi đúng |
| `MAXPATH` | 128 | Kiểm tra độ dài đường dẫn trước khi ghép |
| `O_TRUNC` | có | `cp` mở dst bằng `O_WRONLY \| O_CREATE \| O_TRUNC` |

### 4.6. Shell của xv6 — cách tạo dữ liệu test

- Hỗ trợ: `;` `|` `>` `<` `&` `( )`. **`>>` KHÔNG append** (ghi đè từ đầu file, không truncate).
- `echo` **không** hỗ trợ `-e` hay `\n` → lệnh `echo -e "apple\nbanana"` trong đề **không chạy được** trên xv6.
- Tạo file nhiều dòng trong xv6: `(echo apple; echo banana; echo carrot) > f1` (subshell + redirect, mỗi `echo` 1 dòng).

---

## 5. Quy ước chung của nhóm

> ⚠️ **ĐỀ XUẤT, CHƯA CHỐT.** Cứ làm theo đề xuất; nếu muốn khác thì báo nhóm trưởng. Mọi quy ước phải ghi lại trong report.

| Bài | Vấn đề đề bài để mở | Đề xuất |
|---|---|---|
| tree | Style ký hiệu | Dùng `├── `, `└── `, `│   `, 4 khoảng trắng (giống output mẫu; xv6 in UTF-8 qua `%s` bình thường) |
| tree | Thứ tự tham số | Chấp nhận path và option ở vị trí bất kỳ: `tree -d .`, `tree . -L 2 -d` |
| du | Có tính size của bản thân thư mục? | **Không**: size thư mục = tổng size các file bên trong (đệ quy). Thư mục rỗng = 0 |
| du | Số liệu mẫu trong đề (16/32/64/96) | Không khớp `st.size` thật (`echo 1234567890` → 11 byte). Output đúng là size thật, ghi chú trong report |
| du | Thứ tự in | Hậu thứ tự (con trước, cha sau), theo thứ tự entry trong thư mục, format `<bytes>\t<path>` |
| diff | Hai file giống nhau | Không in gì, `exit(0)` |
| diff | Vị trí `-q` | Chấp nhận ở bất kỳ vị trí nào (`diff -q f1 f2` và `diff f1 f2 -q`) |
| diff | So sánh | Theo từng dòng cùng chỉ số (dòng i với dòng i), không dùng LCS. Format `f1:i: < ...` / `f2:i: > ...`, file hết trước in `EOF` |
| chung | Lỗi tham số | In usage ra stderr (`fprintf(2, ...)`) và `exit(1)` |

---

## 6. Khi hoàn thành bài — gửi cho nhóm trưởng

1. PR vào `util` từ nhánh của mình. Code build không warning, chạy đúng các output mẫu trong đề.
2. Một đoạn ngắn (markdown, trong mô tả PR) cho report:
   - Ý tưởng giải pháp từng bài (không dán code, đề cấm đưa code vào report).
   - Các case đã test và kết quả, kèm **ảnh chụp màn hình** QEMU.
   - Khó khăn gặp phải, quy ước đã chọn.
3. **Hiểu code của mình.** Đề chấm cả "sự hiểu biết về mã nguồn", có thể bị hỏi trực tiếp.

---

## 7. Dành riêng cho AI agent

- Phạm vi: chỉ tạo/sửa file trong cột "File được tạo/sửa" ở mục 0 của người đang dùng bạn, cộng dòng `UPROGS` tương ứng. **Không sửa `kernel/`**, không sửa file của thành viên khác, không sửa `SETUP.md`/`docs/` (báo nhóm trưởng nếu thấy sai).
- Đọc đúng phần đề của bài mình trong [docs/CQ_Lab01.md](docs/CQ_Lab01.md) và tuân theo mục 4 và 5 của file này.
- Sau mỗi thay đổi: `make qemu` phải build sạch (xv6 build với `-Werror`, mọi warning đều làm hỏng build), rồi chạy thử từng output mẫu của đề trong QEMU.
- Git: chỉ commit/push lên nhánh của người dùng (`long` / `nhat`). Không push `util`, không force push, không merge, không đổi cài đặt repo (visibility, collaborators...).
- Không chép lời giải có sẵn trên mạng (đề phạt 0 điểm nếu bài giống nhau). Viết code dễ đọc, có comment ngắn ở chỗ khó, để thành viên giải thích được khi bị hỏi.
- Không cần làm `find`, `xargs`, `time.txt` của MIT.

## 8. Nộp bài (nhóm trưởng làm)

```bash
git checkout util && git pull
git diff base -- . ':(exclude)docs' ':(exclude)SETUP.md' > 24120026_24120088_24120110.patch
make clean
# zip toàn bộ source → 24120026_24120088_24120110.zip gồm: Report.pdf + .patch + source zip
```

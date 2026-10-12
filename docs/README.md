# Tài liệu Lab01 (bản markdown)

| File | Nguồn | Nội dung |
|---|---|---|
| [CQ_Lab01.md](CQ_Lab01.md) | `CQ_Lab01.docx` | **Đề bài**: quy định nộp, 7 chương trình (sleep, pingpong, primes, cp, tree, du, diff), thang điểm |
| [HuongDan_XV6.md](HuongDan_XV6.md) | `HƯỚNG DẪN THỰC HÀNH XV6.pdf` + `fix.md` | Cài WSL/xv6, fix QEMU, viết user program, thêm syscall, struct proc, fork, pipe, bảng trang |
| [xv6-book/](xv6-book/) | `book-xv6-riscv-rev4.pdf` | Sách xv6 tách theo chương (text thô, mất hình) |

## Chương sách liên quan tới Lab01

- [ch01-operating-system-interfaces.md](xv6-book/ch01-operating-system-interfaces.md): processes, file descriptors, **pipes**, **file system** (stat, mkdir, link). Đọc chương này là đủ cho cả lab.
- [ch08-file-system.md](xv6-book/ch08-file-system.md): 8.11 directory layer (`struct dirent`), 8.12 path names. Hữu ích cho tree/du.
- [ch07-scheduling.md](xv6-book/ch07-scheduling.md): 7.7 code pipes, 7.8 wait/exit. Đọc thêm nếu cần.
- Các chương còn lại (page tables, traps, locking...) không cần cho lab này.

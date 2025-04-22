Dev C++編輯時 : 工具 -> 編譯器選項 -> 在連結器命令列中加入以下的命令 : "-static-libgcc -lgdi32 -lws2_32 -std=c++11"

file.exe : 新增一個名為"ip"的文字文件(.txt)在同一個父資料夾內，連線ip就會設定為該文字文件內的文字(ip.txt內存放指定ip位置)

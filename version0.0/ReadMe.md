2020/10/17 AM 11:14

### 文件说明

```bash
src 		# 资源目录
gdb_test 	# 与项目无关
```

### 启动方式

```bash
gcc epoll_server.c main.c -o server.out 
./server.out 8080 ./src/
```

![](https://wooyooyoo-photo.oss-cn-hangzhou.aliyuncs.com/blog/2020/10/Snipaste_2020-10-17_10-05-14.png)

### 项目说明

这个`webserver`我是看了`黑马linux高并发web服务器开发`后写的.

我遇到的坑:

在给浏览器发送请求响应时也就是`http_respond()`中我在发送空行时,`\ 写成了/ 导致页面一直刷不出来`

另外一个难点就是要理解`http`展示中文内容,中文的解码和转码那一块,那块的函数我是直接用的黑马视频中提供的函数~.
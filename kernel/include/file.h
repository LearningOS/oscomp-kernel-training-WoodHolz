#ifndef __FILE_H
#define __FILE_H

struct file {
  enum { FD_NONE, FD_PIPE, FD_ENTRY, FD_DEVICE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe; // FD_PIPE
  struct dirent *ep;
  uint off;          // FD_ENTRY
  short major;       // FD_DEVICE
};

 struct MapArea{
   uint64 start;     //起始地址
   int len;          //长度
   int prot;         //权限（read/write）
   int flags;        //映射是否与其他进程共享的标志（shared/private）
   int off;          //文件偏移量
   int fd;
   int valid;        //有效
   struct file* f;   //映射的文件
 };
// #define major(dev)  ((dev) >> 16 & 0xFFFF)
// #define minor(dev)  ((dev) & 0xFFFF)
// #define	mkdev(m,n)  ((uint)((m)<<16| (n)))

// map major device number to device functions.
struct devsw {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

extern struct devsw devsw[];

#define CONSOLE 1

struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, uint64, int n);
int             filestat(struct file*, uint64 addr);
int             filewrite(struct file*, uint64, int n);
int             dirnext(struct file *f, uint64 addr);
//int             fdalloc(struct file *f);
int             fdalloc2(struct file *f, int newfd);
int             fileopen(int dfd, char *path, int omode);
#endif
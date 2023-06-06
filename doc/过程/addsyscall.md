# 添加系统调用

## 以`wait4`和`uname`为例

1. 在`kernel/include/sysnum.h`中添加系统调用和其对应编号的宏

   ``` c
   // kernel/include
   #define SYS_wait4     260
   ```

2. 在`kernel/syscall.c`的`static char *sysnames[]`数组中添加系统调用号和相应的系统调用名称之间的映射关系

   ``` c
   // kernel/syscall.c
   static char *sysnames[] = {
       ...
       [SYS_wait4]       "wait4",
       ...
   };
   ```

3. 在`kernel/syscall.c`中声明的系统调用函数并`static uint64 (*syscalls[])(void)`中添加系统调用号和相应的系统调用函数之间的映射关系

   ``` c
   // kernel/syscall.c
   extern uint64 sys_wait4(void);
   
   static uint64 (*syscalls[])(void) = {
       ...
       [SYS_wait]        sys_wait,
       ...
   };
   ```

4. 在`kernel/sysproc.c`中添加`sys_wait4`函数

   ``` c
   // kernel/sysproc.c
   uint64 sys_wait4(void)
   {
     int pid;
     int *status;
     int options;
     if(argint(0, &pid) < 0 || argaddr(1, (void *)&status) < 0 || argint(2, &options) < 0 ){
       return -1;
     }
     return wait4(pid, status, options);
     
   }
   ```

5. 在`kernel/include/proc.h`中声明`wait4`函数

   ``` c
   // kernel/include/proc.h
   #ifndef __PROC_H
   #define __PROC_H
   ...
   int wait4(int pid, int *status, int options);
   ...
   #endif
   ```

6. 在`kernel/proc.c`中定义`wait4`函数

   ``` c
   // kernel/proc.c
   int wait4(int pid, int *status, int options)
   {
     struct proc *p = myproc();
     struct proc *child;
     int havekids, foundpid;
     acquire(&p->lock);
     ...
     release(&child->lock);
       }
   ```

7. (可选)添加`c`文件(如`uname.c`)在`kernel/include`中创建`uname.c`的头文件并将其添加到`kernel/syscall.c`中

   ``` c
   // 在kernel/include创建info.h
   #define maxlen (65)
   struct utsname {
   char sysname[maxlen];
   char nodename[maxlen];
   char release[maxlen];
   char version[maxlen];
   char machine[maxlen];
   char domainname[maxlen];
   };
   
   int uname(struct utsname *uts);
   
   
   // kernel/syscall.c
   ...
   #include "include/info.h"
   ...
   
   ```

8. (可选)如添加`c`文件,则需要在`Makefile`中添加相关信息

   ``` makefile
   # Makefile
   OBJS += \
    ...
    $K/uname.o \
    ...
   
   ```

## 理想结果

``` log
// make all && make run # shell

...
========== START test_waitpid ==========
This is child process
waitpid successfully.
wstatus: 3
========== END test_waitpid ==========
========== START test_uname ==========
Uname: origin qemu 0.0 a very first version rv64 on qemu 0.0.0.0
========== END test_uname ==========
...
```

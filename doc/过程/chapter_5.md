# 第五章

## 进程的数据结构

``` c
enum procstate {
    UNUSED,     // 未初始化
    USED,       // 基本初始化，未加载用户程序
    SLEEPING,   // 休眠状态(未使用，留待后续拓展)
    RUNNABLE,   // 可运行
    RUNNING,    // 当前正在运行
    ZOMBIE,     // 已经 exit；一个进程存在父进程且在父进程未结束时就结束，在等待父进程释放其资源
};

struct proc {
  struct spinlock lock;     // 进程锁

  // 使用这些变量需要持有 p->lock：
  enum procstate state;     // 进程状态
  struct proc *parent;      // 父进程
  void *chan;               // 如果不为零，则正在睡眠等待 chan
  int killed;               // 如果不为零，则已被杀死
  int xstate;               // 返回给父进程 wait 的退出状态码
  int pid;                  // 进程ID

  // 这些变量是私有的，因此不需要持有 p->lock。
  uint64 kstack;            // 内核栈的虚拟地址
  uint64 sz;                // 进程内存大小（字节）
  pagetable_t pagetable;    // 用户页表
  pagetable_t kpagetable;   // 内核页表
  struct trapframe *trapframe; // trampoline.S 的数据页
  struct context context;   // 在此处运行进程的 swtch()
  struct file *ofile[NOFILE];  // 打开的文件
  struct dirent *cwd;       // 当前目录
  char name[16];            // 进程名称（用于调试）
  int tmask;                // 跟踪掩码
};
```

## 进程的基本管理

``` c
void scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  extern pagetable_t kernel_pagetable;

  c->proc = 0;
  for (;;) {
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();

    int found = 0;
    for (p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if (p->state == RUNNABLE) {
	// Switch to chosen process.  It is the process's job
	// to release its lock and then reacquire it
	// before jumping back to us.
	// printf("[scheduler]found runnable proc with pid: %d\n", p->pid);
	p->state = RUNNING;
	c->proc = p;
	w_satp(MAKE_SATP(p->kpagetable));
	sfence_vma();
	swtch(&c->context, &p->context);
	w_satp(MAKE_SATP(kernel_pagetable));
	sfence_vma();
	// Process is done running for now.
	// It should have changed its p->state before coming back.
	c->proc = 0;

	found = 1;
      }
      release(&p->lock);
    }
    if (found == 0) {
      intr_on();
      asm volatile("wfi");
    }
  }
}
```

``` mermaid
graph LR;
  A[开始] --> B[获取当前CPU c的进程描述符指针]
  B --> C{扫描进程表}
  C-- 找到RUNNABLE的进程 --> D[获取进程p的锁]
  D --> E{将进程p状态改为RUNNING}
  E --> F[设置当前CPU c的进程描述符指针为进程p]
  F --> G[切换到进程p的上下文, 并更新c的上下文]
  G --> H{将进程p状态改为READY}
  H -- 是 --> I[释放进程p的锁]
  H -- 否 --> C
  I --> J[设置当前CPU c的进程描述符指针为0]
  J --> C
  C-- 没有RUNNABLE的进程 --> K{进入等待模式}
  K -- 是 --> L[开启中断并等待下一个中断]
  K -- 否 --> C
  L --> C
```



``` c
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&p->lock))
    panic("sched p->lock");
  if(mycpu()->noff != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}


```

``` mermaid
graph LR;
  A[开始] --> B[获取当前进程p的指针]
  B --> C{检查p的锁是否被占用}
  C -- 占用 --> D[触发panic:p->lock]
  C -- 未占用 --> E{检查当前CPU是否正在持有锁}
  E -- 持有 --> F[触发panic:sched locks]
  E -- 未持有 --> G{检查p的状态是否为RUNNING}
  G -- 是 --> H[触发panic:sched running]
  G -- 否 --> I{检查p是否可以中断}
  I -- 可中断 --> J[触发panic:sched interruptible]
  I -- 不可中断 --> K[保存并禁止当前CPU中断标志]
  K --> L[切换到进程p的上下文]
  L --> M[恢复当前CPU中断标志]
```



## 进程的重要系统调用

### fork系统调用

``` mermaid
graph TD;
  A[开始] --> B[分配新的进程描述符np]
  B --> C{复制用户空间数据}
  C-- 复制成功 --> D[设置np大小和trapframe值与p相同]
  D --> E{复制实际文件描述符表和当前工作目录}
  E -- 复制成功 --> F[设置np名称为p的名称]
  F --> G[设置np状态为RUNNABLE]
  G --> H[返回子进程的pid]
  C-- 复制失败 --> I[释放内存并返回-1]
  E -- 复制失败 --> I
  I --> J[结束]
```

### exec系统调用

``` mermaid
graph TD;
  A[开始] --> B[获取当前进程p的指针]
  B --> C{复制当前进程的kpagetable}
  C -- 复制成功 --> D[分配新的pagetable]
  D --> E{查找文件路径}
  E -- 查找成功 --> F[读取ELF头]
  F -- ELF头有效 --> G[加载程序的段到内存中]
  G --> H[分配新的用户栈并复制参数到用户栈中]
  H --> I[为用户程序设置参数并设置为进程的入口点]
  I --> J[保存程序名称]
  J --> K[切换到新的pagetable并释放旧的pagetable]
  K --> L[返回参数argc]
  E -- 查找失败 --> M[跳转到bad]
  F -- ELF头无效 --> M
  G -- 加载程序段失败 --> M
  H -- 分配用户栈失败 --> M
  H -- 复制参数失败 --> M
  I -- 参数无效 --> M
  J -- 保存程序名称失败 --> M
  K -- 切换pagetable失败 --> M
  M --> N[释放资源]
```

### wait系统调用

``` mermaid
graph LR;
  A[开始] --> B[获取当前进程p的锁]
  B --> C{扫描进程表}
  C-- 找到退出的子进程 --> D[获取子进程np的锁]
  D --> E{将np的xstate复制到addr}
  E -- 复制成功 --> F[释放子进程np的锁并删除进程描述符]
  F --> G[释放当前进程p的锁并返回子进程pid]
  E -- 复制失败 --> H[释放子进程np的锁并返回-1]
  H --> B
  C-- 没找到退出的子进程 --> I{没有子进程或进程已被杀死?}
  I -- 是 --> J[释放当前进程p的锁并返回-1]
  I -- 否 --> K[等待子进程退出并重新扫描进程表]
  K --> B
```


#include "include/types.h"
#include "include/riscv.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/spinlock.h"
#include "include/proc.h"
#include "include/syscall.h"
#include "include/timer.h"
#include "include/kalloc.h"
#include "include/string.h"
#include "include/printf.h"
#include "include/vm.h"
#include "include/mmap.h"

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
extern int exec(char *path, char **argv);

uint64 sys_gettimeofday(void)
{
  struct timeval *tv;
  struct timezone *tz;

  if(argaddr(0, (void*)&tv) < 0 || argaddr(1, (void*)&tz) < 0) {
    return -1;
  }

  if (tv == NULL) {
    return 0;
  }

  uint64 time = r_time();
	tv->sec = time / 12500000;
	tv->usec = (time % 12500000) * 1000000 / 12500000;
  
  return 0;
}

uint64 sys_getppid(void)
{
  return getppid();
}

uint64 sys_brk(void) 
{
  int addr;
  if(argint(0, &addr) < 0)
    return -1;
  
  //addr = PGROUNDDOWN(addr);
  if(addr >= KERNBASE || addr <= 0)
    return myproc()->sz;

  int n = addr - myproc()->sz;
  
  return growproc(n); 
}

uint64 sys_sched_yield(void)
{
  yield();
  return 0;
}
extern struct proc proc[];
uint64 sys_times(void)
{
  struct proc *curp = myproc();
  uint64 addr;
  struct tms tms;
  tms.tms_utime = curp->utimes;
  tms.tms_stime = curp->stimes;
  tms.tms_cutime = 0;
  tms.tms_cstime = 0;
  
  struct proc *p;
  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p->parent == curp)
    {
      tms.tms_cutime += p->utimes;
      tms.tms_cstime += p->stimes;
    }
  }

  if (argaddr(0, &addr) < 0) 
  {
    return -1;
  }

  copyout2(addr, (char *)&tms, sizeof(tms));

  return tms.tms_stime+tms.tms_utime;
}

uint64 sys_nanosleep(void)
{
  struct timespec *req, *rem;
  uint ticks0;
  int n;

  if(argaddr(0, (uint64 *)&req) < 0 || argaddr(1, (uint64 *)&rem) < 0)
    return -1;
  n = req->tv_sec + req->tv_nsec/(1 << 9);

  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64 sys_execve(void)
{
  char path[FAT32_MAX_PATH], *argv[MAXARG], *envp[MAXARG];
  int i;
  uint64 uargv, uarg, uenvp, uenv;

  if(argstr(0, path, FAT32_MAX_PATH) < 0 || argaddr(1, &uargv) < 0 || argaddr(2, &uenvp) < 0){
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  memset(envp, 0, sizeof(envp));
  for(i=0;; i++){
    if(i >= NELEM(argv)){
      goto bad;
    }
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      goto bad;
    }
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    argv[i] = kalloc();
    if(argv[i] == 0)
      goto bad;
    if(fetchstr(uarg, argv[i], PGSIZE) < 0)
      goto bad;
  }
  for(i=0;; i++){
    if(i >= NELEM(envp)){
      goto bad;
    }
    if(fetchaddr(uenvp+sizeof(uint64)*i, (uint64*)&uenv) < 0){
      goto bad;
    }
    if(uenv == 0){
      envp[i] = 0;
      break;
    }
    envp[i] = kalloc();
    if(envp[i] == 0)
      goto bad;
    if(fetchstr(uenv, envp[i], PGSIZE) < 0)
      goto bad;
  }

  int ret = execve(path, argv, envp);

  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  for(i = 0; i < NELEM(envp) && envp[i] != 0; i++)
    kfree(envp[i]);

  return ret;

 bad:
  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  return -1;
}

uint64
sys_exec(void)
{
  char path[FAT32_MAX_PATH], *argv[MAXARG];
  int i;
  uint64 uargv, uarg;

  if(argstr(0, path, FAT32_MAX_PATH) < 0 || argaddr(1, &uargv) < 0){
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv)){
      goto bad;
    }
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      goto bad;
    }
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    argv[i] = kalloc();
    if(argv[i] == 0)
      goto bad;
    if(fetchstr(uarg, argv[i], PGSIZE) < 0)
      goto bad;
  }

  int ret = exec(path, argv);

  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);

  return ret;

 bad:
  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  return -1;
}

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

// uint64 sys_clone(void)
// {
//    int flags;
//     uint64 stack;
//     pid_t ptid;
//     uint64 tls;
//     uint64 ctid;
//     argint(0, &flags);
//     argaddr(1, &stack);
//     argint(2, &ptid);
//     argaddr(3, &tls);
//     argaddr(4, &ctid);
//     return do_clone(flags, stack, ptid, tls, (pid_t *)ctid); 
// }

uint64
sys_clone(void)
{
  uint64 stack, epc;
  if(argaddr(1, &stack) < 0) {
    return -1;
  }
  fetchaddr(stack, &epc);
  return do_clone(stack, epc);
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{
  int mask;
  if(argint(0, &mask) < 0) {
    return -1;
  }
  myproc()->tmask = mask;
  return 0;
}

uint64
sys_mmap(void)
{
  
  uint64 start,len, prot, flags, fd, off;

  if (argaddr(0, &start) < 0||argaddr(1, &len) < 0||argaddr(2, &prot)||argaddr(3, &flags)||argaddr(4, &fd)||argaddr(5, &off)){
		return -1;
  }
  struct proc* p = myproc();
  struct file* f = p -> ofile[fd];

  if( ( flags == 0x01 && f->writable == 0 && (prot&2)) )
  return -1;
  //uint64 addrstart = p -> sz;

  struct MapArea* v = NULL;
  for(int i=0; i < 16; i++ ){
    if(p -> mmap_Areas[i].valid == 0){
      v = &p -> mmap_Areas[i];
      v -> valid = 1;
      v -> start = PGROUNDUP(p -> sz);
      v -> len = len;
      v -> prot = prot;
      v -> flags = flags;
      v -> fd = fd;
      v -> off = off;
      v -> f = f;
      edup(v -> f -> ep);
      break;
    }
  }
  if(!v) return -1;
  else{
    growproc(v->start - p->sz + 5);
    if (growproc(len + 5) < 0)
      panic("pagefault map error");
    for(int i = 0; len ; i += PGSIZE){
      uint64 mem;
      if((mem = (uint64)kalloc())==NULL) return -1;
      int l = len<PGSIZE?len:PGSIZE;
      mem = walkaddr(p -> pagetable, v->start+i);
      elock( v->f->ep );
      eread( v->f->ep , 0 , mem , off , l);
      eunlock( v->f->ep );
      len -= l;
      off += l;
    }
  }
  return v -> start;
}
uint64 sys_munmap(void)
{
  return 0;
}
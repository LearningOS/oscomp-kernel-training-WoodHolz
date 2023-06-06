
#include "include/types.h"
#include "include/syscall.h"
#include "include/riscv.h"
#include "include/timer.h"
#include "include/proc.h"
#include "include/vm.h"
#include "include/timer.h"

uint64 sys_gettimeofday(void)
{
  struct timeval tv;
  struct timezone tz;

  if(argstr(0, (void*)&tv, sizeof(tv)) < 0 || argstr(1, (void*)&tz, sizeof(tz)) < 0) {
    return -1;
  }

  uint64 time = r_time();
	tv.sec = time / 12500000;
	tv.usec = (time % 12500000) * 1000000 / 12500000;
  
  return 0;
}

uint64 sys_nanosleep(void)
{
    uint64 tv;
    if (argaddr(0, &tv) < 0) {
        return -1;
    }
    // 这里参数１和参数２一样，原本１是所需睡眠时间，２是返回剩余时间（可能被唤醒？）


    TimeVal tm;
    // 将 虚拟地址tv的内容　拷贝到tm
    copyin2((char *)&tm, tv, sizeof(tm));

    if (tm.sec >= 1000000000L || tm.sec < 0)
		return -1;

    uint32 t = ksleep(tm.sec);
    tm.sec = t;

    copyout2(tv, (char *)&tm, sizeof(tm));

    return t;
}
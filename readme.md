# OSKernel2023-openCh

基于 _`xv6-k210`_ 和 _`uCore`_

``` tree
oskernel2023-origin
├── debug
│   ├── kendryte_openocd
│   └── openocd_cfg
├── doc
│   ├── 过程
│   ├── 技术报告
│   ├── Chat-Talk
│   │   ├── u1
│   │   └── u2
│   └── img
├── initcodes
├── kernel
│   └── include
├── linker
├── target
├── tools
└── xv6-user

17 directories

```

## 搭建并运行内核

1. [环境搭建流程](环境配置过程.md)
2. __`make all`__ # 编译内核
3. __`make run`__ # 运行内核并进行测例
4. __`make clean`__ # 清空内核和镜像

## 文档

1. ### [架构](doc/过程/architecture.md)

2. ### [riscv基础](doc/过程/chapter_2.md)

3. ### [内核态用户态的切换/中断](doc/过程/chapter_3.md)

4. ### [进程部分](doc/过程/chapter_5.md)

5. ### [添加系统调用](doc/过程/addsyscall.md)

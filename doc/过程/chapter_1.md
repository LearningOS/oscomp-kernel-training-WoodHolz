# DOC

## 应用程序基本执行环境

``` graph-easy
+-------------------------+
|       Application       |
+-------------------------+
  ^
  | function call
  |
+-------------------------+
|         std lib         |
+-------------------------+
  ^
  | syscall
  |
+-------------------------+
| Operating System/kernel |
+-------------------------+
  ^
  | ISA
  |
+-------------------------+
|    Hardware Platform    |
+-------------------------+


# graph-easy <<< 'graph{flow:north}[Hardware Platform] - ISA -> [Operating System/kernel]- syscall -> [std lib] - function call -> [Application]'
```

通用的操作系统中的执行环境栈，下层是上层的执行环境，保证上层程序正常运行。相邻上下层间通过接口完成调用。

其中函数标准库和操作系统/内核都是对下层资源的__抽象__。

> 对于用某种编程语言实现的应用程序源代码而言，编译器在将其通过编译、链接得到可执行文件的时候需要知道程序要在哪个**平台** (Platform) 上运行。这里平台主要是指 CPU 类型、操作系统类型和标准运行时库的组合。可以看出：
>
> 1. 如果用户态基于的内核不同，会导致系统调用接口不同或者语义不一致；
>
> 2. 如果底层硬件不同，对于硬件资源的访问方式会有差异。特别是如果 ISA 不同，则向软件提供的指令集和寄存器都不同。
>
> 它们都会导致最终生成的可执行文件有很大不同。需要指出的是，某些编译器支持同一份源代码无需修改就可编译到多个不同的目标平台并在上面运行。这种情况下，源代码是 **跨平台** 的。而另一些编译器则已经预设好了一个固定的目标平台。

---

## 在qemu上的启动流程

``` makefile
# BOARD
BOARD		?= qemu
SBI			?= rustsbi
BOOTLOADER	:= ./bootloader/rustsbi-qemu.bin

QEMU = qemu-system-riscv64
QEMUOPTS = \
	-nographic \ # 设置为没有图形界面
	-machine virt \ # Hardware Platform
	-bios $(BOOTLOADER) \
	-kernel build/kernel	\

run: build/kernel
	$(QEMU) $(QEMUOPTS)

```

设置qemu模拟 `virt` 硬件平台，所以物理内存的起始物理地址为 `0x80000000`~~死规定~~。上面代码中:

``` makefile
BOOTLOADER	:= ./bootloader/rustsbi-qemu.bin
QEMUOPTS = \
	-bios $(BOOTLOADER)
```

就是将作为bootloader的 `rustsbi-qemu.bin` 加载到物理地址`0x80000000` 处。



这节有一些待定/未定的代码，之后再完成这里的文档

---

## 程序的内存布局和编译流程

### 内存布局

当程序被编译为可执行文件后，该可执行文件的内容可以分为__代码__和__数据__2个部分。CPU执行代码部分，并将数据部分作为可读写的内存空间。但是现实中，往往进一步对这两个部分进行进一步的划分，我们称这些更小的单位为`段`（`Section`）。

将不同`段`按一定的"约定"分配放置在内存的位置上，这就形成了__内存布局__。

``` graph-easy
			# high address
+---------+ 
|  stack  | # 栈
+---------+
|   ...   |
+---------+
|  heap   | # 堆
+---------+
|  .bss   | # 未初始化数据段 
+---------+
|  .data  | # 可修改的全局数据
+---------+
| .rodata | # 只读的全局数据
+---------+
|  .text  | # Code Memory 其他都属于Data Memory
+---------+ 
			# low address

# graph-easy <<< '[stack||...||heap||.bss||.data||.rodata||.text] '
```

### 编译流程

``` graph-easy
+-------------------------+
| application source code |
+-------------------------+
  |
  | compiler
  v
+-------------------------+
|        assembler        |
+-------------------------+
  |
  | assembler
  v
+-------------------------+
|       object code       |
+-------------------------+
  |
  | linker
  v
+-------------------------+
|       executables       |
+-------------------------+

# graph-easy <<< 'graph{flow:south}[application source code] - compiler -> [assembler] - assembler -> [object code] - linker -> [executables]'
```

#### 链接器

##### Before

``` graph-easy
+-----------+ # 0x0
|  .text_1  |
+-----------+ # 0x2000
| .rodata_1 |
+-----------+ # 0x5000
|  .data_1  |
+-----------+ # 0x7000
+-----------+ # 0x0
|  .text_2  |
+-----------+ # 0x1000
| .rodata_2 |
+-----------+ # 0x3000
|  .data_2  |
+-----------+ # 0x6000

# graph-easy <<< '[.text_1||.rodata_1||.data_1] [.text_2||.rodata_2||.data_2]'
```

##### After

``` graph-easy

+-----------+ # 0x0
|  .text_1  |
+-----------+ # 0x2000
|  .text_2  |
+-----------+ # 0x3000
| .rodata_1 |
+-----------+ # 0x6000
| .rodata_2 |
+-----------+ # 0x8000
|  .data_1  |
+-----------+ # 0xa000
|  .data_2  |
+-----------+ # 0xd000

graph-easy <<< '[.text_1||.text_2||.rodata_1||.rodata_2||.data_1||.data_2]'

                             +-----------+     +-----------------+
                             |  .text_1  | --> | sections from 1 | <+
                             +-----------+     +-----------------+  |
     +-----------------+     |           |       ^                  |
  +> | sections from 2 | <-- |  .text_2  |       |                  |
  |  +-----------------+     +-----------+       |                  |
  |    ^                     |           |       |                  |
  |    |                     | .rodata_1 | ------+------------------+
  |    |                     +-----------+       |
  |    |                     |           |       |
  +----+-------------------- | .rodata_2 |       |
       |                     +-----------+       |
       |                     |           |       |
       |                     |  .data_1  | ------+
       |                     +-----------+
       |                     |           |
       +-------------------- |  .data_2  |
                             +-----------+

# echo "graph{flow:east}[.text_1||.text_2||.rodata_1||.rodata_2||.data_1||.data_2] [.text_1.text_2.rodata_1.rodata_2.data_1.data_2.0] -> [sections from 1] [.text_1.text_2.rodata_1.rodata_2.data_1.data_2.1] -> [sections from 2] [.text_1.text_2.rodata_1.rodata_2.data_1.data_2.2] -> [sections from 1] [.text_1.text_2.rodata_1.rodata_2.data_1.data_2.3] -> [sections from 2] [.text_1.text_2.rodata_1.rodata_2.data_1.data_2.4] -> [sections from 1] [.text_1.text_2.rodata_1.rodata_2.data_1.data_2.5] -> [sections from 2]" > graph.txt
# graph-easy ./graph.txt

```

---

系统的链接脚本(kernel.ld)决定了elf程序的内存地址的虚拟映射，__程序中的绝对地址在链接时确定__。我们需要把系统放置在物理地址0x80200000处，需要进行这步操作的原因是在这个运行时刚进入S态，并没有激活虚存机制。~~我还不知道为什么是这个地址~~。

### entry.s

``` assembly
# os/entry.s
    .section .text.entry #该 段为内核入口点，放在段最低地址; 在下一个新的部分结束，即.section .bss.stack
    .globl _entry # 声明全局符号_entry
_entry: # 声明入口点的开始
    la sp, boot_stack_top # (sp) = (boot_stack_top)
    call main # 调用main函数，进入Cの世界 ^_^

    .section .bss.stack # 堆栈未初始化数据段
    .globl boot_stack
boot_stack:
    .space 4096 * 16 # 2^16B == 4KB * 16 == 64KB
    				 # 定义堆栈空间大小需要满足大小是4KB的整数倍。
    .globl boot_stack_top
boot_stack_top:

```

> 这是一个汇编语言源文件，可以作为可执行程序的启动代码。下面是逐行解释： 
>
> 1. `.section .text.entry`: 这行代码表示将下面的代码放置在名为 ".text.entry" 的代码段中。 
> 2. `.globl _entry`: 这行代码定义了一个全局符号 "_entry"，该符号是程序的入口点，在程序启动时执行。 
> 3. `_entry:`: 这行代码是 _entry 符号的标签，代表程序的入口点。 
> 4. `la sp, boot_stack_top`: 这行代码使用 la 命令生成代码，将 boot_stack_top 符号的地址加载到 sp 寄存器中，将栈指针设置为堆栈的顶部。 
> 5. `call main`: 这行代码通过 call 指令调用名为 "main" 的函数。 
> 6. `.section .bss.stack`: 这行代码将下面的代码放置在名为 ".bss.stack" 的未初始化数据段中。 
> 7. `.globl boot_stack`: 这行代码定义了一个全局符号 "boot_stack"，用于表示堆栈的起始位置。 
> 8. `boot_stack:`: 这行代码是 boot_stack 符号的标签，表示变量存储在此处。 
> 9. `.space 4096 * 16`: 这行代码使用 space 命令在内存中分配 4096 * 16 字节的空间。这是堆栈的空间大小。 
> 10. `.globl boot_stack_top`: 这行代码定义了一个全局符号 "boot_stack_top"，该符号用于表示堆栈的顶部位置，即栈指针初始值。 
> 11. `boot_stack_top:`: 这行代码是 boot_stack_top 符号的标签，表示指针存储在此处。

#### 小知识点

* `.globl`是什么？

`.globl`指示符表示一个全局符号/标签；例：`.globl _entry`: 这行代码定义了一个全局符号 "_entry"

* 以下代码中`la`是什么意思？

``` assembly
la sp, boot_stack_top # (sp) = (boot_stack_top)
```

"la"是RISC-V指令集中的一个伪指令(pseudo-instruction），它的全称为“load address”，通常用于将地址加载到寄存器中。

下面是"la"伪指令的语法格式：

```assembly
la rd, symbol
```

其中，"rd"表示目标寄存器，"symbol"表示要加载的地址符号。

例如，下面的代码将符号"array"的地址加载到寄存器x1中：

```assembly
la x1, array
```

当汇编器处理这个伪指令时，它将使用类似于以下实际指令的指令序列来替代它：

```assembly
auipc x1, %pcrel_hi(array)
addi x1, x1, %pcrel_lo(array)
```

这个指令序列使用了两个新的伪指令：%pcrel_hi和%pcrel_lo。这两个伪指令用于计算符号的高位和低位偏移，在这个例子中，将符号"array"的地址作为立即数加载到寄存器x1中。


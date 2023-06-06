    .align 4
    .section .data
    .global initcode
    .global initcodesize
initcode:
    .incbin "./xv6-user/initcode"
codeend:

initcodesize:
    .long codeend-initcode


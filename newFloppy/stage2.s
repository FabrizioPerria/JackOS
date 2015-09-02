.code16
_start:
    jmp main

main:
    jmp _section2
    nop

_section2:
    mov $0,%ax
    int $0x16
    mov $0xe,%ah
    mov $0,%bx
    int $0x10
    jmp _section2
    jmp .

.byte 0x55
.byte 0xaa

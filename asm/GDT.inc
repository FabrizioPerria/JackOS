##############################
# INSTALL the GDT table and  #
# RESET segment registers    #
##############################

.global gdt_flush

gdt_flush:
    lgdt (gdt_ptr)
    movw $0x10,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%fs
    movw %ax,%gs
    movw %ax,%ss
    jmp $0x8,$_next
_next:
    ret

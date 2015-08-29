ENTRY(_start)
phys = 0x00100000;		
SECTIONS
{   
    .text phys :AT(phys) {
        code = .;
        *(.text)
        . = ALIGN(4);
    }
    
    .rodata : AT(phys + (rodata - code)) {
        rodata = .;
        *(.rodata)
        . = ALIGN(4);
    }
    
    .data : AT(phys + (data - code)) {
        data = .;
        *(.data)
        . = ALIGN(4);
    }
    
    .bss : AT(phys + (bss - code)) {
        bss = .;
        *(.bss)
        . = ALIGN(4);
    }
    end = .;
}

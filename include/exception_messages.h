#ifndef __EXCEPTION_MESSAGES_H
#define __EXCEPTION_MESSAGES_H

/* Message structure for the exceptions */

char* exception_messages[] =
{
    "INT0: Division By Zero Exception",
    "INT1: Debug Exception",
    "INT2: Non-Maskable Interrupt Exception",
    "INT3: Breakpoint Exception",
    "INT4: Into Detected Overflow Exception",
    "INT5: Out Of Buonds Exception",
    "INT6: Invalid Opcode Exception",
    "INT7: No Coprocessor Exception",
    "INT8: Double Fault Exception",
    "INT9: Coprocessor Segment Overrun Exception",
    "INT10: Bad TSS Exception",
    "INT11: Segment Not Present Exception",
    "INT12: Stack Fault Exception",
    "INT13: General Protection Fault Exception",
    "INT14: Page Fault Exception",
    "INT15: Unknown Interrupt Exception",
    "INT16: Coprocessor Fault Exception",
    "INT17: Alignment Check Exception",
    "INT18: Machine Check Exception",
    "Reserved Exception"
};

#endif

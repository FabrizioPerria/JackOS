.extern handler
.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23
.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31

.global irq0
.global irq1
.global irq2
.global irq3
.global irq4
.global irq5
.global irq6
.global irq7
.global irq8
.global irq9
.global irq10
.global irq11
.global irq12
.global irq13
.global irq14
.global irq15

#Divide by zero Exception
isr0:
    cli
    pushl $0
    jmp isr_common

#Debug Exception
isr1:
    cli
    pushl $1
    jmp isr_common
    
#NMI Exception
isr2:
    cli
    pushl $2
    jmp isr_common

#BreakPoint Exception
isr3:
    cli
    pushl $3
    jmp isr_common

#Overflow Exception
isr4:
    cli
    pushl $4
    jmp isr_common

#Out of Bounds Excpetion
isr5:
    cli
    pushl $5
    jmp isr_common
    
#Invalid Opcode Exception
isr6:
    cli
    pushl $6
    jmp isr_common
    
#No Coprocessor Eception
isr7:
    cli
    pushl $7
    jmp isr_common
    
#Double Fault Exception
isr8:
    cli
    pushl $8
    jmp isr_common
    
#Coprocessor Overrun Exception
isr9:
    cli
    pushl $9
    jmp isr_common
    
#Bad TSS Exception
isr10:
    cli
    pushl $10
    jmp isr_common

#Segment Not Present Exception
isr11:
    cli
    pushl $11
    jmp isr_common

#Stack Fault Exception
isr12:
    cli
    pushl $12
    jmp isr_common
    
#General Protection Fault Exception
isr13:
    cli
    pushl $13
    jmp isr_common
    
#Page Fault Exception    
isr14:
    cli
    pushl $14
    jmp isr_common

#Unknown Int Exception
isr15:
    cli
    pushl $15
    jmp isr_common
    
#Coprocessor Fault Exception
isr16:
    cli
    pushl $16
    jmp isr_common
    
#Alignment Check Exception
isr17:
    cli
    pushl $17
    jmp isr_common
    
#Machine Check Excpetion
isr18:
    cli 
    pushl $18
    jmp isr_common

#Reserved
isr19:
    cli
    pushl $19
    jmp isr_common

#Reserved
isr20:
    cli
    pushl $20
    jmp isr_common

#Reserved
isr21:
    cli
    pushl $21
    jmp isr_common

#Reserved
isr22:
    cli 
    pushl $22
    jmp isr_common

#Reserved
isr23:
    cli 
    pushl $23
    jmp isr_common

#Reserved
isr24:
    cli 
    pushl $24
    jmp isr_common

#Reserved
isr25:
    cli 
    pushl $25
    jmp isr_common

#Reserved
isr26:
    cli 
    pushl $26
    jmp isr_common

#Reserved
isr27:
    cli 
    pushl $27
    jmp isr_common

#Reserved
isr28:
    cli
    pushl $28
    jmp isr_common

#Reserved
isr29:
    cli 
    pushl $29
    jmp isr_common

#Reserved
isr30:
    cli 
    pushl $30
    jmp isr_common

#Reserved
isr31:
    cli
    pushl $31
    jmp isr_common

irq0:
    cli
    push $32
    jmp irq_common

irq1:
    cli
    push $33
    jmp irq_common

irq2:
    cli
    push $34
    jmp irq_common
    
irq3:
    cli
    push $35
    jmp irq_common
    
irq4:
    cli
    push $36
    jmp irq_common

irq5:
    cli
    push $37
    jmp irq_common
    
irq6:
    cli
    push $38
    jmp irq_common
    
irq7:
    cli
    push $39
    jmp irq_common
    
irq8:
    cli
    push $40
    jmp irq_common
   
irq9:
    cli
    push $41
    jmp irq_common
    
irq10:
    cli
    push $42
    jmp irq_common
    
irq11:
    cli
    push $43
    jmp irq_common
    
irq12:
    cli
    push $44
    jmp irq_common
    
irq13:
    cli
    push $45
    jmp irq_common
    
irq14:
    cli
    push $46
    jmp irq_common
    
irq15:
    cli
    push $47
    jmp irq_common

#########################
# ISR                   #
# Save registers before #
# executing the routine #
#########################

.extern isr_handler

isr_common:
    pushal
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    movl %esp,%eax
    pushl %eax
    movl $0x10,%eax
    movl %eax,%ds
    movl %eax,%es
    movl %eax,%fs
    movl %eax,%gs
    movl $isr_handler,%eax
    call *%eax
    popl %eax
    popl %gs
    popl %fs
    popl %es
    popl %ds
    popal 
    add $4,%esp
    iret

#########################
# IRQ                   # 
# Save registers before #
# executing the routine #
#########################   
.extern irq_handler

irq_common:
#Save the registers
    pushal
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    movl %esp,%eax
    pushl %eax
    movl $0x10,%eax
    movl %eax,%ds
    movl %eax,%es
    movl %eax,%fs
    movl %eax,%gs
#execute the handler
    movl $irq_handler,%eax
    call *%eax
#restore the registers
    popl %eax
    popl %gs
    popl %fs
    popl %es
    popl %ds
    popal
    add $4,%esp
    iret

#####################
# INSTALL IDT table #
#####################

.global idt_flush

idt_flush:
    lidt (idtPtr)
    ret


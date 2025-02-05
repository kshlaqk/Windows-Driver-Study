.code

; 函数: ReadAll_asm
; 功能: 将 CR0, CR2, CR3, CR4, CR8, IDTR, GDTR, LDTR 读取到传入的全局数组中，返回该数组的地址
; 参数: rdi - 全局数组的地址

ReadAll_asm proc
    ; 保存被调用者保存的寄存器
    push rbx
    push rsi

    ; 读取 CR0
    mov rax, cr0
    mov [rcx + 0*8], rax

    ; 读取 CR2
    mov rax, cr2
    mov [rcx + 1*8], rax

    ; 读取 CR3
    mov rax, cr3
    mov [rcx + 2*8], rax

    ; 读取 CR4
    mov rax, cr4
    mov [rcx + 3*8], rax

    ; 读取 CR8 (仅在 x86-64 架构上支持)
    mov rax, cr8
    mov [rcx + 4*8], rax

    ; 读取 IDTR (Interrupt Descriptor Table Register)
    lea rsi, [rcx + 5*8]  ; 将 IDTR 写入传入数组的位置
    sidt [rsi]

    ; 读取 GDTR (Global Descriptor Table Register)
    lea rsi, [rcx + 6*8]  ; 将 GDTR 写入传入数组的位置
    sgdt [rsi]

    ; 返回全局数组的地址
    mov rax, rcx

    ; 恢复寄存器
    pop rsi
    pop rbx
    ret
ReadAll_asm endp

end
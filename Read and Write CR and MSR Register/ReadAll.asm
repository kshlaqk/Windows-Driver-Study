.code

; ����: ReadAll_asm
; ����: �� CR0, CR2, CR3, CR4, CR8, IDTR, GDTR, LDTR ��ȡ�������ȫ�������У����ظ�����ĵ�ַ
; ����: rdi - ȫ������ĵ�ַ

ReadAll_asm proc
    ; ���汻�����߱���ļĴ���
    push rbx
    push rsi

    ; ��ȡ CR0
    mov rax, cr0
    mov [rcx + 0*8], rax

    ; ��ȡ CR2
    mov rax, cr2
    mov [rcx + 1*8], rax

    ; ��ȡ CR3
    mov rax, cr3
    mov [rcx + 2*8], rax

    ; ��ȡ CR4
    mov rax, cr4
    mov [rcx + 3*8], rax

    ; ��ȡ CR8 (���� x86-64 �ܹ���֧��)
    mov rax, cr8
    mov [rcx + 4*8], rax

    ; ��ȡ IDTR (Interrupt Descriptor Table Register)
    lea rsi, [rcx + 5*8]  ; �� IDTR д�봫�������λ��
    sidt [rsi]

    ; ��ȡ GDTR (Global Descriptor Table Register)
    lea rsi, [rcx + 6*8]  ; �� GDTR д�봫�������λ��
    sgdt [rsi]

    ; ����ȫ������ĵ�ַ
    mov rax, rcx

    ; �ָ��Ĵ���
    pop rsi
    pop rbx
    ret
ReadAll_asm endp

end
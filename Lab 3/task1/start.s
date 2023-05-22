section .rodata
endline: db 10, 0
in: db "-i", 0
out: db "-o", 0
stdout EQU 1 
stderr EQU 2
SYS_READ EQU 3
SYS_WRITE EQU 4
SYS_OPEN EQU 5
SYS_CLOSE EQU 6


section .data
Infile dd 0
Outfile dd 1
message db 'c'


section .text
global _start
global system_call
global main

extern strlen
extern strncmp

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main
    call    encode
    call    fcloseFiles

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller


openInFile:
    push ebp
    mov ebp, esp
    pushad

    mov eax, SYS_OPEN
    mov ebx, [ebp + 8]
    mov ecx, 2             ;read write
    int 0x80

    mov [Infile], eax

    popad
    pop ebp
    ret

fcloseFiles:
    push ebp
    mov ebp, esp
    pushad

    mov eax, SYS_CLOSE
    mov ebx, [Infile]
    int 0x80

    mov eax, SYS_CLOSE
    mov ebx, [Outfile]
    int 0x80

    popad
    pop ebp
    ret

openOutFile:
    push ebp
    mov ebp, esp
    pushad

    mov eax, SYS_OPEN
    mov ebx, [ebp + 8]
    mov ecx, 2                 ;read write
    int 0x80

    mov [Outfile], eax

    popad
    pop ebp
    ret

main:
    push ebp
    mov ebp, esp
    pushad
    
    ; set up registers for looping over command line arguments
    mov ebx, [ebp + 8] ; argc
    mov ecx, [ebp + 12] ; argv
    mov edx, 0
    
    ; loop over command line arguments
    loop:
        push ebx
        push ecx
        push edx
        push dword [ecx]
        call strlen
        add esp, 4
        pop edx
        pop ecx
        pop ebx
        
        ; push argument length and string pointer to stack and call system call to write to stderr
        push eax
        push dword [ecx]
        push stderr
        push SYS_WRITE
        call system_call
        add esp, 16
        
        ; write newline character to sterr
        push dword 1
        push dword endline
        push stderr
        push SYS_WRITE
        call system_call
        add esp, 16

        ;checkinfile
        push ebx
        push ecx
        push edx
        push dword 2
        push dword [ecx]
        push dword in
        call strncmp
        add esp, 12
        pop edx
        pop ecx
        pop ebx
        cmp eax, 0
        jne checkoutfile
        add dword [ecx], 2
        push dword [ecx]
        call openInFile
        add esp, 4


        checkoutfile:
        push ebx
        push ecx
        push edx
        push dword 2
        push dword [ecx]
        push dword out
        call strncmp
        add esp, 12
        pop edx
        pop ecx
        pop ebx
        cmp eax, 0
        jne doneChecking
        add dword [ecx], 2
        push dword [ecx]
        call openOutFile
        add esp, 4

        doneChecking:
        ; update loop counter and argument pointer
        inc edx
        add ecx, 4
        cmp ebx, edx
        jnz loop
        
    popad
    pop ebp
    ret



encode:
    push ebp
    mov ebp, esp
    pushad

    ; Loop through each character in the input file
    loop2:
        ; Read one character from the input file
        mov eax, SYS_READ
        mov ebx, [Infile]
        mov ecx, message
        mov edx, 1
        int 0x80

        ; Check if end of file has been reached
        cmp eax, 0
        jle end

        ; Check if the character is in range and increment it
        cmp byte [message], 'A'
        jl write
        cmp byte [message], 'z'
        jg write
        inc byte [message]

    ; Write the character to the output file
    write:
        mov eax, SYS_WRITE
        mov ebx, [Outfile]
        mov ecx, message
        mov edx, 1
        int 0x80

    jmp loop2

    end:
        popad
        pop ebp
        ret
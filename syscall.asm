DEFAULT REL
EXTERN __SYSCALL_ADDR 
EXTERN SSN_wrvirmem
EXTERN SSN_opproc
EXTERN SSN_allvirmem
EXTERN SSN_crth

section .code
    global _impwrvirmem
    global _impopproc
    global _impallvirmem
    global _impcrth

_impwrvirmem:
 mov r10,rcx
 mov eax, [rel SSN_wrvirmem]
 jmp  [rel  __SYSCALL_ADDR]
 ret 



 _impopproc:
    mov r10,rcx
    mov eax,[rel SSN_opproc]
    jmp [rel __SYSCALL_ADDR]
    ret


_impallvirmem:
    mov r10,rcx
    mov eax, [rel SSN_allvirmem]
    jmp [rel __SYSCALL_ADDR]
    ret


_impcrth:
    mov r10,rcx
    mov eax, [rel SSN_crth]
    jmp [rel __SYSCALL_ADDR]
    ret


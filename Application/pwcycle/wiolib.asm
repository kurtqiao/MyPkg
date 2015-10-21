
;.MODEL small 
;.386

  PUBLIC set_int_flag
.code

set_int_flag proc
  push rbp
  mov rbp, rsp
  mov rax, [rbp+4]    ;parameter 1
  cli
  cmp rax, 0
  jz ccc
  sti
ccc:

  pop rbp

  ret
set_int_flag endp

END
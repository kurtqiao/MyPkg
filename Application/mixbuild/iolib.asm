;Mixing Assembly and C
;https://courses.engr.illinois.edu/ece390/books/labmanual/c-prog-mixing.html

;https://msdn.microsoft.com/en-us/library/zthk2dkh(v=vs.80).aspx
;Integer: First 4 parameters – RCX, RDX, R8, R9. Others passed on stack.

;http://biosren.com/thread-6632-1-2.html
;RAX, RCX, RDX, R8, R9, R10, R11 are considered volatile and 
;must be considered destroyed on function calls, so there is 
;no reason to save them.
;First 4 parameters - RCX, RDX, R8, R9. Others passed on stack.
;The registers RBX, RBP, RDI, RSI, R12, R13, R14, and R15 are 
;considered nonvolatile and must be saved and restored by a 
;function that uses them.

.MODEL small 
.386

  PUBLIC _ADD_VAL
  PUBLIC _in_byte
  PUBLIC _out_byte
.code 

_in_byte proc
  push bp
  mov bp, sp
  xor ax, ax
  mov dx, word ptr [bp+4]
  in al, dx
  out 0edh, al
  pop bp
  ret
_in_byte endp

_out_byte proc
  push dx
  push bp
  mov bp, sp
  xor ax, ax
  mov dx, [bp+4]    ;parameter 1, port
  mov ax, [bp+6]    ;parameter 2, data
  out dx, al
  pop bp
  pop dx
  ret
_out_byte endp

_ADD_VAL proc
  push dx
  push bp
  mov bp, sp
  mov ax, [bp+4]
  mov bx, [bp+6]
  mov ax, bx
  pop bp
  pop dx
  ret
_ADD_VAL endp

END

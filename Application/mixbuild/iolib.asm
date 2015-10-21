
.MODEL small 
.386

  PUBLIC _ADD_VAL
  PUBLIC _in_byte
  PUBLIC _out_byte
.code 

_in_byte proc
;  push dx
  push bp
  mov bp, sp
  xor ax, ax
  mov dx, word ptr [bp+4]
  in al, dx
  out 0edh, al
  pop bp
;  pop dx
  ret
_in_byte endp

_out_byte proc
  push dx
  push bp
  mov bp, sp
  xor ax, ax
  mov ax, [bp+6]
  mov dx, [bp+4]
  out dx, al
;  out 0edh, al
;  mov ax, dx
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
;  mov dx, 5
  pop bp
  pop dx
  ret
_ADD_VAL endp

END

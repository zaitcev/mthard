CODE Segment Para
  Assume CS:CODE

  org 100h
  Start:
    lea AX,StackTop
    mov SP,AX

    sub AX,AX          ;Hang up
;-- mov AX,1           ;Drop carrier, keep connection
    mov DX,03FCh       ; for COM1
    out DX,AX

    mov AX,4C00h
    int 21h

  Stack:  db 100h dup (?)
  StackTop:

CODE EndS
    End Start

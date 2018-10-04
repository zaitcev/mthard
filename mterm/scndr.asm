_TEXT Segment byte public 'CODE'
  Assume CS:_TEXT

;  Video services
_VIDEO_SETMODE     = 000h  ;Set video mode
_VIDEO_SETPOS      = 002h  ;Set cursor position
_VIDEO_ACTPAGE     = 005h  ;Select active display page
_VIDEO_UPSCROLL    = 006h  ;Scroll window
_VIDEO_DOWNSCROLL  = 007h  ;Reverse scroll for a window
_VIDEO_READ        = 008h  ;Read character/attribute at cursor
_VIDEO_WRITE       = 009h  ;Write  - - - -  

;
;    Video mode setting
;    call:   Sinit( mode );   where mode is a BIOS mode number
;
  Public _Sinit
_Sinit Proc near
     push BP
InitMode = 4
     mov BP,SP

     mov AX,InitMode[BP]
     mov AH,_VIDEO_SETMODE
     int 010h
     mov AH,_VIDEO_ACTPAGE
     sub AL,AL
     int 010h

     pop  BP
     ret
_Sinit EndP

;
;    Screen erasing
;    call:    Serase( top, bottom, lpos, rpos, attr );
;
  Public _Serase
_Serase Proc near
    push DX
    push CX
    push BX
    push BP
EraseFrame Struc
  _BP     dw ?
  _BX     dw ?
  _CX     dw ?
  _DX     dw ?
  _IP     dw ?
  SeTop   dw ?
  SeBot   dw ?
  SeLeft  dw ?
  SeRight dw ?
  SeAttr  dw ?
EraseFrame EndS
    mov BP,SP

    mov CX,SeLeft[BP]
    mov AX,SeTop[BP]
    mov CH,AL

    mov DX,SeRight[BP]
    mov AX,SeBot[BP]
    mov DH,AL

    mov BX,SeAttr[BP]
    mov BH,BL

    mov AH,_VIDEO_UPSCROLL
    sub AL,AL              ;Pseudo-scroll for erasing

    int 010h

    pop BP
    pop BX
    pop CX
    pop DX
    ret
_Serase EndP

;
;   Spos( Y, X );
;
  Public _Spos
_Spos Proc near
    push DX
    push BX
    push BP
PosFrame Struc
          dw 4 dup (?)
    SetY  dw ?
    SetX  dw ?
PosFrame EndS
    mov BP,SP

    mov DX,SetX[BP]
    mov AX,SetY[BP]
    mov DH,AL
    sub BH,BH          ;Page = 0
    mov AH,_VIDEO_SETPOS
    int 010h

    pop BP
    pop BX
    pop DX
    ret
_Spos EndP

;
;   Write character into video storage
;   call:   Swrite( c, attr );
;
  Public _Swrite
_Swrite Proc near
    push CX
    push BX
    push BP
WriteFrame Struc
               dw 4 dup (?)
    WriteChar  dw ?
    WriteAttr  dw ?
WriteFrame EndS
    mov BP,SP

    mov CX,1
    mov BX,WriteAttr[BP]
    sub BH,BH              ;Page = 0
    mov AX,WriteChar[BP]
    mov AH,_VIDEO_WRITE
    int 010h

    pop BP
    pop BX
    pop CX
    ret
_Swrite EndP

;
;    General-purpose BIOS Video Service Call
;    call: void int10( union REGS* );
;
  Public _int10
_int10 Proc near
     push BP
     mov  BP,SP
     push BX
     push CX
     mov  BP,4[BP]
     mov AX,0[BP]
     mov BX,2[BP]
     mov CX,4[BP]
     mov DX,6[BP]
     push BP
     int 010h
     pop BP
     mov 0[BP],AX
     mov 2[BP],BX
     mov 4[BP],CX
     mov 6[BP],DX
     pop CX
     pop BX
     pop BP
     ret
_int10 EndP

_TEXT EndS
     End

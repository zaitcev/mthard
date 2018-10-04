_TEXT Segment byte public 'CODE'
  Assume CS:_TEXT
;
;    Keyboard test
;    call    if (i = Kdpm()) < 0 than  nothing was entered
;
  Public _Kdpm
_Kdpm Proc near
     mov AH,001h       ;Check for the keystroke
     int 016h
     mov AX,0FFFFh
     jz ret0

     mov AH,000h       ;Read the ASCII code
     int 016h
     and AH,07Fh

     ret0:
     ret
_Kdpm EndP

_TEXT EndS
     End

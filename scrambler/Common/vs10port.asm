.686p 
.model flat, stdcall
public _imp__DecodePointer@4
public _imp__EncodePointer@4
.data
_imp__DecodePointer@4 dq dummy32
_imp__EncodePointer@4 dq dummy32
.code
dummy32 PROC p: DWORD
mov eax, p
ret sizeof(DWORD)
dummy32 ENDP
end

;.model flat

;.data
;__imp__EncodePointer@4 dd dummy
;__imp__DecodePointer@4 dd dummy
;EXTERNDEF __imp__EncodePointer@4 : DWORD
;EXTERNDEF __imp__DecodePointer@4 : DWORD

;.code
;dummy proc
;mov eax, [esp+4]
;ret 4
;dummy endp

;end
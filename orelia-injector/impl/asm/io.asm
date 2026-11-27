EXTERN g_nt_device_io_control_ssn:DWORD

.code

direct_device_control PROC

	mov r10, rcx
	mov eax, DWORD PTR [g_nt_device_io_control_ssn]
	syscall
	ret

direct_device_control ENDP

END
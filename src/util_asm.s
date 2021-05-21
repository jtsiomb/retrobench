	.text

	.equ F_ID,	0x200000

	.globl _read_cpuid
	.globl read_cpuid
_read_cpuid:
read_cpuid:
	# determine if cpuid is available
	pushf
	pop %eax
	mov %eax, %edx	# keep a copy of the original eflags in edx
	xor $F_ID, %eax
	push %eax
	popf
	pushf
	pop %eax
	cmp %eax, %edx
	jnz 0f
	# failed to flip ID bit, CPUID not supported
	mov $-1, %eax
	ret
0:
	push %ebp
	mov %esp, %ebp
	push %ebx
	push %edi
	sub $8, %esp
	mov 8(%ebp), %edi

	xor %eax, %eax
	mov %eax, (%esp)	# current index
	cpuid

	mov %eax, (%edi)
	# clamp to the size of our cpuid_info structure
	cmp $1, %eax
	jbe 0f
	mov $1, %eax
0:	mov %eax, 4(%esp)	# maximum index

	mov %ebx, 4(%edi)
	mov %edx, 8(%edi)
	mov %ecx, 12(%edi)
	add $16, %edi

cpuid_loop:
	mov (%esp), %eax
	inc %eax
	cmp 4(%esp), %eax
	ja cpuid_done
	mov %eax, (%esp)
	cpuid
	mov %eax, (%edi)
	mov %ebx, 4(%edi)
	mov %edx, 8(%edi)
	mov %ecx, 12(%edi)
	add $16, %edi
	jmp cpuid_loop

cpuid_done:
	add $8, %esp
	pop %edi
	pop %ebx
	pop %ebp
	xor %eax, %eax
	ret

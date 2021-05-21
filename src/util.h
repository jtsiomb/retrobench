#ifndef UTIL_H_
#define UTIL_H_


#ifdef NO_STDINT_H
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long intptr_t;
#else
#include <stdint.h>
#endif

#ifdef __GNUC__
#define INLINE __inline
#define PACKED __attribute__((packed))

#elif defined(__WATCOMC__)
#define INLINE __inline
#define PACKED

#else
#define INLINE
#define PACKED
#endif

#define BSWAP16(x)	((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define BSWAP32(x)	\
	((((x) >> 24) & 0xff) | \
	 (((x) >> 8) & 0xff00) | \
	 (((x) << 8) & 0xff0000) | \
	 ((x) << 24))


extern short sinlut[];

#define SIN(x) (int)sinlut[(x) & 0x7ff]
#define COS(x) (int)sinlut[((x) + 512) & 0x7ff]

int mask_to_shift(unsigned int mask);

#if defined(__i386__) || defined(__x86_64__) || defined(__386__) || defined(MSDOS)
/* fast conversion of double -> 32bit int
 * for details see:
 *  - http://chrishecker.com/images/f/fb/Gdmfp.pdf
 *  - http://stereopsis.com/FPU.html#convert
 */
static INLINE int32_t cround64(double val)
{
	val += 6755399441055744.0;
	return *(int32_t*)&val;
}
#else
#define cround64(x)	((int32_t)(x))
#endif

static INLINE float rsqrt(float x)
{
	float xhalf = x * 0.5f;
	int32_t i = *(int32_t*)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*)&i;
	x = x * (1.5f - xhalf * x * x);
	return x;
}

extern uint32_t perf_start_count, perf_interval_count;

#ifdef __WATCOMC__
void memset16(void *dest, uint16_t val, int count);
#pragma aux memset16 = \
	"cld" \
	"test ecx, 1" \
	"jz memset16_dwords" \
	"rep stosw" \
	"jmp memset16_done" \
	"memset16_dwords:" \
	"shr ecx, 1" \
	"push ax" \
	"shl eax, 16" \
	"pop ax" \
	"rep stosd" \
	"memset16_done:" \
	parm[edi][ax][ecx];

#ifdef USE_MMX
void memcpy64(void *dest, void *src, int count);
#pragma aux memcpy64 = \
	"cploop:" \
	"movq mm0, [edx]" \
	"movq [ebx], mm0" \
	"add edx, 8" \
	"add ebx, 8" \
	"dec ecx" \
	"jnz cploop" \
	"emms" \
	parm[ebx][edx][ecx] \
	modify[8087];
#else
#define memcpy64(dest, src, count)	memcpy(dest, src, (count) << 3)
#endif

void perf_start(void);
#pragma aux perf_start = \
	"xor eax, eax" \
	"cpuid" \
	"rdtsc" \
	"mov [perf_start_count], eax" \
	modify[eax ebx ecx edx];

void perf_end(void);
#pragma aux perf_end = \
	"xor eax, eax" \
	"cpuid" \
	"rdtsc" \
	"sub eax, [perf_start_count]" \
	"mov [perf_interval_count], eax" \
	modify [eax ebx ecx edx];

void debug_break(void);
#pragma aux debug_break = "int 3";
#endif

#ifdef __GNUC__
#if defined(__i386__) || defined(__x86_64__)
#define memset16(dest, val, count) asm volatile ( \
	"cld\n\t" \
	"test $1, %2\n\t" \
	"jz 0f\n\t" \
	"rep stosw\n\t" \
	"jmp 1f\n\t" \
	"0:\n\t" \
	"shr $1, %2\n\t" \
	"push %%ax\n\t" \
	"shl $16, %%eax\n\t" \
	"pop %%ax\n\t" \
	"rep stosl\n\t" \
	"1:\n\t"\
	:: "D"(dest), "a"((uint16_t)(val)), "c"(count) \
	: "memory")
#else
static void INLINE memset16(void *dest, uint16_t val, int count)
{
	uint16_t *ptr = dest;
	while(count--) *ptr++ = val;
}
#endif

#ifdef USE_MMX
#define memcpy64(dest, src, count) asm volatile ( \
	"0:\n\t" \
	"movq (%1), %%mm0\n\t" \
	"movq %%mm0, (%0)\n\t" \
	"add $8, %1\n\t" \
	"add $8, %0\n\t" \
	"dec %2\n\t" \
	"jnz 0b\n\t" \
	"emms\n\t" \
	:: "r"(dest), "r"(src), "r"(count) \
	: "%mm0")
#else
#define memcpy64(dest, src, count)	memcpy(dest, src, (count) << 3)
#endif

#define perf_start()  asm volatile ( \
	"xor %%eax, %%eax\n" \
	"cpuid\n" \
	"rdtsc\n" \
	"mov %%eax, %0\n" \
	: "=m"(perf_start_count) \
	:: "%eax", "%ebx", "%ecx", "%edx")

#define perf_end() asm volatile ( \
	"xor %%eax, %%eax\n" \
	"cpuid\n" \
	"rdtsc\n" \
	"sub %1, %%eax\n" \
	"mov %%eax, %0\n" \
	: "=m"(perf_interval_count) \
	: "m"(perf_start_count) \
	: "%eax", "%ebx", "%ecx", "%edx")

#define debug_break() \
	asm volatile ("int $3")
#endif

#ifdef _MSC_VER
void __inline memset16(void *dest, uint16_t val, int count)
{
	__asm {
		cld
		mov ax, val
		mov edi, dest
		mov ecx, count
		test ecx, 1
		jz memset16_dwords
		rep stosw
		jmp memset16_done
		memset16_dwords:
		shr ecx, 1
		push ax
		shl eax, 16
		pop ax
		rep stosd
		memset16_done:
	}
}

#define perf_start() \
	do { \
		__asm { \
			xor eax, eax \
			cpuid \
			rdtsc \
			mov [perf_start_count], eax \
		} \
	} while(0)

#define perf_end() \
	do { \
		__asm { \
			xor eax, eax \
			cpuid \
			rdtsc \
			sub eax, [perf_start_count] \
			mov [perf_interval_count], eax \
		} \
	} while(0)

#define debug_break() \
	do { \
		__asm { int 3 } \
	} while(0)
#endif

struct cpuid_info {
	uint32_t maxidx;	/* 0: eax */
	char vendor[12];	/* 0: ebx, edx, ecx */
	uint32_t id;		/* 1: eax */
	uint32_t rsvd0;		/* 1: ebx */
	uint32_t feat;		/* 1: edx */
	uint32_t feat2;		/* 1: ecx */
};

#define CPUID_STEPPING(id)	((id) & 0xf)
#define CPUID_MODEL(id)		(((id) >> 4) & 0xf)
#define CPUID_FAMILY(id)	(((id) >> 8) & 0xf)

#define CPUID_FEAT_FPU			0x00000001
#define CPUID_FEAT_VME			0x00000002
#define CPUID_FEAT_DBGEXT		0x00000004
#define CPUID_FEAT_PSE			0x00000008
#define CPUID_FEAT_TSC			0x00000010
#define CPUID_FEAT_MSR			0x00000020
#define CPUID_FEAT_PAE			0x00000040
#define CPUID_FEAT_MCE			0x00000080
#define CPUID_FEAT_CX8			0x00000100
#define CPUID_FEAT_APIC			0x00000200
#define CPUID_FEAT_SEP			0x00000800
#define CPUID_FEAT_MTRR			0x00001000
#define CPUID_FEAT_PGE			0x00002000
#define CPUID_FEAT_MCA			0x00004000
#define CPUID_FEAT_CMOV			0x00008000
#define CPUID_FEAT_PAT			0x00010000
#define CPUID_FEAT_PSE36		0x00020000
#define CPUID_FEAT_PSN			0x00040000
#define CPUID_FEAT_CLF			0x00080000
#define CPUID_FEAT_DTES			0x00200000
#define CPUID_FEAT_ACPI			0x00400000
#define CPUID_FEAT_MMX			0x00800000
#define CPUID_FEAT_FXSR			0x01000000
#define CPUID_FEAT_SSE			0x02000000
#define CPUID_FEAT_SSE2			0x04000000
#define CPUID_FEAT_SS			0x08000000
#define CPUID_FEAT_HTT			0x10000000
#define CPUID_FEAT_TM1			0x20000000
#define CPUID_FEAT_IA64			0x40000000
#define CPUID_FEAT_PBE			0x80000000

#define CPUID_FEAT2_SSE3		0x00000001
#define CPUID_FEAT2_PCLMUL		0x00000002
#define CPUID_FEAT2_DTES64		0x00000004
#define CPUID_FEAT2_MONITOR		0x00000008
#define CPUID_FEAT2_DS_CPL		0x00000010
#define CPUID_FEAT2_VMX			0x00000020
#define CPUID_FEAT2_SMX			0x00000040
#define CPUID_FEAT2_EST			0x00000080
#define CPUID_FEAT2_TM2			0x00000100
#define CPUID_FEAT2_SSSE3		0x00000200
#define CPUID_FEAT2_CID			0x00000400
#define CPUID_FEAT2_FMA			0x00001000
#define CPUID_FEAT2_CX16		0x00002000
#define CPUID_FEAT2_ETPRD		0x00004000
#define CPUID_FEAT2_PDCM		0x00008000
#define CPUID_FEAT2_PCIDE		0x00020000
#define CPUID_FEAT2_DCA			0x00040000
#define CPUID_FEAT2_SSE41		0x00080000
#define CPUID_FEAT2_SSE42		0x00100000
#define CPUID_FEAT2_X2APIC		0x00200000
#define CPUID_FEAT2_MOVBE		0x00400000
#define CPUID_FEAT2_POPCNT		0x00800000
#define CPUID_FEAT2_AES			0x02000000
#define CPUID_FEAT2_XSAVE		0x04000000
#define CPUID_FEAT2_OSXSAVE		0x08000000
#define CPUID_FEAT2_AVX			0x10000000

int read_cpuid(struct cpuid_info *info);

#endif	/* UTIL_H_ */

#include <cpu.h>
#include <screen.h>

/* 
   CPU scanner
   Returns the vendor and the features related to the installed CPU
*/

/*Arrays of possible features of the CPU */
static char* cpuFeaturesEDX[32]={
	"Onboard FPU",
	"V86 Mode Extensions",
	"Debugging Extensions",
	"Page Size Extensions",
	"Timestamp Counter",
	"Model-specific Registers",
	"Physical Address Extension",
	"Machine Check Exception",
	"Compare-and-swap Byte Instruction Support",
	"Onboard APIC",
	"\0",
	"SYSENTER/SYSEXIT Instructions Support",
	"Memory Type Range Registers",
	"Page Global Enable Support",
	"Machine Check Architecture",
	"Conditional Move Instruction Support",
	"Page Attribute Table",
	"36-bit Page Size Extension",
	"Processor Serial Number",
	"SSE2 CLFLUSH Instruction Support",
	"\0",
	"Save Trace of executed jumps",
	"Onboard Thermal Control for ACPI",
	"MMX Support",
	"FXSAVE/FXRESTOR Instruction Support",
	"SSE Support",
	"SSE2 Support",
	"Cache Self-Snoop support",
	"Hyperthreading Support",
	"Temperature-limiter support",
	"IA64 processor emulating x86",
	"Wake-Up Support" 
};

static char* cpuFeaturesECX[32]={
	"SS3 Support",
	"PCLMULQDQ Support",
	"64-bit Debug Store",
	"MONITOR/MWAIT Instruction Support",
	"CPL Qualified Debug Store",
	"Virtual Machine extensions",
	"Safer Mode Extensions",
	"Enhanced SpeedStep",
	"Thermal Monitor 2 Present",
	"Supplemental SSE3 Instructions Support",
	"L1 Context ID",
	"Silicon Debug Interface",
	"FMA3 Support",
	"Compare-and-swap Word Instruction Support",
	"Avoid Send Task Priority Messages Support",
	"Perfmon and Debug Capability",
	"\0",
	"Process Context Identifiers",
	"Direct Cache Access for DMA writes",
	"SSE4.1 Support",
	"SSE4.2 Support",
	"x2APIC Support",
	"Big-endian Move Support",
	"POPCNT Instruction Support",
	"TSC deadline value for APIC one-shot operations",
	"AES Instruction set Available",
	"XSAVE/XRESTOR/XSETBV/XGETBV Instruction Support",
	"XSAVE Enabled by OS",
	"Advanced Vector Extensions",
	"Half-Precision Floating Point Support",
	"Onboard Random Number Generator",
	"Running on Hypervisor"
};

/* Print the CPU Vendor based on EAX=0 function of cpuid */
void getCPUVendor()
{
	static unsigned char vendor[12]={0};
	__asm__ __volatile__("movl $0, %%eax\n"
							"leal (%0),%%edi\n"
							"cpuid\n"
							"movl %%ebx,(%%edi)\n"
							"movl %%edx,4(%%edi)\n"
							"movl %%ecx,8(%%edi)\n":
							"=m" (vendor)::);

	print("CPU Vendor: %s\r\n",vendor);
}

/* Print the list of CPU features based on EAX=1 function of cpuid */
void getCPUFeatures()
{
	int i=0;
	int edxContent=0;
	int ecxContent=0;
	__asm__ __volatile__("movl $1, %%eax\n"
							"cpuid\n"
							"movl %%edx,%0\n"
							"movl %%ecx,%1\n":
							"=m" (edxContent), "=m" (ecxContent)::);

	for(i=0;i<32;i++){
		if((edxContent >> i)&1){
			print("%s\r\n",cpuFeaturesEDX[i]);
		}
	}

	for(i=0;i<32;i++){
		if((ecxContent >> i)&1){
			print("%s\r\n",cpuFeaturesECX[i]);
		}
	}
}

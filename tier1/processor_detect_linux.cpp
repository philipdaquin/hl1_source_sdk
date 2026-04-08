//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: linux dependant ASM code for CPU capability detection
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#ifndef __EMSCRIPTEN__
#define cpuid(in,a,b,c,d)												\
	asm("pushl %%ebx\n\t" "cpuid\n\t" "movl %%ebx,%%esi\n\t" "pop %%ebx": "=a" (a), "=S" (b), "=c" (c), "=d" (d) : "a" (in));
#endif

bool CheckMMXTechnology(void)
{
#ifdef __EMSCRIPTEN__
    return false;
#else
    unsigned long eax,ebx,edx,unused;
    cpuid(1,eax,ebx,unused,edx);

    return edx & 0x800000;
#endif
}

bool CheckSSETechnology(void)
{
#ifdef __EMSCRIPTEN__
    return true;
#else
    unsigned long eax,ebx,edx,unused;
    cpuid(1,eax,ebx,unused,edx);

    return edx & 0x2000000L;
#endif
}

bool CheckSSE2Technology(void)
{
#ifdef __EMSCRIPTEN__
    return true;
#else
    unsigned long eax,ebx,edx,unused;
    cpuid(1,eax,ebx,unused,edx);

    return edx & 0x04000000;
#endif
}

bool Check3DNowTechnology(void)
{
#ifdef __EMSCRIPTEN__
    return false;
#else
    unsigned long eax, unused;
    cpuid(0x80000000,eax,unused,unused,unused);

    if ( eax > 0x80000000L )
    {
     	cpuid(0x80000001,unused,unused,unused,eax);
		return ( eax & 1<<31 );
    }
    return false;
#endif
}

/*
 * CP15 C0 registers
 */
#define	CP15_MIDR(rr)		p15, 0, rr, c0, c0,  0 /* Main ID Register */

#define _FX(s...) #s

#define _RF0(fname, aname...)						\
static __inline uint32_t						\
fname(void)								\
{									\
	uint32_t reg;							\
	__asm __volatile("mrc\t" _FX(aname): "=r" (reg));		\
	return(reg);							\
}

_RF0(cp15_midr_get, CP15_MIDR(%0))

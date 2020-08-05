#ifndef MATH_SH4
#define MATH_SH4
void * memsetfast (void *dest, const uint8_t val, size_t len);
inline float SHFMAC( float a, float b, float c )
{
     register float __FR0 __asm__("fr0") = a; 
     register float __FR1 __asm__("fr1") = b; 
     register float __FR2 __asm__("fr2") = c;      
     
     __asm__ __volatile__( 
        "fmac   fr0, fr1, fr2\n"
        : "=f" (__FR0), "=f" (__FR1), "=f" (__FR2)
        : "0" (__FR0), "1" (__FR1), "2" (__FR2)
        );
        
     return __FR2;       
}

/* SH4 fmac - floating-point multiply/accumulate */
/* Returns a*b+c at the cost of a single floating-point operation */
inline float FMAC( float a, float b, float c )
{
     register float __FR0 __asm__("fr0") = a; 
     register float __FR1 __asm__("fr1") = b; 
     register float __FR2 __asm__("fr2") = c;      
     
     __asm__ __volatile__( 
        "fmac   fr0, fr1, fr2\n"
        : "=f" (__FR0), "=f" (__FR1), "=f" (__FR2)
        : "0" (__FR0), "1" (__FR1), "2" (__FR2)
        );
        
     return __FR2;       
}

/* SH4 fmac - floating-point multiply/decrement */
/* Returns a*b-c at the cost of a single floating-point operation */
inline float FMDC( float a, float b, float c )
{
     register float __FR0 __asm__("fr0") = a; 
     register float __FR1 __asm__("fr1") = b; 
     register float __FR2 __asm__("fr2") = -c;      
     
     __asm__ __volatile__( 
        "fmac   fr0, fr1, fr2\n"
        : "=f" (__FR0), "=f" (__FR1), "=f" (__FR2)
        : "0" (__FR0), "1" (__FR1), "2" (__FR2)
        );
        
     return __FR2;       
}

static inline __attribute__((always_inline)) float FABS(float x)
{
  asm volatile ("fabs %[floatx]\n"
    : [floatx] "+f" (x) // outputs, "+" means r/w
    : // no inputs
    : // no clobbers
  );

  return x;
}
inline float FABS1( float n)
{
    register float __x __asm__("fr15") = n; 
	
	__asm__ __volatile__( 
		"fabs	fr15\n" 
		: "=f" (__x)
		: "0" (__x) );   
		
    return __x; 
}



static inline __attribute__((always_inline)) float FSQRT2(float x)
{
  asm volatile ("fsqrt %[floatx]\n"
    : [floatx] "+f" (x) // outputs, "+" means r/w
    : // no inputs
    : // no clobbers
  );

  return x;
}

// fast sqrt(x)
// Crazy thing: invert(fsrra(x)) is actually about 3x faster than fsqrt.
static inline __attribute__((always_inline)) float  FSQRT(float x)
{
  return  FSQRT(MATH_fsrra(x));
}
inline float FSQRT1( float n)
{
    register float __x __asm__("fr15") = n; 
	
	__asm__ __volatile__( 
		"fsqrt	fr15\n" 
		: "=f" (__x)
		: "0" (__x) );   
		
    return __x; 
}

#endif

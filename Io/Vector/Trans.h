/*
 *  transcendentals.h
 *
 *  A collection of inline AltiVec math functions.
 *
 *  Written in 2003 by Leo Fink (leofink@mac.com).
 *
 **/

inline vector float rsqrt(const vector float& v)
{  
    const vector float _0 = (vector float)(-.0); 
    // obtain estimate
	vector float y = vec_rsqrte(v);
    // one round of Newton-Raphson
	y = vec_madd(vec_nmsub(v,vec_madd(y,y,_0),(vector float)(1.0)),vec_madd(y,(vector float)(0.5),_0),y);
	
	return y;
}

inline vector float sqrt(const vector float& v)
{
    const vector float _0 = (vector float)(-.0);
    return vec_sel(vec_madd(v,rsqrt(v),_0),_0,vec_cmpeq(v,_0));
}

inline vector float log2(const vector float& x) // |error| < 2.3e-6
{
    const vector unsigned int SHIFT = (vector unsigned int)(23);
    const vector unsigned int BIAS = (vector unsigned int)(127);
    
    // extract exponent and mantissa. 1 <= m < 2
    const vector float e = vec_ctf((vector signed int)vec_sub(vec_sr(vec_and((vector unsigned int)(x),(vector unsigned int)(0x7f800000)),SHIFT),BIAS),0);
    vector float m = (vector float)(vec_or(vec_and((vector unsigned int)(x),(vector unsigned int)(0x007FFFFF)),vec_sl(BIAS,SHIFT))); 
    
    // approximate log2(m) by a chebyshev polynomial
    m = vec_sub(m,(vector float)(1.));
    
    vector float y = vec_madd(m,(vector float)(-.0258411662),(vector float)(.1217970128));
    y = vec_madd(m,y,(vector float)(-.2779042655));
    y = vec_madd(m,y,(vector float)(.4575485901));
    y = vec_madd(m,y,(vector float)(-.7181451002));
    y = vec_madd(m,y,(vector float)(1.4425449290));
    
    return vec_madd(m,y,e);	// log2(x) = log2(m)+e
}

inline vector float log(const vector float& x)
{	// ln(x) = log2(x)/ln(2)
    return vec_madd(log2(x),(vector float)(.69314718056),(vector float)(-.0));
}

inline vector float exp2(const vector float& x) // |error|/exp2(x) < 3.4e-6
{	
    // extract integral and fractional part. x=n+r, 0 <= r < 1
    const vector float n = vec_max(vec_floor(x),(vector float)(-127));
    const vector float r = vec_sub(x,n);
    
    // approximate 2^r by a chebyshev polynomial
    vector float y = vec_madd(r,(vector float)(.0135557571),(vector float)(.0520323499));
    y = vec_madd(r,y,(vector float)(.2413797743));
    y = vec_madd(r,y,(vector float)(.6930321187));
    y = vec_madd(r,y,(vector float)(1.0));
    
    // calculate power of integral part by bit-shifting
    const vector float zhn = (vector float)(vec_sl(vec_add((vector signed int)(127),vec_cts(n,0)),(vector unsigned int)(23)));
    
    return vec_madd(y,zhn,(vector float)(-.0)); // 2^x = 2^r*2^n;
}

inline vector float exp(const vector float& x)
{	// exp(x) = 2^(x/ln(2))
    return exp2(vec_madd(x,(vector float)(1.442695041),(vector float)(-.0)));
}

inline vector float pow(const vector float x,const vector float p)
{	// x^p = 2^(p*log2(x))
    return exp2(vec_madd(p,log2(x),(vector float)(-.0)));
}

inline vector float cos(const vector float& x) // |error| < 5e-7
{
    const vector float _0 = (vector float)(-.0);
    const vector float H = (vector float)(.5);
    
    // reduce argument to [0,1[
    vector float r = vec_madd(x,(vector float)(.1591549431),_0);
    r = vec_abs(vec_sub(vec_sub(r,vec_floor(r)),H));
    r = vec_nmsub(r,(vector float)(2),H);
    
    // save sign bit
    vector unsigned int s = vec_andc((vector unsigned int)(0x80000000),(vector unsigned int)(r));
    
    // reduce argument to [0,1/2[
    r = vec_sub(H,vec_abs(r));
    
    // approximate cos(r*pi) by a chebyshev polynomial
    r = vec_madd(r,r,_0);
    vector float y = vec_madd(r,(vector float)(-.2580631862e-1),(vector float)(.2353306303));
    y = vec_madd(y,r,(vector float)(-1.335262769));
    y = vec_madd(y,r,(vector float)(4.058712126));
    y = vec_madd(y,r,(vector float)(-4.934802201));
    y = vec_madd(y,r,(vector float)(1.));
    
    // restore sign
    return (vector float)(vec_xor((vector unsigned int)(y),s));
}


inline vector float sin(const vector float& x) // |error| < 5e-7
{
    const vector float _0 = (vector float)(-.0);
    const vector float H = (vector float)(.5);

    // reduce argument to [0,1[
    vector float r = vec_madd(x,(vector float)(.1591549431),_0);	
    r = vec_sub(vec_sub(r,vec_floor(r)),H);
    
    // save sign bit
    vector unsigned int s = vec_andc((vector unsigned int)(0x80000000),(vector unsigned int)(r));
    
    // reduce argument to [0,1/2[
    r = vec_sub(H,vec_abs(vec_nmsub(vec_abs(r),(vector float)(2),H)));
    
    // approximate sin(r*pi) by a chebyshev polynomial
    const vector float r_ = r;
    r = vec_madd(r,r,_0);
    
    vector float y = vec_madd(r,(vector float)(-.7370292530e-2),(vector float)(.8214588660e-1));
    y = vec_madd(y,r,(vector float)(-.5992645293));
    y = vec_madd(y,r,(vector float)(2.550164040));
    y = vec_madd(y,r,(vector float)(-5.167712780));
    y = vec_madd(y,r,(vector float)(3.141592654));
    y = vec_madd(y,r_,_0);
    
    // restore sign
    return (vector float)(vec_xor((vector unsigned int)(y),s));
}


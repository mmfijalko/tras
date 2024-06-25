/*							polevl.c
 *							p1evl.c
 *
 *	Evaluate polynomial
 *
 *
 *
 * SYNOPSIS:
 *
 * int N;
 * double x, y, coef[N+1], polevl[];
 *
 * y = polevl( x, coef, N );
 *
 *
 *
 * DESCRIPTION:
 *
 * Evaluates polynomial of degree N:
 *
 *                     2          N
 * y  =  C  + C x + C x  +...+ C x
 *        0    1     2          N
 *
 * Coefficients are stored in reverse order:
 *
 * coef[0] = C  , ..., coef[N] = C  .
 *            N                   0
 *
 *  The function p1evl() assumes that coef[N] = 1.0 and is
 * omitted from the array.  Its calling arguments are
 * otherwise the same as polevl().
 *
 *
 * SPEED:
 *
 * In the interest of speed, there are no checks for out
 * of bounds arithmetic.  This routine is used by most of
 * the functions in the library.  Depending on available
 * equipment features, the user may wish to rewrite the
 * program in microcode or assembly language.
 *
 */


/*
Cephes Math Library Release 2.1:  December, 1988
Copyright 1984, 1987, 1988 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/
#include "mconf.h"

#include <stdio.h>
#include <math.h>

int merror = 0;

/* Notice: the order of appearance of the following
 * messages is bound to the error codes defined
 * in mconf.h.
 */
static char *ermsg[7] = {
"unknown",      /* error code 0 */
"domain",       /* error code 1 */
"singularity",  /* et seq.      */
"overflow",
"underflow",
"total loss of precision",
"partial loss of precision"
};


int mtherr( name, code )
char *name;
int code;
{

/* Display string passed by calling program,
 * which is supposed to be the name of the
 * function in which the error occurred:
 */
printf( "\n%s ", name );

/* Set global error message word */
merror = code;

/* Display error message defined
 * by the code argument.
 */
if( (code <= 0) || (code >= 7) )
	code = 0;
printf( "%s error\n", ermsg[code] );

/* Return to calling
 * program
 */
return( 0 );
}

#include <math.h>

double MACHEP =  1.11022302462515654042E-16;   /* 2**-53 */
double MAXLOG =  7.09782712893383996732E2;     /* log(MAXNUM) */

#if 0
#ifdef INFINITIES
double INFINITY = 1.0/0.0;  /* 99e999; */
#else
double INFINITY =  1.79769313486231570815E308;    /* 2**1024*(1-MACHEP) */
#endif
#endif

#if 0
#ifdef NANS
double NAN = 1.0/0.0 - 1.0/0.0;
#else
double NAN = 0.0;
#endif
#endif

double PI     =  3.14159265358979323846;       /* pi */

double polevl( x, coef, N )
double x;
double coef[];
int N;
{
double ans;
int i;
double *p;

p = coef;
ans = *p++;
i = N;

do
	ans = ans * x  +  *p++;
while( --i );

return( ans );
}

/*							p1evl()	*/
/*                                          N
 * Evaluate polynomial when coefficient of x  is 1.0.
 * Otherwise same as polevl.
 */

double p1evl( x, coef, N )
double x;
double coef[];
int N;
{
double ans;
double *p;
int i;

p = coef;
ans = x + *p++;
i = N-1;

do
	ans = ans * x  + *p++;
while( --i );

return( ans );
}

/*							gamma.c
 *
 *	Gamma function
 *
 *
 *
 * SYNOPSIS:
 *
 * double x, y, gamma();
 * extern int sgngam;
 *
 * y = gamma( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns gamma function of the argument.  The result is
 * correctly signed, and the sign (+1 or -1) is also
 * returned in a global (extern) variable named sgngam.
 * This variable is also filled in by the logarithmic gamma
 * function lgam().
 *
 * Arguments |x| <= 34 are reduced by recurrence and the function
 * approximated by a rational function of degree 6/7 in the
 * interval (2,3).  Large arguments are handled by Stirling's
 * formula. Large negative arguments are made positive using
 * a reflection formula.  
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    DEC      -34, 34      10000       1.3e-16     2.5e-17
 *    IEEE    -170,-33      20000       2.3e-15     3.3e-16
 *    IEEE     -33,  33     20000       9.4e-16     2.2e-16
 *    IEEE      33, 171.6   20000       2.3e-15     3.2e-16
 *
 * Error for arguments outside the test range will be larger
 * owing to error amplification by the exponential function.
 *
 */
/*							lgam()
 *
 *	Natural logarithm of gamma function
 *
 *
 *
 * SYNOPSIS:
 *
 * double x, y, lgam();
 * extern int sgngam;
 *
 * y = lgam( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the base e (2.718...) logarithm of the absolute
 * value of the gamma function of the argument.
 * The sign (+1 or -1) of the gamma function is returned in a
 * global (extern) variable named sgngam.
 *
 * For arguments greater than 13, the logarithm of the gamma
 * function is approximated by the logarithmic version of
 * Stirling's formula using a polynomial approximation of
 * degree 4. Arguments between -33 and +33 are reduced by
 * recurrence to the interval [2,3] of a rational approximation.
 * The cosecant reflection formula is employed for arguments
 * less than -33.
 *
 * Arguments greater than MAXLGM return MAXNUM and an error
 * message.  MAXLGM = 2.035093e36 for DEC
 * arithmetic or 2.556348e305 for IEEE arithmetic.
 *
 *
 *
 * ACCURACY:
 *
 *
 * arithmetic      domain        # trials     peak         rms
 *    DEC     0, 3                  7000     5.2e-17     1.3e-17
 *    DEC     2.718, 2.035e36       5000     3.9e-17     9.9e-18
 *    IEEE    0, 3                 28000     5.4e-16     1.1e-16
 *    IEEE    2.718, 2.556e305     40000     3.5e-16     8.3e-17
 * The error criterion was relative when the function magnitude
 * was greater than one but absolute when it was less than one.
 *
 * The following test used the relative error criterion, though
 * at certain points the relative error could be much higher than
 * indicated.
 *    IEEE    -200, -4             10000     4.8e-16     1.3e-16
 *
 */

/*							gamma.c	*/
/*	gamma function	*/

/*
Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1987, 1989, 1992, 2000 by Stephen L. Moshier
*/


static double P[] = {
  1.60119522476751861407E-4,
  1.19135147006586384913E-3,
  1.04213797561761569935E-2,
  4.76367800457137231464E-2,
  2.07448227648435975150E-1,
  4.94214826801497100753E-1,
  9.99999999999999996796E-1
};
static double Q[] = {
-2.31581873324120129819E-5,
 5.39605580493303397842E-4,
-4.45641913851797240494E-3,
 1.18139785222060435552E-2,
 3.58236398605498653373E-2,
-2.34591795718243348568E-1,
 7.14304917030273074085E-2,
 1.00000000000000000320E0
};
#define MAXGAM 171.624376956302725
static double LOGPI = 1.14472988584940017414;

/* Stirling's formula for the gamma function */
static double STIR[5] = {
 7.87311395793093628397E-4,
-2.29549961613378126380E-4,
-2.68132617805781232825E-3,
 3.47222221605458667310E-3,
 8.33333333333482257126E-2,
};
#define MAXSTIR 143.01608
static double SQTPI = 2.50662827463100050242E0;



int sgngam = 0;
extern int sgngam;
#ifdef ANSIPROT
extern int isnan ( double );
extern int isfinite ( double );
double lgam ( double );
#else
int isnan(), isfinite();
static double stirf();
double lgam();
#endif

/* Gamma function computed by Stirling's formula.
 * The polynomial STIR is valid for 33 <= x <= 172.
 */
static double stirf(x)
double x;
{
double y, w, v;

w = 1.0/x;
w = 1.0 + w * polevl( w, STIR, 4 );
y = exp(x);
if( x > MAXSTIR )
	{ /* Avoid overflow in pow() */
	v = pow( x, 0.5 * x - 0.25 );
	y = v * (v / y);
	}
else
	{
	y = pow( x, x - 0.5 ) / y;
	}
y = SQTPI * y * w;
return( y );
}



double gamma(x)
double x;
{
double p, q, z;
int i;

sgngam = 1;
#ifdef NANS
if( isnan(x) )
	return(x);
#endif
#ifdef INFINITIES
#ifdef NANS
if( x == INFINITY )
	return(x);
if( x == -INFINITY )
	return(NAN);
#else
if( !isfinite(x) )
	return(x);
#endif
#endif
q = fabs(x);

if( q > 33.0 )
	{
	if( x < 0.0 )
		{
		p = floor(q);
		if( p == q )
			{
#ifdef NANS
gamnan:
			mtherr( "gamma", DOMAIN );
			return (NAN);
#else
			goto goverf;
#endif
			}
		i = p;
		if( (i & 1) == 0 )
			sgngam = -1;
		z = q - p;
		if( z > 0.5 )
			{
			p += 1.0;
			z = q - p;
			}
		z = q * sin( PI * z );
		if( z == 0.0 )
			{
#ifdef INFINITIES
			return( sgngam * INFINITY);
#else
goverf:
			mtherr( "gamma", OVERFLOW );
			return( sgngam * MAXNUM);
#endif
			}
		z = fabs(z);
		z = PI/(z * stirf(q) );
		}
	else
		{
		z = stirf(x);
		}
	return( sgngam * z );
	}

z = 1.0;
while( x >= 3.0 )
	{
	x -= 1.0;
	z *= x;
	}

while( x < 0.0 )
	{
	if( x > -1.E-9 )
		goto small;
	z /= x;
	x += 1.0;
	}

while( x < 2.0 )
	{
	if( x < 1.e-9 )
		goto small;
	z /= x;
	x += 1.0;
	}

if( x == 2.0 )
	return(z);

x -= 2.0;
p = polevl( x, P, 6 );
q = polevl( x, Q, 7 );
return( z * p / q );

small:
if( x == 0.0 )
	{
#ifdef INFINITIES
#ifdef NANS
	  goto gamnan;
#else
	  return( INFINITY );
#endif
#else
	mtherr( "gamma", SING );
	return( MAXNUM );
#endif
	}
else
	return( z/((1.0 + 0.5772156649015329 * x) * x) );
}



/* A[]: Stirling's formula expansion of log gamma
 * B[], C[]: log gamma function between 2 and 3
 */
#ifdef UNK
static double A[] = {
 8.11614167470508450300E-4,
-5.95061904284301438324E-4,
 7.93650340457716943945E-4,
-2.77777777730099687205E-3,
 8.33333333333331927722E-2
};
static double B[] = {
-1.37825152569120859100E3,
-3.88016315134637840924E4,
-3.31612992738871184744E5,
-1.16237097492762307383E6,
-1.72173700820839662146E6,
-8.53555664245765465627E5
};
static double C[] = {
/* 1.00000000000000000000E0, */
-3.51815701436523470549E2,
-1.70642106651881159223E4,
-2.20528590553854454839E5,
-1.13933444367982507207E6,
-2.53252307177582951285E6,
-2.01889141433532773231E6
};
/* log( sqrt( 2*pi ) ) */
static double LS2PI  =  0.91893853320467274178;
#define MAXLGM 2.556348e305
#endif

/* Logarithm of gamma function */


double lgam(x)
double x;
{
double p, q, u, w, z;
int i;

sgngam = 1;
#ifdef NANS
if( isnan(x) )
	return(x);
#endif

#ifdef INFINITIES
if( !isfinite(x) )
	return(INFINITY);
#endif

if( x < -34.0 )
	{
	q = -x;
	w = lgam(q); /* note this modifies sgngam! */
	p = floor(q);
	if( p == q )
		{
lgsing:
#ifdef INFINITIES
		mtherr( "lgam", SING );
		return (INFINITY);
#else
		goto loverf;
#endif
		}
	i = p;
	if( (i & 1) == 0 )
		sgngam = -1;
	else
		sgngam = 1;
	z = q - p;
	if( z > 0.5 )
		{
		p += 1.0;
		z = p - q;
		}
	z = q * sin( PI * z );
	if( z == 0.0 )
		goto lgsing;
/*	z = log(PI) - log( z ) - w;*/
	z = LOGPI - log( z ) - w;
	return( z );
	}

if( x < 13.0 )
	{
	z = 1.0;
	p = 0.0;
	u = x;
	while( u >= 3.0 )
		{
		p -= 1.0;
		u = x + p;
		z *= u;
		}
	while( u < 2.0 )
		{
		if( u == 0.0 )
			goto lgsing;
		z /= u;
		p += 1.0;
		u = x + p;
		}
	if( z < 0.0 )
		{
		sgngam = -1;
		z = -z;
		}
	else
		sgngam = 1;
	if( u == 2.0 )
		return( log(z) );
	p -= 2.0;
	x = x + p;
	p = x * polevl( x, B, 5 ) / p1evl( x, C, 6);
	return( log(z) + p );
	}

if( x > MAXLGM )
	{
#ifdef INFINITIES
	return( sgngam * INFINITY );
#else
loverf:
	mtherr( "lgam", OVERFLOW );
	return( sgngam * MAXNUM );
#endif
	}

q = ( x - 0.5 ) * log(x) - x + LS2PI;
if( x > 1.0e8 )
	return( q );

p = 1.0/(x*x);
if( x >= 1000.0 )
	q += ((   7.9365079365079365079365e-4 * p
		- 2.7777777777777777777778e-3) *p
		+ 0.0833333333333333333333) / x;
else
	q += polevl( p, A, 4 ) / x;
return( q );
}


/*							igam.c
 *
 *	Incomplete gamma integral
 *
 *
 *
 * SYNOPSIS:
 *
 * double a, x, y, igam();
 *
 * y = igam( a, x );
 *
 * DESCRIPTION:
 *
 * The function is defined by
 *
 *                           x
 *                            -
 *                   1       | |  -t  a-1
 *  igam(a,x)  =   -----     |   e   t   dt.
 *                  -      | |
 *                 | (a)    -
 *                           0
 *
 *
 * In this implementation both arguments must be positive.
 * The integral is evaluated by either a power series or
 * continued fraction expansion, depending on the relative
 * values of a and x.
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0,30       200000       3.6e-14     2.9e-15
 *    IEEE      0,100      300000       9.9e-14     1.5e-14
 */
/*							igamc()
 *
 *	Complemented incomplete gamma integral
 *
 *
 *
 * SYNOPSIS:
 *
 * double a, x, y, igamc();
 *
 * y = igamc( a, x );
 *
 * DESCRIPTION:
 *
 * The function is defined by
 *
 *
 *  igamc(a,x)   =   1 - igam(a,x)
 *
 *                            inf.
 *                              -
 *                     1       | |  -t  a-1
 *               =   -----     |   e   t   dt.
 *                    -      | |
 *                   | (a)    -
 *                             x
 *
 *
 * In this implementation both arguments must be positive.
 * The integral is evaluated by either a power series or
 * continued fraction expansion, depending on the relative
 * values of a and x.
 *
 * ACCURACY:
 *
 * Tested at random a, x.
 *                a         x                      Relative error:
 * arithmetic   domain   domain     # trials      peak         rms
 *    IEEE     0.5,100   0,100      200000       1.9e-14     1.7e-15
 *    IEEE     0.01,0.5  0,100      200000       1.4e-13     1.6e-15
 */

/*
Cephes Math Library Release 2.8:  June, 2000
Copyright 1985, 1987, 2000 by Stephen L. Moshier
*/

static double big = 4.503599627370496e15;
static double biginv =  2.22044604925031308085e-16;

double igam(double a, double x);

double igamc( a, x )
double a, x;
{
double ans, ax, c, yc, r, t, y, z;
double pk, pkm1, pkm2, qk, qkm1, qkm2;

if( (x <= 0) || ( a <= 0) )
	return( 1.0 );

if( (x < 1.0) || (x < a) )
	return( 1.0 - igam(a,x) );

ax = a * log(x) - x - lgam(a);
if( ax < -MAXLOG )
	{
	mtherr( "igamc", UNDERFLOW );
	return( 0.0 );
	}
ax = exp(ax);

/* continued fraction */
y = 1.0 - a;
z = x + y + 1.0;
c = 0.0;
pkm2 = 1.0;
qkm2 = x;
pkm1 = x + 1.0;
qkm1 = z * x;
ans = pkm1/qkm1;

do
	{
	c += 1.0;
	y += 1.0;
	z += 2.0;
	yc = y * c;
	pk = pkm1 * z  -  pkm2 * yc;
	qk = qkm1 * z  -  qkm2 * yc;
	if( qk != 0 )
		{
		r = pk/qk;
		t = fabs( (ans - r)/r );
		ans = r;
		}
	else
		t = 1.0;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;
	if( fabs(pk) > big )
		{
		pkm2 *= biginv;
		pkm1 *= biginv;
		qkm2 *= biginv;
		qkm1 *= biginv;
		}
	}
while( t > MACHEP );

return( ans * ax );
}



/* left tail of incomplete gamma function:
 *
 *          inf.      k
 *   a  -x   -       x
 *  x  e     >   ----------
 *           -     -
 *          k=0   | (a+k+1)
 *
 */

double igam( a, x )
double a, x;
{
double ans, ax, c, r;

if( (x <= 0) || ( a <= 0) )
	return( 0.0 );

if( (x > 1.0) && (x > a ) )
	return( 1.0 - igamc(a,x) );

/* Compute  x**a * exp(-x) / gamma(a)  */
ax = a * log(x) - x - lgam(a);
if( ax < -MAXLOG )
	{
	mtherr( "igam", UNDERFLOW );
	return( 0.0 );
	}
ax = exp(ax);

/* power series */
r = a;
c = 1.0;
ans = 1.0;

do
	{
	r += 1.0;
	c *= x/r;
	ans += c;
	}
while( c/ans > MACHEP );

return( ans * ax/a );
}

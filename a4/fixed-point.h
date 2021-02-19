#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#define FIX_BITS 32
#define FIX_P 14   
#define FIX_Q 17               
#define FIX_F (1 << FIX_Q)      
#define FIX_MIN_INT (-FIX_MAX_INT)      
#define FIX_MAX_INT ((1 << FIX_P) - 1) 

typedef struct
{
  int f;
}
fixed_point;

//storing f in struct
static inline fixed_point
make_fp (int f)
{
  fixed_point x;
  x.f = f;
  return x;
}

//stores a fp whose value is n
static inline fixed_point
fp_int (int n)
{
  ASSERT (n >= FIX_MIN_INT && n <= FIX_MAX_INT);
  return make_fp (n * FIX_F);
}

//stores a fp whose value is n/d
static inline fixed_point
fp_frac (int n, int d)
{
  ASSERT (d != 0);
  ASSERT (n / d >= FIX_MIN_INT && n / d <= FIX_MAX_INT);
  return make_fp ((long long) n * FIX_F / d);
}

//rounding x to nearest int
static inline int
fp_round (fixed_point x)
{
  return (x.f + FIX_F / 2) / FIX_F;
}

//truncates x to nearest int
static inline int
fp_trunc (fixed_point x)
{
  return x.f / FIX_F;
}

//add fps
static inline fixed_point
fp_add (fixed_point x, fixed_point y)
{
  return make_fp (x.f + y.f);
}

//subract fps
static inline fixed_point
fp_sub (fixed_point x, fixed_point y)
{
  return make_fp (x.f - y.f);
}

//multiply fps
static inline fixed_point
fp_mul (fixed_point x, fixed_point y)
{
  return make_fp ((long long) x.f * y.f / FIX_F);
}

//multiply fp and int
static inline fixed_point
fp_scale (fixed_point x, int n)
{
  ASSERT (n >= 0);
  return make_fp (x.f * n);
}

//divide fps
static inline fixed_point
fp_div (fixed_point x, fixed_point y)
{
  return make_fp ((long long) x.f * FIX_F / y.f);
}

//divide fp and int
static inline fixed_point
fp_unscale (fixed_point x, int n)
{
  ASSERT (n > 0);
  return make_fp (x.f / n);
}

//reciprocal of fp
static inline fixed_point
fp_inv (fixed_point x)
{
  return fp_div (fp_int (1), x);
}

//compares fp
static inline int
fp_compare (fixed_point x, fixed_point y)
{
  return x.f < y.f ? -1 : x.f > y.f;
}

#endif

#pragma once

namespace avCore
{

// HALF limits
const float HALF_MIN      = 5.96046448e-08f ;  // Smallest positive half
const float HALF_NRM_MIN  = 6.10351562e-05f ;  // Smallest positive normalized half
const float HALF_MAX      = 65504.0f        ;  // Largest positive half
const float HALF_EPSILON  = 0.00097656f     ;  // Smallest positive e for which half(1.0 + e) != half(1.0)
const int   HALF_MANT_DIG = 11              ;  // Number of digits in mantissa (significant + hidden leading 1)
const int   HALF_DIG      = 2               ;  // Number of base 10 digits that can be represented without change
const int   HALF_RADIX    = 2               ;  // Base of the exponent
const int   HALF_MIN_EXP  = -13             ;  // Minimum negative integer such that HALF_RADIX raised to the power of
                                               // one less than that integer is a normalized half
const int   HALF_MAX_EXP  = 16              ;  // Maximum positive integer such that HALF_RADIX raised to the power of
                                               // one less than that integer is a normalized half
const int   HALF_MIN_10_EXP = -4            ;  // Minimum positive integer such that 10 raised to that power is a normalized half
const int   HALF_MAX_10_EXP = 4             ;  // Maximum positive integer such that 10 raised to that power is a normalized half

// HALF class
#pragma pack(push, 1)

class half
{
public:

   // float-to-uint union
   union uif
   {
      unsigned int i;
      float f;
   };

   // Constructors
   __forceinline half()
      : _h( 0 )
   {}

   __forceinline half(half const & h)
      : _h( h._h )
   {}

   template < class S >
      __forceinline half(S f)
   {
      init(static_cast< float >(f));
   }

   // Conversion to float
   __forceinline operator float() const
   {
      static _half_to_float_helper _toFloat;

      return _toFloat[_h];
   }

   // Unary minus
   __forceinline half operator - () const
   {
      half h;
      h._h = _h ^ 0x8000;
      return h;
   }

   // Assignment
   __forceinline half & operator = (half const & h)
   {
      _h = h._h;
      return *this;
   }
   __forceinline half & operator = (float f)
   {
      *this = half(f);
      return *this;
   }

   // Addition with assignment
   __forceinline half & operator += (half h)
   {
      *this = half(float(*this) + float(h));
      return *this;
   }
   __forceinline half & operator += (float f)
   {
      *this = half(float(*this) + f);
      return *this;
   }

   // Subtraction with assignment
   __forceinline half & operator -= (half h)
   {
      *this = half(float(*this) - float(h));
      return *this;
   }
   __forceinline half & operator -= (float f)
   {
      *this = half(float(*this) - f);
      return *this;
   }

   // Multiplication with assignment
   __forceinline half & operator *= (half h)
   {
      *this = half(float(*this) * float(h));
      return *this;
   }
   __forceinline half & operator *= (float f)
   {
      *this = half(float(*this) * f);
      return *this;
   }

   // Division with assignment
   __forceinline half & operator /= (half h)
   {
      *this = half(float(*this) / float(h));
      return *this;
   }
   __forceinline half & operator /= (float f)
   {
      *this = half(float(*this) / f);
      return *this;
   }


   // Round to n-bit precision (n should be between 0 and 10).
   __forceinline half round(unsigned int n) const
   {
      if (n >= 10) {
         return *this;
      }
      unsigned short s = _h & 0x8000;
      unsigned short e = _h & 0x7fff;
      e >>= 9 - n;
      e  += e & 1;
      e <<= 9 - n;
      if (e >= 0x7c00)
      {
         e = _h;
         e >>= 10 - n;
         e <<= 10 - n;
      }
      half h;
      h._h = s | e;
      return h;
   }

   // Access to the internal representation
   __forceinline unsigned short getBits() const
   {
      return _h;
   }
   __forceinline void setBits(unsigned short bits)
   {
      _h = bits;
      return;
   }

private:

   __forceinline void init(float f)
   {
      static _float_to_half_helper _eLut;

      if (f == 0.f)
         _h = 0;
      else
      {
         uif x;
         x.f = f;
         register int e = (x.i >> 23) & 0x000001ff;
         e = _eLut[e];
         if (e)
            _h = e + (((x.i & 0x007fffff) + 0x00001000) >> 13);
         else
            _h = convert(x.i);
      }
      return;
   }

   // exact float-to-half conversion routine
   static __forceinline unsigned short convert(int i)
   {
      register int s =  (i >> 16) & 0x00008000;
      register int e = ((i >> 23) & 0x000000ff) - (127 - 15);
      register int m =   i        & 0x007fffff;

      if (e <= 0)
      {
         if (e < -10)
            return 0;
         m = (m | 0x00800000) >> (1 - e);
         if (m & 0x00001000)
            m += 0x00002000;
         return s | (m >> 13);
      }
      else if (e == 0xff - (127 - 15))
      {
         if (m == 0)
            return s | 0x7c00;
         else
         {
            m >>= 13;
            return s | 0x7c00 | m | (m == 0);
         }
      }
      else
      {
         if (m & 0x00001000)
         {
            m += 0x00002000;
            if (m & 0x00800000) {
               m = 0;
               e += 1;
            }
         }
         if (e > 30)
            return s | 0x7c00;
         return s | (e << 10) | (m >> 13);
      }
   }

   // storage data, 16 bits
   unsigned short _h;

   // helper for float-to-half conversion
   struct _float_to_half_helper
   {
      __forceinline _float_to_half_helper()
      {
         fractional_lut = new unsigned short [1 << 9];
         for (unsigned int i = 0; i < 0x100; i++)
         {
            int e = (i & 0x0ff) - (127 - 15);
            if (e <= 0 || e >= 30)
            {
               fractional_lut[i]         = 0;
               fractional_lut[i | 0x100] = 0;
            }
            else
            {
               fractional_lut[i]         =  (e << 10);
               fractional_lut[i | 0x100] = ((e << 10) | 0x8000);
            }
         }
      }
      __forceinline ~_float_to_half_helper()
      {
         delete [] fractional_lut;
      }

      __forceinline unsigned short operator [] (int index) const
      {
         return fractional_lut[index];
      }

   private:
      unsigned short * fractional_lut;
   };

   // helper for half-to-float conversion
   struct _half_to_float_helper
   {
      __forceinline _half_to_float_helper()
      {
         float_lut = new uif [1 << 16];
         for (unsigned int i = 0; i < 0x10000; i++)
         {
            unsigned short y = (unsigned short)i;
            int s = (y >> 15) & 0x00000001;
            int e = (y >> 10) & 0x0000001f;
            int m =  y        & 0x000003ff;
            if (e == 0)
            {
               if (m == 0)
               {
                  float_lut[i].i = s << 31;
                  continue;
               }
               else
               {
                  while (!(m & 0x00000400))
                  {
                     m <<= 1;
                     e -=  1;
                  }
                  e += 1;
                  m &= ~0x00000400;
               }
            }
            else if (e == 31)
            {
               if (m == 0)
                  float_lut[i].i = (s << 31) | 0x7f800000;
               else
                  float_lut[i].i = (s << 31) | 0x7f800000 | (m << 13);
               continue;
            }
            e = e + (127 - 15);
            m = m << 13;
            float_lut[i].i = (s << 31) | (e << 23) | m;
         }
      }
      __forceinline ~_half_to_float_helper()
      {
         delete [] float_lut;
      }

      __forceinline float operator [] (unsigned short index) const
      {
         return float_lut[index].f;
      }

   private:
      uif * float_lut;
   };
};

#pragma pack(pop)

} // end of avCore namespace
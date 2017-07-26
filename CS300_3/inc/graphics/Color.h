#ifndef H_COLOR
#define H_COLOR

#include "framework/Utilities.h"

namespace Graphics
{
  // This is a structure representing a 32-bit RGBA color. Each color channel
  // is stored as a flaot within the structure. The structure is designed so
  // that an instance of Color may be converted to a byte array and uploaded
  // directly to the graphics card with RGBA format; no conversion is
  // necessary. It also has functionality to be converted to a 32-bit integral
  // ARGB packed structure, another acceptable format by OpenGL.
  #pragma pack(1)
  struct Color
  {
    union
    {
      #pragma pack(1)
      struct
      {
        f32 r; /* color's red chanel */
        f32 g; /* color's green chanel */
        f32 b; /* color's blue chanel */
        f32 a; /* color's alpha chanel */
      };
      f32 components[4]; /* r: components[0], g: components[1], etc. */
    };

    static Color const Red;     /* R=1.0, G=0.0, B=0.0 */
    static Color const Green;   /* R=0.0, G=1.0, B=0.0 */
    static Color const Blue;    /* R=0.0, G=0.0, B=1.0 */
    static Color const Yellow;  /* R=1.0, G=1.0, B=0.0 */
    static Color const Cyan;    /* R=0.0, G=1.0, B=1.0 */
    static Color const Magenta; /* R=1.0, G=0.0, B=1.0 */
    static Color const White;   /* R=1.0, G=1.0, B=1.0 */
    static Color const Black;   /* R=0.0, G=0.0, B=0.0 */
    static Color const Gray;    /* R=0.5, G=0.5, B=0.5 */

    explicit Color(f32 r = 0.f, f32 g = 0.f, f32 b = 0.f, f32 a = 1.f);
    explicit Color(u32 argb);
    Color(Color const &rhs);

    // Retrieves the packed integral ARGB representation of this color. For
    // example, the constant Magenta above would have an ARGB value of:
    // 0xFFFF00FF and green would be 0xFF00FF00.
    u32 GetARGB() const;

    // Retrieves a float pointer to this color, allowing random access of its
    // color data. The first element is R, second G, third B, and fourth A.
    f32 *ToFloats();

    // Retrieves a float pointer to this color, allowing random access of its
    // color data. The first element is R, second G, third B, and fourth A.
    f32 const *ToFloats() const;

    // Component-wise multiplies two colors together. That is:
    // Color result = Color(left.r * right.r, left.g * right.g, ...)
    // This saves the result within this color.
    Color &operator*=(Color const &rhs);

    // Component-wise adds two colors together. That is:
    // Color result = Color(left.r + right.r, left.g + right.g, ...)
    // This saves the result within this color.
    Color &operator+=(Color const &rhs);

    // Component-wise multiplies two colors together. That is:
    // Color result = Color(left.r * right.r, left.g * right.g, ...)
    Color operator*(Color const &rhs) const;

    // Component-wise adds two colors together. That is:
    // Color result = Color(left.r + right.r, left.g + right.g, ...)
    Color operator+(Color const &rhs) const;
  };
}

#endif

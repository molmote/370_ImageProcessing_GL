#include "Precompiled.h"
#include "graphics/Color.h"

// These are useful macros for extracting the red, green, blue, and alpha float
// values out of the ARGB integral representation of a color.
#define GETA(iv) (static_cast<f32>((iv >> 24) & 0xff) / 255.f)
#define GETR(iv) (static_cast<f32>((iv >> 16) & 0xff) / 255.f)
#define GETG(iv) (static_cast<f32>((iv >> 8) & 0xff) / 255.f)
#define GETB(iv) (static_cast<f32>(iv & 0xff) / 255.f)

// These are useful macros for storing the red, green, blue, and alpha float
// values within the ARGB integral representation of a color.
#define SETA(fv) ((static_cast<u32>(fv * 255.f) & 0xff) << 24U)
#define SETR(fv) ((static_cast<u32>(fv * 255.f) & 0xff) << 16U)
#define SETG(fv) ((static_cast<u32>(fv * 255.f) & 0xff) << 8U)
#define SETB(fv) (static_cast<u32>(fv * 255.f) & 0xff)

namespace Graphics
{
  // These are constant colors that may be helpful while developing any
  // assignments using the framework.
  Color const Color::Red(1, 0, 0);
  Color const Color::Green(0, 1, 0);
  Color const Color::Blue(0, 0, 1);
  Color const Color::Yellow(1, 1, 0);
  Color const Color::Cyan(0, 1, 1);
  Color const Color::Magenta(1, 0, 1);
  Color const Color::White(1, 1, 1);
  Color const Color::Black(0, 0, 0);
  Color const Color::Gray(0.5f, 0.5f, 0.5f);

  Color::Color(f32 _r/* = 0.f*/, f32 _g/* = 0.f*/, f32 _b/* = 0.f*/,
    f32 _a/* = 1.f*/)
    : r(_r), g(_g), b(_b), a(_a)
  {
  }

  Color::Color(u32 argb)
    : r(GETR(argb)), g(GETG(argb)), b(GETB(argb)), a(GETA(argb))
  {
  }

  Color::Color(Color const &rhs)
    : r(rhs.r), g(rhs.g), b(rhs.b), a(rhs.a)
  {
  }

  // Convert the 4 component floats to a single, integral ARGB representation.
  u32 Color::GetARGB() const
  {
    return SETA(a) | SETR(r) | SETG(g) | SETB(b);
  }

  // Converts this color to a contiguous aray of floats.
  f32 *Color::ToFloats()
  {
    return reinterpret_cast<f32 *>(this);
  }

  // Converts this color to a contiguous aray of floats.
  f32 const *Color::ToFloats() const
  {
    return reinterpret_cast<f32 const *>(this);
  }

  Color &Color::operator*=(Color const &rhs)
  {
    // component-wise multiplication
    r *= rhs.r;
    g *= rhs.g;
    b *= rhs.b;
    a *= rhs.a;
    return *this;
  }

  Color &Color::operator+=(Color const &rhs)
  {
    // component-wise addition
    r += rhs.r;
    g += rhs.g;
    b += rhs.b;
    a += rhs.a;
    return *this;
  }

  Color Color::operator*(Color const &rhs) const
  {
    // multiply to a copy
    return Color(r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a);
  }

  Color Color::operator+(Color const &rhs) const
  {
    // add to a copy
    return Color(r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a);
  }
}

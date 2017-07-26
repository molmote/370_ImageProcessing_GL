#ifndef H_UTILITIES
#define H_UTILITIES

// From CS250's Utilities.h

// Typedefs for consistency (used sparingly throughout the framework...you do
// not have to use these at all).
typedef unsigned char      u8;
typedef char               s8;
typedef unsigned int       u32;
typedef int                s32;
typedef unsigned long long u64;
typedef long long          s64;
typedef float              f32;
typedef double             f64;

// This is the path used to locate assets (shaders and models) by the
// ShaderProgram and MeshLoader loaders. This path is dictated by the premake
// script and is always kept relative to the VS project directory.
#ifndef ASSET_PATH
#define ASSET_PATH "../"
#endif

#endif
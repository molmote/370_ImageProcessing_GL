#version 330

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 FragmentColor;
 

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
   uniform vec2 res;
uniform int shaderflag;
uniform float TIME;
float sum = 0;


highp float rand(float n){
    return fract(sin(mod(dot(n ,12.9898),3.14))*43758.5453);
}

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }
// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

float sigma = 3;     // The sigma value for the gaussian function: higher value means more blur
                     // A good value for 9x9 is around 3 to 5
                     // A good value for 7x7 is around 2.5 to 4
                     // A good value for 5x5 is around 2 to 3.5
                     // ... play around with this based on what you need :)



const float pi = 3.14159265f;
float blurSize = 1.0f / 640;  // This should usually be equal to
                              // 1.0f / texture_pixel_width for a horizontal blur, and
                              // 1.0f / texture_pixel_height for a vertical blur.
vec4 Blur(int size)
{
    float numBlurPixelsPerSide = size - 1;
    vec2 blurMultiplyVec = vec2(1.0f, 1.0f);

    // Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
    vec3 incrementalGaussian;
    incrementalGaussian.x = 1.0f / (sqrt(2.0f * pi) * sigma);
    incrementalGaussian.y = exp(-0.5f / (sigma * sigma));
    incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;

    vec4 avgValue = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    float coefficientSum = 0.0f;

    // Take the central sample first...
    avgValue += texture(myTextureSampler, UV) * incrementalGaussian.x;
    coefficientSum += incrementalGaussian.x;
    incrementalGaussian.xy *= incrementalGaussian.yz;

    // Go through the remaining 8 vertical samples (4 on each side of the center)
    for (float i = 1.0f; i <= numBlurPixelsPerSide; i++)
    {
        avgValue += texture(myTextureSampler, UV - i * blurSize *
                              blurMultiplyVec) * incrementalGaussian.x;
        avgValue += texture(myTextureSampler, UV + i * blurSize *
                              blurMultiplyVec) * incrementalGaussian.x;
        coefficientSum += 2 * incrementalGaussian.x;
        incrementalGaussian.xy *= incrementalGaussian.yz;
    }

    return avgValue / coefficientSum;
}

float MYMIN(float x, float y, float z)
{
    if (x < y)
    {
        if (x < z)
        {
            return x;
        }
        if (x >= z)
        {
            return z;
        }
    }
    if (x >= y)
    {
        if (y < z)
        {
            return y;
        }
        if (y >= z)
        {
            return z;
        }
    }
    return x;
}

float MYMAX(float x, float y, float z)
{
    if (x > y)
    {
        if (x > z)
        {
            return x;
        }
        if (x <= z)
        {
            return z;
        }
    }
    if (x <= y)
    {
        if (y > z)
        {
            return y;
        }
        if (y <= z)
        {
            return z;
        }
    }
    return x;
}
//H: 0-360
//S: 0-1
//V: 0-1
vec3 RGBtoHSV(vec3 RGB)
{
  float min, max, delta;
  vec3 HSV = vec3(0);
  min = MYMIN(RGB.x, RGB.y, RGB.z);
  max = MYMAX(RGB.x, RGB.y, RGB.z);
  HSV.z = max;				// V
  delta = max - min;
  if (max > 0)
    HSV.y = delta / max;		// S
  if (max <= 0)
  {
    HSV.y = 0; //S
    HSV.x = 0; //H
    return HSV;
  }
  if (RGB.x == max) //Red highest
    HSV.x = (RGB.y - RGB.z) / delta;
  else if (RGB.y == max) //Green highest
    HSV.x = 2 + (RGB.z - RGB.x) / delta;
  else //Blue highest
    HSV.x = 4 + (RGB.x - RGB.y) / delta;
  HSV.x *= 60;				// Convert hue into degrees
  if (HSV.x < 0)
    HSV.x += 360;
  return HSV;
}

vec3 HSVtoRGB(vec3 HSV)
{
  int i;
  float f, p, q, t;
  vec3 RGB = vec3(0);
  
  if (HSV.x < 0.0)
    HSV.x += 360.0;
  if (HSV.x >= 360.0)
    HSV.x -= 359.0;
  
  if (HSV.y < 0.0)
    HSV.y = 0.0;
  if (HSV.y > 1.0)
    HSV.y = 1.0;
  
  if (HSV.z < 0.0)
    HSV.z = 0.0;
  if (HSV.z > 1.0)
    HSV.z = 1.0;
  
  if (HSV.y == 0)
  {
    // achromatic (grey)
    RGB.x = RGB.y = RGB.z = HSV.z;
    return RGB;
  }
  HSV.x /= 60;			// sector 0 to 5
  i = int(HSV.x);
  f = HSV.x - i;			// factorial part of h
  p = HSV.z * (1 - HSV.y);
  q = HSV.z * (1 - HSV.y * f);
  t = HSV.z * (1 - HSV.y * (1 - f));
  if (i == 0)
  {
    RGB.x = HSV.z;
    RGB.y = t;
    RGB.z = p;
  }
  else if (i == 1)
  {
    RGB.x = q;
    RGB.y = HSV.z;
    RGB.z = p;
  }
  else if (i == 2)
  {
    RGB.x = p;
    RGB.y = HSV.z;
    RGB.z = t;
  }
  else if (i == 3)
  {
    RGB.x = p;
    RGB.y = q;
    RGB.z = HSV.z;
  }
  else if (i == 4)
  {
    RGB.x = t;
    RGB.y = p;
    RGB.z = HSV.z;
  }
  else if (i == 5)
  {
    RGB.x = HSV.z;
    RGB.y = p;
    RGB.z = q;
  }
  return RGB;
}

float aastep(float threshold, float value)
{
#ifdef GL_OES_standard_derivatives
    float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
#else
    float afwidth = frequency * (1.0 / 200.0) / uScale / cos(uYrot);
#endif
    return smoothstep(threshold - afwidth, threshold + afwidth, value);
}


// Explicit bilinear texture lookup to circumvent bad hardware precision.
// The extra arguments specify the dimension of the texture. (GLSL 1.30
// introduced textureSize() to get that information from the sampler.)
// 'dims' is the width and height of the texture, 'one' is 1.0/dims.
// (Precomputing 'one' saves two divisions for each lookup.)
// vec3 texcolor = texture2D_bilinear(myTextureSampler, UV, res, vOne).rgb;
vec4 texture2D_bilinear(sampler2D tex, vec2 st, vec2 dims, vec2 one)
{
    vec2 uv = st * dims;
    vec2 uv00 = floor(uv - vec2(0.5)); // Lower left corner of lower left texel
    vec2 uvlerp = uv - uv00 - vec2(0.5); // Texel-local lerp blends [0,1]
    vec2 st00 = (uv00 + vec2(0.5)) * one;
    vec4 texel00 = texture(tex, st00);
    vec4 texel10 = texture(tex, st00 + vec2(one.x, 0.0));
    vec4 texel01 = texture(tex, st00 + vec2(0.0, one.y));
    vec4 texel11 = texture(tex, st00 + one);
    vec4 texel0 = mix(texel00, texel01, uvlerp.y);
    vec4 texel1 = mix(texel10, texel11, uvlerp.y);
    return mix(texel0, texel1, uvlerp.x);
}

vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x * 34.0) + 1.0) * x); }

float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                       -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
                                            // First corner
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    // Other corners
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
                             + i.x + vec3(0.0, i1.x, 1.0));
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy),
                            dot(x12.zw, x12.zw)), 0.0);
    m = m * m; m = m * m;
    // Gradients
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 a0 = x - floor(x + 0.5);
    // Normalise gradients implicitly by scaling m
    m *= 1.792843 - 0.853735 * (a0 * a0 + h * h);
    // Compute final noise value at P
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

void main(){

	// Output color = color of the texture at the specified UV
	FragmentColor = texture( myTextureSampler, UV ).rgba;

	//int flag = shaderflag;

    float offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0);
    float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216,
                                       0.0540540541, 0.0162162162);
    // Uniform Blur
    if ((shaderflag & int(1)) == int(1))
    {
        FragmentColor = Blur(2);
    }

    // Bloom or Glow effect
    if ((shaderflag & int(4)) == int(4))
    {
        FragmentColor += Blur(21);
        FragmentColor += Blur(7);
        FragmentColor += Blur(3);
    }


    // Additive Noise
    if ((shaderflag & int(8)) == int(8))
    {
        vec3 input_Time = vec3( UV, TIME);
        float e = random(input_Time);
        vec3 luma = vec3 (e);
        FragmentColor= FragmentColor+vec4(luma, e);
    }

    //varying 
    // RGB TO HSV
    if ((shaderflag & int(16)) == int(16))
    {
      vec3 TempColor = RGBtoHSV(FragmentColor.rgb);
      TempColor.x = 0.0;
      TempColor = HSVtoRGB(TempColor);
      FragmentColor = vec4(TempColor,1.0);
    }


    // Scratched Fild Effect
    if ((shaderflag & int(32)) == int(32))
    {
    }

    // Sepia
    // Sepia : Hue 0-40 from dankest to lightest
    if ((shaderflag & int(64)) == int(64))
    {
      vec3 TempColor = RGBtoHSV(FragmentColor.rgb);
      TempColor.z = TempColor.z - TempColor.y*0.25;
      TempColor.x = TempColor.z*40.0;
      TempColor = HSVtoRGB(TempColor);
      FragmentColor = vec4(TempColor,1.0);
    }
    
    // Greyscale
    if ((shaderflag & int(128)) == int(128))
    {
      vec3 TempColor = RGBtoHSV(FragmentColor.rgb);
      TempColor.z -= TempColor.y*0.5;
      TempColor.y = 0.0;
      TempColor = HSVtoRGB(TempColor);
      FragmentColor = vec4(TempColor,1.0);
    }

    // Hue Change
    // for this one I Need another key for sepia / Black&White / Gray scale toggle
    if ((shaderflag & int(512)) == int(512))
    {
      float hue = 0.0;
      vec3 TempColor = RGBtoHSV(FragmentColor.rgb);
      hue = TempColor.x;
      hue += 60.0;
      TempColor = HSVtoRGB(vec3(hue,TempColor.yz));
      FragmentColor = vec4(TempColor,1.0);
    }
    
    // Hue Change
    // for this one I Need another key for sepia / Black&White / Gray scale toggle
    if ((shaderflag & int(1024)) == int(1024))
    {
      float hue = 0.0;
      vec3 TempColor = RGBtoHSV(FragmentColor.rgb);
      hue = TempColor.x;
      hue += 120.0;
      TempColor = HSVtoRGB(vec3(hue,TempColor.yz));
      FragmentColor = vec4(TempColor,1.0);
    }
    
    // Hue Change
    // for this one I Need another key for sepia / Black&White / Gray scale toggle
    if ((shaderflag & int(2048)) == int(2048))
    {
      float hue = 0.0;
      vec3 TempColor = RGBtoHSV(FragmentColor.rgb);
      hue = TempColor.x;
      hue += 180.0;
      TempColor = HSVtoRGB(vec3(hue,TempColor.yz));
      FragmentColor = vec4(TempColor,1.0);
    }
    
    // Hue Change
    // for this one I Need another key for sepia / Black&White / Gray scale toggle
    if ((shaderflag & int(4096)) == int(4096))
    {
      float hue = 0.0;
      vec3 TempColor = RGBtoHSV(FragmentColor.rgb);
      hue = TempColor.x;
      hue += 240.0;
      TempColor = HSVtoRGB(vec3(hue,TempColor.yz));
      FragmentColor = vec4(TempColor,1.0);
    }
    
    // Hue Change
    // for this one I Need another key for sepia / Black&White / Gray scale toggle
    if ((shaderflag & int(8192)) == int(8192))
    {
      float hue = 0.0;
      vec3 TempColor = RGBtoHSV(FragmentColor.rgb);
      hue = TempColor.x;
      hue += 300.0;
      TempColor = HSVtoRGB(vec3(hue,TempColor.yz));
      FragmentColor = vec4(TempColor,1.0);
    }

    
    /*uniform */
    float uScale = 10; // For imperfect, isotropic anti-aliasing in
                      /*uniform */
    float uYrot = 10;  // absence of dFdx() and dFdy() functions
    float frequency = 40.0; // Needed globally for lame version of aastep()

    // HalfTone
    if ((shaderflag & int(256)) == int(256))
    {
        vec3 texcolor = texture(myTextureSampler, UV).rgb; // Unrotated coords

        float n = 0.1 * snoise(UV * 200.0); // Fractal noise
        n += 0.05 * snoise(UV * 400.0);
        n += 0.025 * snoise(UV * 800.0);
        vec3 white = vec3(n * 0.2 + 0.97);
        vec3 black = vec3(n + 0.1);

        // Perform a rough RGB-to-CMYK conversion
        vec4 cmyk;
        cmyk.xyz = 1.0 - texcolor;
        cmyk.w = min(cmyk.x, min(cmyk.y, cmyk.z)); // Create K
        cmyk.xyz -= cmyk.w; // Subtract K equivalent from CMY

        // Distance to nearest point in a grid of
        // (frequency x frequency) points over the unit square
        vec2 Kst = frequency * mat2(0.707, -0.707, 0.707, 0.707) * UV;
        vec2 Kuv = 2.0 * fract(Kst) - 1.0;
        float k = aastep(0.0, sqrt(cmyk.w) - length(Kuv) + n);
        vec2 Cst = frequency * mat2(0.966, -0.259, 0.259, 0.966) * UV;
        vec2 Cuv = 2.0 * fract(Cst) - 1.0;
        float c = aastep(0.0, sqrt(cmyk.x) - length(Cuv) + n);
        vec2 Mst = frequency * mat2(0.966, 0.259, -0.259, 0.966) * UV;
        vec2 Muv = 2.0 * fract(Mst) - 1.0;
        float m = aastep(0.0, sqrt(cmyk.y) - length(Muv) + n);
        vec2 Yst = frequency * UV; // 0 deg
        vec2 Yuv = 2.0 * fract(Yst) - 1.0;
        float y = aastep(0.0, sqrt(cmyk.z) - length(Yuv) + n);

        vec3 rgbscreen = 1.0 - 0.9 * vec3(c, m, y) + n;
        rgbscreen = mix(rgbscreen, black, 0.85 * k + 0.3 * n);

        FragmentColor = vec4(rgbscreen, 1.0);
    }

    // Depth Of Field
    if ((shaderflag & int(2)) == int(2))
    {
        // uniform sampler2D bgl_RenderedTexture;
        // uniform sampler2D bgl_DepthTexture;

        const float blurclamp = 3.0;  // max blur amount
        const float bias = 0.6; //aperture - bigger values for shallower depth of field
        //uniform float focus;  // this value comes from ReadDepth script.
        float focus = 1.0f;

        float aspectratio = 800.0 / 600.0;
        vec2 aspectcorrect = vec2(1.0, aspectratio);

        // vec4 depth1   = texture2D(bgl_DepthTexture,gl_TexCoord[0].xy );
        vec4 depth1 = texture(myTextureSampler, UV);

        float factor = (depth1.x - focus);

        vec2 dofblur = vec2(clamp(factor * bias, -blurclamp, blurclamp));


        vec4 col = vec4(0.0);

        col += texture(myTextureSampler, UV);
        col += texture(myTextureSampler, UV + (vec2(0.0, 0.4) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(0.15, 0.37) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(0.29, 0.29) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(-0.37, 0.15) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(0.4, 0.0) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(0.37, -0.15) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(0.29, -0.29) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(-0.15, -0.37) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(0.0, -0.4) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(-0.15, 0.37) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(-0.29, 0.29) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(0.37, 0.15) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(-0.4, 0.0) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(-0.37, -0.15) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur);
        col += texture(myTextureSampler, UV + (vec2(0.15, -0.37) * aspectcorrect) * dofblur);

        col += texture(myTextureSampler, UV + (vec2(0.15, 0.37) * aspectcorrect) * dofblur * 0.9);
        col += texture(myTextureSampler, UV + (vec2(-0.37, 0.15) * aspectcorrect) * dofblur * 0.9);
        col += texture(myTextureSampler, UV + (vec2(0.37, -0.15) * aspectcorrect) * dofblur * 0.9);
        col += texture(myTextureSampler, UV + (vec2(-0.15, -0.37) * aspectcorrect) * dofblur * 0.9);
        col += texture(myTextureSampler, UV + (vec2(-0.15, 0.37) * aspectcorrect) * dofblur * 0.9);
        col += texture(myTextureSampler, UV + (vec2(0.37, 0.15) * aspectcorrect) * dofblur * 0.9);
        col += texture(myTextureSampler, UV + (vec2(-0.37, -0.15) * aspectcorrect) * dofblur * 0.9);
        col += texture(myTextureSampler, UV + (vec2(0.15, -0.37) * aspectcorrect) * dofblur * 0.9);

        col += texture(myTextureSampler, UV + (vec2(0.29, 0.29) * aspectcorrect) * dofblur * 0.7);
        col += texture(myTextureSampler, UV + (vec2(0.4, 0.0) * aspectcorrect) * dofblur * 0.7);
        col += texture(myTextureSampler, UV + (vec2(0.29, -0.29) * aspectcorrect) * dofblur * 0.7);
        col += texture(myTextureSampler, UV + (vec2(0.0, -0.4) * aspectcorrect) * dofblur * 0.7);
        col += texture(myTextureSampler, UV + (vec2(-0.29, 0.29) * aspectcorrect) * dofblur * 0.7);
        col += texture(myTextureSampler, UV + (vec2(-0.4, 0.0) * aspectcorrect) * dofblur * 0.7);
        col += texture(myTextureSampler, UV + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur * 0.7);
        col += texture(myTextureSampler, UV + (vec2(0.0, 0.4) * aspectcorrect) * dofblur * 0.7);

        col += texture(myTextureSampler, UV + (vec2(0.29, 0.29) * aspectcorrect) * dofblur * 0.4);
        col += texture(myTextureSampler, UV + (vec2(0.4, 0.0) * aspectcorrect) * dofblur * 0.4);
        col += texture(myTextureSampler, UV + (vec2(0.29, -0.29) * aspectcorrect) * dofblur * 0.4);
        col += texture(myTextureSampler, UV + (vec2(0.0, -0.4) * aspectcorrect) * dofblur * 0.4);
        col += texture(myTextureSampler, UV + (vec2(-0.29, 0.29) * aspectcorrect) * dofblur * 0.4);
        col += texture(myTextureSampler, UV + (vec2(-0.4, 0.0) * aspectcorrect) * dofblur * 0.4);
        col += texture(myTextureSampler, UV + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur * 0.4);
        col += texture(myTextureSampler, UV + (vec2(0.0, 0.4) * aspectcorrect) * dofblur * 0.4);

        FragmentColor = col / 41.0;
        FragmentColor.a = 1.0;
    }
}
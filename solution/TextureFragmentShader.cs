#version 330

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 FragmentColor;
 

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform int shaderflag;
uniform float TIME;
float sum = 0;

float sigma = 3;     // The sigma value for the gaussian function: higher value means more blur
                         // A good value for 9x9 is around 3 to 5
                         // A good value for 7x7 is around 2.5 to 4
                         // A good value for 5x5 is around 2 to 3.5
                         // ... play around with this based on what you need :)

float blurSize = 1.0f / 640;  // This should usually be equal to
                         // 1.0f / texture_pixel_width for a horizontal blur, and
                         // 1.0f / texture_pixel_height for a vertical blur.

// uniform sampler2D blurSampler;  // Texture that will be blurred by this shader

const float pi = 3.14159265f;

#define BLUR_5 0;

// The following are all mutually exclusive macros for various 
// seperable blurs of varying kernel size
#if defined(VERTICAL_BLUR_9)
const float numBlurPixelsPerSide = 4.0f;
const vec2  blurMultiplyVec      = vec2(0.0f, 1.0f);
#elif defined(HORIZONTAL_BLUR_9)
const float numBlurPixelsPerSide = 4.0f;
const vec2  blurMultiplyVec      = vec2(1.0f, 0.0f);
#elif defined(VERTICAL_BLUR_7)
const float numBlurPixelsPerSide = 3.0f;
const vec2  blurMultiplyVec      = vec2(0.0f, 1.0f);
#elif defined(HORIZONTAL_BLUR_7)
const float numBlurPixelsPerSide = 3.0f;
const vec2  blurMultiplyVec      = vec2(1.0f, 0.0f);
#elif defined(VERTICAL_BLUR_5)
const float numBlurPixelsPerSide = 2.0f;
const vec2  blurMultiplyVec      = vec2(0.0f, 1.0f);
#elif defined(HORIZONTAL_BLUR_5)
const float numBlurPixelsPerSide = 2.0f;
const vec2  blurMultiplyVec      = vec2(1.0f, 0.0f);
#elif defined(BLUR_5)
const float numBlurPixelsPerSide = 2.0f;
const vec2  blurMultiplyVec      = vec2(1.0f, 1.0f);
#else
// This only exists to get this shader to compile when no macros are defined
const float numBlurPixelsPerSide = 0.0f;
const vec2 blurMultiplyVec = vec2(0.0f, 0.0f);
#endif

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
        FragmentColor = texture(myTextureSampler, vec2(gl_FragCoord) / 1024.0) * weight[0];
        for (int i = 1; i < 5; i++)
        {
            FragmentColor +=
                texture(myTextureSampler, (vec2(gl_FragCoord) + vec2(0.0, offset[i])) / 1024.0)
                    * weight[i];
            FragmentColor +=
                texture(myTextureSampler, (vec2(gl_FragCoord) - vec2(0.0, offset[i])) / 1024.0)
                    * weight[i];
        }
    }


    if ((shaderflag & int(4)) == int(4))
    {
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

        FragmentColor = avgValue / coefficientSum;
    }


    // Additive Noise
    if ((shaderflag & int(8)) == int(8))
    {
        vec3 input_Time = vec3( UV, TIME);
        float e = random(input_Time);
        vec3 luma = vec3 (e);
        FragmentColor= FragmentColor+vec4(luma, e);
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
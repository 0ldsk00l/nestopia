/*
MIT License

CRTea - Configurable CRT Fragment Shader
Copyright (c) 2020-2022 Rupert Carmichael

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// Based on Public Domain work by Timothy Lottes

precision highp float;

uniform sampler2D source;
uniform vec4 sourceSize;
uniform vec4 targetSize;

uniform int masktype; // Mask Type
uniform float maskstr; // Mask Strength
uniform float scanstr; // Scanline Strength
uniform float sharpness; // Pixel Sharpness
uniform float curve; // Screen Curvature
uniform float corner; // Screen Curvature
uniform float tcurve; // Trinitron Curve

in vec2 texCoord;

out vec4 FragColor;

//#define CRTS_DEBUG 1 // Define to see on/off split screen
//#define CRTS_2_TAP 1 // Faster very pixely 2-tap filter (off is 8)
#define CRTS_WARP 1 // Apply screen warp

#define INPUT_MASK (1.0 - maskstr)
#define INPUT_SCAN (0.5 + (0.5 * scanstr))
#define INPUT_SHRP (-1.0 * sharpness)
#define INPUT_X sourceSize.x
#define INPUT_Y sourceSize.y

#define CURVATURE curve
#define TRINITRON_CURVE tcurve
#define CORNER corner
#define CRT_GAMMA 2.4
#define CRTS_TONE 1 // Normalize mid-level and process color
#define CRTS_CONTRAST 0 // Process color - enable contrast control
#define CRTS_SATURATION 0 // Process color - enable saturation control

#define CrtsRcpF1(x) (1.0/(x))
#define CrtsSatF1(x) clamp((x),0.0,1.0)

// sRGB-Linear Conversions
float FromSrgb1(float c) {
    return (c <= 0.04045) ? c * (1.0 / 12.92) :
        pow(c * (1.0 / 1.055) + (0.055 / 1.055), CRT_GAMMA);
}

vec3 FromSrgb(vec3 c) {
    return vec3(FromSrgb1(c.r), FromSrgb1(c.g), FromSrgb1(c.b));
}

float ToSrgb1(float c) {
    return (c < 0.0031308 ? c * 12.92 : 1.055 * pow(c, 0.41666) - 0.055);
}

vec3 ToSrgb(vec3 c) {
    return vec3(ToSrgb1(c.r), ToSrgb1(c.g), ToSrgb1(c.b));
}

// Fetch the input image colour
vec3 CrtsFetch(vec2 uv) {
    // Scale to get native texels in the image
    uv *= vec2(INPUT_X, INPUT_Y) / sourceSize.xy;
    return FromSrgb(texture(source, uv.xy, -16.0).rgb);
}

// Get the maximum of 3 floats
float CrtsMax3F1(float a, float b, float c) {
    return max(a, max(b, c));
}

// Tonal Control
vec4 CrtsTone(float contrast, float saturation, float thin, float mask) {
    if (masktype == 0) mask = 1.0;

    if (masktype == 1) {
        // Normal R mask is {1.0,mask,mask}
        // LITE   R mask is {mask,1.0,1.0}
        mask = 0.5 + mask * 0.5;
    }

    vec4 ret;
    float midOut = 0.18 / ((1.5 - thin) * (0.5 * mask + 0.5));
    float pMidIn = pow(0.18, contrast);
    ret.x = contrast;
    ret.y = ((-pMidIn) + midOut) / ((1.0 - pMidIn) * midOut);
    ret.z = ((-pMidIn) * midOut + pMidIn) / (midOut * (-pMidIn) + midOut);
    ret.w = contrast + saturation;
    return ret;
}

// Apply the mask
vec3 CrtsMask(vec2 pos, float dark) {
    if (masktype == 0) { // Scanlines (No Mask)
        return vec3(1.0, 1.0, 1.0);
    }

    if (masktype == 1) { // Aperture Grille Lite
        vec3 m = vec3(1.0, 1.0, 1.0);
        float x = fract(pos.x * (1.0 / 3.0));
        if (x < (1.0 / 3.0)) m.r = dark;
        else if (x < (2.0 / 3.0)) m.g = dark;
        else m.b = dark;
        return m;
    }

    if (masktype == 2) { // Aperture Grille
        vec3 m = vec3(dark, dark, dark);
        float x = fract(pos.x * (1.0 / 3.0));
        if (x < (1.0 / 3.0)) m.r = 1.0;
        else if (x < (2.0 / 3.0)) m.g = 1.0;
        else m.b = 1.0;
        return m;
    }

    if (masktype == 3) { // Shadow Mask
        pos.x += pos.y * 2.9999;
        vec3 m = vec3(dark, dark, dark);
        float x = fract(pos.x * (1.0 / 6.0));
        if (x < (1.0 / 3.0)) m.r = 1.0;
        else if (x < (2.0 / 3.0)) m.g = 1.0;
        else m.b = 1.0;
        return m;
    }
}

// The filter function itself
    // SV_POSITION, fragCoord.xy
    // sourceSize / targetSize (in pixels)
    // 0.5 * sourceSize (in pixels)
    // 1.0 / sourceSize (in pixels)
    // 1.0 / targetSize (in pixels)
    // 2.0 / targetSize (in pixels)
    // Warp scanlines but not phosphor mask
    // Tonal curve parameters generated by CrtsTone()
vec3 CrtsFilter(vec2 ipos, vec2 inputSizeDivOutputSize, vec2 halfInputSize,
    vec2 rcpInputSize, vec2 rcpOutputSize, vec2 twoDivOutputSize,
    vec2 warp, vec4 tone) {
    #ifdef CRTS_DEBUG
    vec2 uv = ipos * rcpOutputSize;

    // Show second half processed, and first half un-processed
    if (uv.x < 0.5) {
        // Force nearest to get squares
        uv *= 1.0 / rcpInputSize;
        uv = floor(uv) + vec2(0.5, 0.5);
        uv *= rcpInputSize;
        vec3 color = CrtsFetch(uv);
        return color;
    }
    #endif

    // Optional apply warp
    vec2 pos;

    #ifdef CRTS_WARP
    // Convert to {-1 to 1} range
    pos = ipos * twoDivOutputSize - vec2(1.0, 1.0);

    // Distort pushes image outside {-1 to 1} range
    pos *= vec2(1.0 + (pos.y * pos.y) * warp.x, 1.0 + (pos.x * pos.x) * warp.y);

    // TODO: Vignette needs optimization
    float vin = (1.0 -
        ((1.0 - CrtsSatF1(pos.x * pos.x)) * (1.0 - CrtsSatF1(pos.y * pos.y)))) *
        (0.998 + (0.001 * CORNER));
    vin = CrtsSatF1((-vin) * sourceSize.y + sourceSize.y);

    // Leave in {0 to inputSize}
    pos = pos * halfInputSize + halfInputSize;
    #else
    pos = ipos * inputSizeDivOutputSize;
    #endif

    // Snap to center of first scanline
    float y0 = floor(pos.y - 0.5) + 0.5;

    #ifdef CRTS_2_TAP
    // Using Inigo's "Improved Texture Interpolation"
    // http://iquilezles.org/www/articles/texture/texture.htm
    pos.x += 0.5;
    float xi = floor(pos.x);
    float xf = pos.x - xi;
    xf = xf * xf * xf * (xf * (xf * 6.0 - 15.0) + 10.0);
    float x0 = xi + xf - 0.5;
    vec2 p = vec2(x0 * rcpInputSize.x, y0 * rcpInputSize.y);

    // Coordinate adjusted bilinear fetch from 2 nearest scanlines
    vec3 colA = CrtsFetch(p);
    p.y += rcpInputSize.y;
    vec3 colB = CrtsFetch(p);
    #else
    // Snap to center of one of four pixels
    float x0 = floor(pos.x - 1.5) + 0.5;

    // Inital UV position
    vec2 p = vec2(x0 * rcpInputSize.x, y0 * rcpInputSize.y);

    // Fetch 4 nearest texels from 2 nearest scanlines
    vec3 colA0 = CrtsFetch(p);
    p.x += rcpInputSize.x;
    vec3 colA1 = CrtsFetch(p);
    p.x += rcpInputSize.x;
    vec3 colA2 = CrtsFetch(p);
    p.x += rcpInputSize.x;
    vec3 colA3 = CrtsFetch(p);
    p.y += rcpInputSize.y;
    vec3 colB3 = CrtsFetch(p);
    p.x -= rcpInputSize.x;
    vec3 colB2 = CrtsFetch(p);
    p.x -= rcpInputSize.x;
    vec3 colB1 = CrtsFetch(p);
    p.x -= rcpInputSize.x;
    vec3 colB0 = CrtsFetch(p);
    #endif

    // Vertical filter
    // Scanline intensity is using sine wave
    // Easy filter window and integral used later in exposure
    float off = pos.y - y0;
    float pi2 = 6.28318530717958;
    float hlf = 0.5;
    float scanA = cos(min(0.5, off * INPUT_SCAN) * pi2) * hlf + hlf;
    float scanB = cos(min(0.5, (-off) * INPUT_SCAN + INPUT_SCAN) * pi2) *
        hlf + hlf;

    #ifdef CRTS_2_TAP
        #ifdef CRTS_WARP
        // Get rid of wrong pixels on edge
        scanA *= vin;
        scanB *= vin;
        #endif
    // Apply vertical filter
    vec3 color = (colA * scanA) + (colB * scanB);
    #else
    // Horizontal kernel is simple gaussian filter
    float off0 = pos.x - x0;
    float off1 = off0 - 1.0;
    float off2 = off0 - 2.0;
    float off3 = off0 - 3.0;
    float pix0 = exp2(INPUT_SHRP * off0 * off0);
    float pix1 = exp2(INPUT_SHRP * off1 * off1);
    float pix2 = exp2(INPUT_SHRP * off2 * off2);
    float pix3 = exp2(INPUT_SHRP * off3 * off3);
    float pixT = CrtsRcpF1(pix0 + pix1 + pix2 + pix3);
        #ifdef CRTS_WARP
        // Get rid of wrong pixels on edge
        pixT *= vin;
        #endif
    scanA *= pixT;
    scanB *= pixT;

    // Apply horizontal and vertical filters
    vec3 color =
        (colA0 * pix0 + colA1 * pix1 + colA2 * pix2 + colA3 * pix3) * scanA +
        (colB0 * pix0 + colB1 * pix1 + colB2 * pix2 + colB3 * pix3) * scanB;
    #endif

    // Apply phosphor mask
    color *= CrtsMask(gl_FragCoord.xy, INPUT_MASK);

    // Optional color processing
    #ifdef CRTS_TONE
    // Tonal control, start by protecting from /0
    float peak = max(1.0 / (256.0 * 65536.0),
        CrtsMax3F1(color.r, color.g, color.b));

    // Compute the ratios of {R,G,B}
    vec3 ratio = color * CrtsRcpF1(peak);

    // Apply tonal curve to peak value
    #ifdef CRTS_CONTRAST
        peak = pow(peak, tone.x);
    #endif
    peak = peak * CrtsRcpF1(peak * tone.y + tone.z);

    // Apply saturation
    #ifdef CRTS_SATURATION
        ratio = pow(ratio, vec3(tone.w, tone.w, tone.w));
    #endif

    // Reconstruct color
    return ratio * peak;
    #else
    return color;
    #endif
}

void main() {
    vec2 warp_factor;
    warp_factor.x = CURVATURE;
    warp_factor.y = (3.0 / 4.0) * warp_factor.x; // assume 4:3 aspect
    warp_factor.x *= (1.0 - TRINITRON_CURVE);

    FragColor.rgb = CrtsFilter(texCoord.xy * targetSize.xy,
        sourceSize.xy * targetSize.zw,
        sourceSize.xy * vec2(0.5, 0.5),
        sourceSize.zw,
        targetSize.zw,
        2.0 * targetSize.zw,
        warp_factor,
        CrtsTone(1.0, 0.0, INPUT_SCAN, INPUT_MASK));

    // Output non-linear color
    FragColor.rgb = ToSrgb(FragColor.rgb);
}

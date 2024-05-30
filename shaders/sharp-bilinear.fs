/*
MIT License

Sharp Bilinear (Modified)
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
// Based on Public Domain work by Themaister

precision highp float;

uniform sampler2D source;
uniform vec4 sourceSize;
uniform vec4 targetSize;

in vec2 texCoord;

out vec4 fragColor;

void main() {
    float prescale = floor(targetSize.y / sourceSize.y + 0.01);
    if (prescale == 0.0) prescale = 1.0;
    vec2 texel = texCoord.xy * sourceSize.xy;
    vec2 texel_floored = floor(texel);
    vec2 s = fract(texel);
    float region_range = 0.5 - 0.5 / prescale;

   /* Figure out where in the texel to sample to get correct pre-scaled
      bilinear. Uses the hardware bilinear interpolator to avoid having to
      sample 4 times manually.
   */
    vec2 center_dist = s - 0.5;
    vec2 f = (center_dist - clamp(center_dist, -region_range, region_range)) *
        prescale + 0.5;
    vec2 mod_texel = texel_floored + f;
    fragColor = texture(source, mod_texel / sourceSize.xy);
}

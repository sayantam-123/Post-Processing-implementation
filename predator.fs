#version 330

in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform vec2 resolution;
uniform float time;  // optional, for scanline animation
uniform int show_scan;



out vec4 finalColor;

vec3 heatmap(float t) {
    vec3 cold   = vec3(0.1, 0.0, 0.3);
    vec3 medium = vec3(1.0, 0.0, 0.0);
    vec3 hot    = vec3(1.0, 1.0, 0.0);

    if (t < 0.5) return mix(cold, medium, t * 2.0);
    else         return mix(medium, hot, (t - 0.5) * 2.0);
}

void main() {
    vec2 texel = 1.0 / resolution;

    // --- luminance of current pixel ---
    vec3 col = texture(texture0, fragTexCoord).rgb;
    float lum = dot(col, vec3(0.299, 0.587, 0.114));

    // --- sobel on luminance ---
    float tl = dot(texture(texture0, fragTexCoord + vec2(-texel.x, -texel.y)).rgb, vec3(0.299, 0.587, 0.114));
    float t  = dot(texture(texture0, fragTexCoord + vec2( 0.0,     -texel.y)).rgb, vec3(0.299, 0.587, 0.114));
    float tr = dot(texture(texture0, fragTexCoord + vec2( texel.x, -texel.y)).rgb, vec3(0.299, 0.587, 0.114));
    float l  = dot(texture(texture0, fragTexCoord + vec2(-texel.x,  0.0    )).rgb, vec3(0.299, 0.587, 0.114));
    float r  = dot(texture(texture0, fragTexCoord + vec2( texel.x,  0.0    )).rgb, vec3(0.299, 0.587, 0.114));
    float bl = dot(texture(texture0, fragTexCoord + vec2(-texel.x,  texel.y)).rgb, vec3(0.299, 0.587, 0.114));
    float b  = dot(texture(texture0, fragTexCoord + vec2( 0.0,      texel.y)).rgb, vec3(0.299, 0.587, 0.114));
    float br = dot(texture(texture0, fragTexCoord + vec2( texel.x,  texel.y)).rgb, vec3(0.299, 0.587, 0.114));

    float gx = -tl - 2.0*l - bl + tr + 2.0*r + br;
    float gy = -tl - 2.0*t - tr + bl + 2.0*b + br;
    float edge = clamp(sqrt(gx*gx + gy*gy), 0.0, 1.0);

    // --- combine ---
    vec3 heat = heatmap(lum);
    vec3 edgeColor = vec3(1.0, 1.0, 1.0);  // white edges

    vec3 result = mix(heat, edgeColor, edge * 0.8);

    if (show_scan == 1) {
        float scanline = sin(fragTexCoord.y * resolution.y * 3.14159 + time*3) * 0.04;
        result += scanline;
    }

    float scanline = sin(fragTexCoord.y * resolution.y * 3.14159 + time*3) * 0.04;
    result += scanline;


    finalColor = vec4(result, 1.0);
}

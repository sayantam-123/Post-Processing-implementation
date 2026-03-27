#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;

out vec4 finalColor;

void main(){
    vec2 texel = 1.0f/resolution;

    vec3 color = texture(texture0, fragTexCoord + vec2(-texel.x, -texel.y)).rgb;
    float tl = dot(color, vec3(0.299, 0.587, 0.114));
    color = texture(texture0, fragTexCoord + vec2(0, -texel.y)).rgb;
    float t = dot(color, vec3(0.299, 0.587, 0.114));
    color = texture(texture0, fragTexCoord + vec2(texel.x, -texel.y)).rgb;
    float tr = dot(color, vec3(0.299, 0.587, 0.114));
    color = texture(texture0, fragTexCoord + vec2(-texel.x, 0)).rgb;
    float l = dot(color, vec3(0.299, 0.587, 0.114));
    color = texture(texture0, fragTexCoord + vec2(texel.x, 0)).rgb;
    float r = dot(color, vec3(0.299, 0.587, 0.114));
    color = texture(texture0, fragTexCoord + vec2(-texel.x, texel.y)).rgb;
    float bl = dot(color, vec3(0.299, 0.587, 0.114));
    color = texture(texture0, fragTexCoord + vec2(0, texel.y)).rgb;
    float b = dot(color, vec3(0.299, 0.587, 0.114));
    color = texture(texture0, fragTexCoord + vec2(texel.x, texel.y)).rgb;
    float br = dot(color, vec3(0.299, 0.587, 0.114));

    float gx = -tl - 2.0*l - bl + tr + 2.0*r + br;
    float gy = -tl - 2.0*t - tr + bl + 2.0*b + br;

    float edge = sqrt(gx*gx + gy*gy);

    finalColor = vec4(vec3(edge), 1.0);



}

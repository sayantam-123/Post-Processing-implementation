#version 330


in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float time;
uniform float strength; // 0.05 to 0.1 is good

out vec4 finalColor;

void main(){
    vec3 color = texture(texture0, fragTexCoord).rgb;

    float grain = fract(sin(dot(fragTexCoord, vec2(12.9898, 78.233))) * 43758.5453 + time);

    // remap grain from [0,1] to [-0.5, 0.5] so it adds and subtracts equally
    color += (grain - 0.5) * strength;

    finalColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}

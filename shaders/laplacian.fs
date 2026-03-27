#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;

out vec4 finalColor;

void main(){
    vec2 texel = 1.0f/resolution;

    vec3 color;

    vec3 t = texture(texture0, fragTexCoord + vec2(0, -texel.y)).rgb;
    vec3 l = texture(texture0, fragTexCoord + vec2(-texel.x, 0)).rgb;
    vec3 r = texture(texture0, fragTexCoord + vec2(texel.x, 0)).rgb;
    vec3 b = texture(texture0, fragTexCoord + vec2(0, texel.y)).rgb;
    vec3 c = texture(texture0, fragTexCoord).rgb;

    vec3 laplace = -t -l -r -b + 4*c;

    vec3 sharpened = c + laplace;

    finalColor = vec4(clamp(sharpened,0.0,1.0), 1.0);

}

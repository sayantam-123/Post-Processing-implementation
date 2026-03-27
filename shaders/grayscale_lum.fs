#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;

out vec4 finalColor;

void main(){

    vec3 curr_tex = texture(texture0, fragTexCoord).rgb;
    float y = 0.299f*curr_tex.r + 0.587f*curr_tex.g + 0.114f*curr_tex.b;

    finalColor = vec4(vec3(y),1.0);

}

#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;

out vec4 finalColor;

void main(){
    vec3 curr_tex = texture(texture0, fragTexCoord).rgb;

    finalColor = vec4(vec3(1.0 - curr_tex.r, 1.0 - curr_tex.g, 1.0 - curr_tex.b), 1.0);

}

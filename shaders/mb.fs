#version 330
in vec2 fragTexCoord;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform mat4 currentVPInverse;
uniform mat4 previousVP;
uniform int numSamples;
out vec4 finalColor;

void main(){
    float depth = texture(texture2, fragTexCoord).r;

    vec4 H = vec4(
        fragTexCoord.x * 2.0 - 1.0,
        (1.0 - fragTexCoord.y) * 2.0 - 1.0,
        depth * 2.0 - 1.0,    // fixed
        1.0
    );

    vec4 D = currentVPInverse * H;
    vec4 worldPos = D / D.w;

    vec4 previousPos = previousVP * worldPos;
    previousPos /= previousPos.w;

    vec2 velocity = (H.xy - previousPos.xy) * 0.5;

    float maxVel = 0.02;
    velocity = clamp(velocity, -maxVel, maxVel);

    vec2 uv = fragTexCoord;
    vec4 color = texture(texture1, uv);
    float count = 1.0;
    uv += velocity;

    for(int i = 1; i < numSamples; i++, uv += velocity){
        if(uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) break;
        color += texture(texture1, uv);
        count += 1.0;
    }

    finalColor = color / count;
}

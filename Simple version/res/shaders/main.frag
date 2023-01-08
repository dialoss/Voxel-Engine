#version 330

out vec4 fragColor;
in float textInd;
in vec2 textCoord;
in vec3 vColor;
in float visibility;

uniform sampler2DArray texture_array;
uniform vec3 skyColor;
//uniform sampler2D ourTexture;

void main(){
    //texture(ourTexture, textCoord);
    vec4 color = texture(texture_array, vec3(textCoord, textInd)) * vec4(vColor, 1);
    if (color.a < 0.6f)
       discard;
    color = mix(vec4(skyColor, 1.0f), color, visibility);
    fragColor = color;
}
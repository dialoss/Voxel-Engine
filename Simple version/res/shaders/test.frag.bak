#version 330

out vec4 fragColor;
in float textInd;
in vec2 textCoord;

uniform sampler2DArray texture_array;

void main(){
    fragColor = texture(texture_array, vec3(textCoord, textInd));
}
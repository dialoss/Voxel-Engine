#version 330

layout (location = 0) in uvec3 vPos;

out vec2 textCoord;
out float textInd;
out vec3 vColor;
out float visibility;

uniform mat4 view;
uniform mat4 proj;

const float density = 0.005f;
uniform float gradient;

// gl_Position = projection * view * model * vec4(aPos, 1.0f);

void main(){
    float tx =  float((vPos.z & 0x80000000u) >> 31u);
    float ty =  float((vPos.z & 0x40000000u) >> 30u);
    int ind = int((vPos.z & 0x3FF00000u) >> 20u);
    float z =   float((vPos.z & 0xFFFFFu));
    float x =   float((vPos.x & 0xFFFFFFu));
    float y =   float((vPos.y & 0xFFu));
    x -= 5e5;
    z -= 5e5;
    float d = 4.0f;
    float r = (float((vPos.y & 0xFF000000u) >> 24u)) / d / 15.0f;
    float g = (float((vPos.y & 0xFF0000u) >> 16u)) / d / 15.0f;
    float b = (float((vPos.y & 0xFF00u) >> 8u)) / d / 15.0f;
    float s = (float((vPos.x & 0xFF000000u) >> 24u)) / d / 15.0f;
    textCoord = vec2(tx, ty);
    textInd = ind;
    if (ind == 19)
        y -= 0.1f;
    vColor = vec3(r, g, b);
    s += 0.1f;
    vColor.rgb += s * 0.95f;
    vColor.r = min(vColor.r, 1.5f);
    vColor.g = min(vColor.g, 1.5f);
    vColor.b = min(vColor.b, 1.5f);
    vec4 worldPos = view * vec4(x, y, z, 1);

    float distance = length(worldPos.xyz);
    gl_Position = proj * worldPos;
    visibility = exp(-pow((density * distance), gradient));
    visibility = clamp(visibility, 0.0f, 1.0f);
}

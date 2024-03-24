#version 330
#extension GL_ARB_compatibility: enable

in vec2 fTexCoord;
in vec3 fTexCoord_cube, cameraNormal, cameraPos;
out vec4 fColor;

uniform samplerCube skybox;

void main() 
{ 
    vec3 I = normalize(cameraPos);
    vec3 R = reflect(I, normalize(cameraNormal));

    fColor = texture(skybox, R);
} 


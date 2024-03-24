#version 330

in vec2 fTexCoord;
in vec3 fTexCoord_cube, worldNormal, worldPos, cameraPos;
out vec4 fColor;

uniform samplerCube skybox;

void main() 
{ 
    fColor = texture(skybox, fTexCoord_cube);
} 


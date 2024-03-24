#version 330

in vec2 fTexCoord;
in vec3 fTexCoord_cube, worldNormal, worldPos, cameraPos;
out vec4 fColor;

uniform samplerCube skybox;

void main() 
{ 
    vec3 I = normalize(worldPos - cameraPos);
    vec3 R = reflect(I, normalize(worldNormal));
    // vec3 R = 2 * dot(I, normalize(worldNormal)) * normalize(worldNormal) - I;

    fColor = texture(skybox, -R);
} 


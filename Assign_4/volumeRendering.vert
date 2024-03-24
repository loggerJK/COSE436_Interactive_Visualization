#version 140
#extension GL_ARB_compatibility: enable
out vec3 pixelPosition;

void main(){
    pixelPosition=vec3(gl_Vertex);
    gl_Position =gl_Vertex;
}
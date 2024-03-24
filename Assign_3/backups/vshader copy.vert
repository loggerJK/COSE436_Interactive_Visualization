#version 330 
#extension GL_ARB_compatibility: enable


in vec4 vPosition, vNormal;
in vec2 vTexCoord;

out vec4 color;
out vec2 fTexCoord;
out vec3 fTexCoord_cube, cameraNormal, cameraPos;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 World;

void main() 
{
   gl_Position = Projection * ModelView  * World * vPosition;
   cameraNormal = (transpose(inverse(ModelView)) * transpose(inverse(World)) * vNormal).xyz;
   cameraPos = (ModelView * World * vPosition).xyz;
   fTexCoord_cube = (vPosition).xyz;
   fTexCoord = vTexCoord;
   color = gl_Color;   
}



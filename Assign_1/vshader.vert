#version 140
#extension GL_ARB_compatibility: enable

in vec4 vPosition;
out vec4 color;

uniform mat4 ModelView;
uniform mat4 Projection;

void main() 
{
   gl_Position = Projection * ModelView * vPosition;
   color = gl_Color;   
}



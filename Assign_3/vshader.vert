#version 330 

in vec4 vPosition, vNormal;
in vec2 vTexCoord;

out vec2 fTexCoord;
out vec3 fTexCoord_cube, worldNormal, worldPos, cameraPos;


uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 World;

void main() 
{
   gl_Position = Projection * ModelView  * World * vPosition;
   worldNormal = (transpose(inverse(World)) * vNormal).xyz;
   worldPos = (World * vPosition).xyz;
   // cameraPos는 World 좌표계 기준 eye coordinate
   cameraPos.x = inverse(ModelView)[3][0];
   cameraPos.y = inverse(ModelView)[3][1];
   cameraPos.z = inverse(ModelView)[3][2];
   fTexCoord_cube = (-vPosition).xyz;
   fTexCoord = vTexCoord;
}



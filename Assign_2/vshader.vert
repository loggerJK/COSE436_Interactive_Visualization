#version 330
#extension GL_ARB_compatibility: enable

in vec4 vPosition, vColor;
in vec3 vNormal;
out vec4 color;
out vec3 pos, light_pos_0, light_pos_1, normal;
out float dist_0, dist_1;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition_0, LightPosition_1;

void main() 
{
   // Calculate distance in world coord
   dist_0 = length(LightPosition_0 - vPosition);
   dist_1 = length(LightPosition_1 - vPosition);
   pos = (ModelView * vPosition).xyz;
   // Light
   light_pos_0 = (ModelView * LightPosition_0).xyz;
   light_pos_1 = (ModelView * LightPosition_1).xyz;
   // normal = (ModelView * vec4(vNormal, 1.0)).xyz;
   normal = (transpose(inverse(ModelView)) * vec4(vNormal, 1.0)).xyz;
   // normal = normalize(vNormal);
   

   gl_Position = Projection * ModelView * vPosition;
   color = gl_Color;   
   // color = vColor;
   // color = vec4(1.0, 1.0, 1.0, 1.0);
   // color = ambient + diffuse + specular;
   // color = diffuse;


   // color.a = 1.0;

}



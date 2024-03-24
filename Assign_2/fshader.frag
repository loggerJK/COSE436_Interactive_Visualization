#version 330
#extension GL_ARB_compatibility: enable

in  vec4 color;
in  vec3 pos, light_pos_0, light_pos_1, normal;
in float dist_0, dist_1;
out vec4 fColor;

uniform vec4 AmbientProduct_0, DiffuseProduct_0, SpecularProduct_0;
uniform vec4 DiffuseProduct_1, SpecularProduct_1;
uniform float k_d_0, k_d_1, k_a, k_s_0, k_s_1, alpha_0, alpha_1;

uniform vec3 attenuation;

void main() 
{ 
   // ambient
   vec4 ambient = k_a * AmbientProduct_0;

   vec3 l_0 = normalize( (light_pos_0).xyz - pos );
   // Surface Normal
   vec3 n = normalize(normal);
   // Reflection Vector
   vec3 r_0 = 2 * (dot(l_0, n)) * n - l_0;
   vec3 v = normalize(-pos);

   // diffuse
   vec4 diffuse_0 = k_d_0 * max( dot(l_0, n), 0.0 ) * DiffuseProduct_0;

   // specular
   vec4 specular_0 = k_s_0 * pow( max(dot(r_0, v), 0.0), alpha_0) * SpecularProduct_0;
   if ( dot(l_0, n) < 0.0 ) {
      specular_0 = vec4(0.0, 0.0, 0.0, 1.0);
   }

   float att_0 = 1.0 / (attenuation.x + attenuation.y * dist_0 + attenuation.z * dist_0 * dist_0);

   //=============== LIGHT 1 ====================
   vec3 l_1 = normalize( (light_pos_1).xyz - pos );
   // Reflection Vector
   vec3 r_1 = 2 * (dot(l_1, n)) * n - l_1;

   // diffuse
   vec4 diffuse_1 = k_d_1 * max( dot(l_1, n), 0.0 ) * DiffuseProduct_1;

   // specular
   vec4 specular_1 = k_s_1 * pow( max(dot(r_1, v), 0.0), alpha_1) * SpecularProduct_1;
   if ( dot(l_1, n) < 0.0 ) {
      specular_1 = vec4(0.0, 0.0, 0.0, 1.0);
   }

   float att_1 = 1.0 / (attenuation.x + attenuation.y * dist_1 + attenuation.z * dist_1 * dist_1);

    fColor = ambient + att_0 * (diffuse_0 + specular_0) + att_1 * (diffuse_1 + specular_1);
   //  fColor = ambient + (diffuse_0 + specular_0) + (diffuse_1 + specular_1);
   // fColor = diffuse_0 + diffuse_1;
   // color = diffuse;
    fColor.a = 1.0;


} 

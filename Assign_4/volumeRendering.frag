#version 140
#extension GL_ARB_compatibility: enable

in vec3 pixelPosition;
uniform vec3 eyePosition;
uniform vec3 objectMin;
uniform vec3 objectMax;
uniform vec3 up;
uniform sampler3D tex;
uniform vec4 transferFunction[256];

// #define MIP

mat4 translate(float x, float y, float z)
{
	vec3 delta = vec3(x, y, z);
    mat4 m;
    m[0][0] = 1;
    m[1][1] = 1;
    m[2][2] = 1;
    m[3] = vec4(delta, 1.0);
    return m;
}

vec4 texture1DArray(vec4 array[256],float value){
	value = value * 255;
	int l_value = int(value);
	int u_value = l_value+1;
	if(u_value>255){
		return array[255];
	}
	return array[l_value] * (1.0 - (value - l_value)) + array[u_value] * (value-l_value);
}



void main(){
    vec3 right=normalize(cross(-eyePosition,up));
	float pixeScale = 0.7;
    vec3 virtualScreenCoord=eyePosition*0.5+
                                right*pixelPosition.x * pixeScale+
                                up*pixelPosition.y * pixeScale;


	// Perspective Projection
    vec3 rayDirection=normalize(virtualScreenCoord-eyePosition); 
	
//	// Orthogonal Projection
	// vec3 rayDirection=normalize(-eyePosition); 


	//
	// 1. find ray-cube intersect point 
	//
	int cnt=0;
	vec3 endPoint[2]; // 0 : entry point, 1 : exit point
	float scale = 2.0;
	vec3 scaledObjectMin = objectMin * scale;
	vec3 scaledObjectMax = objectMax * scale;


	// ============Intersection test================
	bool intersect = true;
	float tmin = (scaledObjectMin.x - eyePosition.x) / rayDirection.x; 
    float tmax = (scaledObjectMax.x - eyePosition.x) / rayDirection.x; 
	{
 
    // if (tmin > tmax) swap(tmin, tmax); 
	if (tmin > tmax) {
		float temp = tmin;
		tmin = tmax;
		tmax = temp;
	}
 
    float tymin = (scaledObjectMin.y - eyePosition.y) / rayDirection.y; 
    float tymax = (scaledObjectMax.y - eyePosition.y) / rayDirection.y; 
 
    // if (tymin > tymax) swap(tymin, tymax); 
	if (tymin > tymax) {
		float temp = tymin;
		tymin = tymax;
		tymax = temp;
	}
 
    if ((tmin > tymax) || (tymin > tmax)) 
        intersect = false; 
 
    if (tymin > tmin) 
        tmin = tymin; 
 
    if (tymax < tmax) 
        tmax = tymax; 
 
    float tzmin = (scaledObjectMin.z - eyePosition.z) / rayDirection.z; 
    float tzmax = (scaledObjectMax.z - eyePosition.z) / rayDirection.z; 
 
    // if (tzmin > tzmax) swap(tzmin, tzmax); 
	if (tzmin > tzmax) {
		float temp = tzmin;
		tzmin = tzmax;
		tzmax = temp;
	}
 
    if ((tmin > tzmax) || (tzmin > tmax)) 
        intersect = false; 
 
    if (tzmin > tmin) 
        tmin = tzmin; 
 
    if (tzmax < tmax) 
        tmax = tzmax; 
	}
	// =============================================

#ifdef MIP
	//
	// 2. Maximum intensity projection
	//
	vec4 composedColor=vec4(0,0,0,0);

	if (intersect) {
		// composedColor=vec4(1,1,0,0);
		int numSamples = 16;
		float dt = (tmax - tmin) / float(numSamples);
		float maxIntensity = 0.0;
		for (int i = 0; i < numSamples; i++) {
			vec3 samplePoint = eyePosition + rayDirection * (tmin + dt * float(i));
			samplePoint.x = samplePoint.x + 0.5;
			samplePoint.y = samplePoint.y + 0.5;
			samplePoint.z = samplePoint.z + 0.5;
			vec4 sampleColor = texture(tex,  samplePoint);
			float intensity = (sampleColor.r + sampleColor.g + sampleColor.b) / 3.0;
			if (intensity > maxIntensity) {
				maxIntensity = intensity;
			}
		}
		// composedColor = texture1DArray(transferFunction, maxIntensity);
		composedColor.r = maxIntensity;
		composedColor.g = maxIntensity;
		composedColor.b = maxIntensity;
	}
	else{
		composedColor=vec4(1,1,1,0);
	}
	
    gl_FragColor=composedColor;
#else
	//
	// 3. alpha composition
	//
    float composedAlpha=0;
    vec4 composedColor=vec4(0,0,0,0); // Black

	// Front - Back composition
	// Terminate early when alpha is 1
	if (intersect) {
		int numSamples = 256;
		float dt = (tmax - tmin) / float(numSamples);
		for (int i = 0; i < numSamples; i++) {
			vec3 samplePoint = eyePosition + rayDirection * (tmin + dt * float(i));
			samplePoint.x = samplePoint.x + 0.5;
			samplePoint.y = samplePoint.y + 0.5;
			samplePoint.z = samplePoint.z + 0.5;
			vec4 sampleColor = texture(tex,  samplePoint);
			float intensity = (sampleColor.r + sampleColor.g + sampleColor.b) / 3.0;
			// Use texture1DArray to get color and alpha
			vec4 tfColor = texture1DArray(transferFunction, intensity);
			float alpha = tfColor.a;
			composedColor = composedColor + (1.0 - composedAlpha) * tfColor;
			composedAlpha = composedAlpha + (1.0 - composedAlpha) * alpha;
			if (composedAlpha >= 0.9) {
				break;
			}
		}
	}
	else{
		composedColor=vec4(1,1,1,0); // White
	}
    gl_FragColor=composedColor;
#endif

}
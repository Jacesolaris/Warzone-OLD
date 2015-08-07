attribute vec4 attr_Color;
attribute vec2 attr_TexCoord0;
attribute vec3 attr_Position;
attribute vec3 attr_Normal;

uniform sampler2D u_NormalMap;

uniform mat4   u_ModelViewProjectionMatrix;

varying vec4   var_Color;

uniform vec2	u_Dimensions;
uniform vec3	u_ViewOrigin;
uniform float	u_Time;
uniform vec4	u_Local0; // (1=water, 2=lava), 0, 0, 0
uniform vec4	u_Local1; // parallaxScale, haveSpecular, specularScale, materialType

varying vec2	var_TexCoords;
varying float	time;
varying vec4	var_Local0; // (1=water, 2=lava), 0, 0, 0
varying vec4	var_Local1; // parallaxScale, haveSpecular, specularScale, materialType
varying vec2	var_Dimensions;

float waveTime = u_Time;
//float waveWidth = 1.0;
//float waveHeight = 1.0;

//float waveTime = 0.5;
float waveWidth = 0.6;
float waveHeight = 1.0;
//waveFreq = 0.1;

float YRot = (90.0/360.0) * 10.0;

void rotateY (inout vec4 vert, float rads) {
    vec4 old = vert;
    mat2 rotY = mat2(cos(rads), -sin(rads), sin(rads), cos(rads));
    vert.xz = old.xz * rotY;
}

void main(void)
{
	//float cosAngle = cos(YRot);
	//float sinAngle = sin(YRot);
	//float negSinAngle = -sinAngle;
	//mat4 rotateZ; rotateZ[0].x = cosAngle; rotateZ[0].y = negSinAngle; rotateZ[0].z = 0.0; rotateZ[0].w = 0.0; rotateZ[1].x = sinAngle; rotateZ[1].y = cosAngle; rotateZ[1].z = 0.0; rotateZ[1].w = 0.0; rotateZ[2].x = 0.0; rotateZ[2].y = 0.0; rotateZ[2].z = 1.0; rotateZ[2].w = 0.0; rotateZ[3].x = 0.0; rotateZ[3].y = 0.0; rotateZ[3].z = 0.0; rotateZ[3].w = 1.0; 

	vec4 v = vec4(attr_Position.xyz, 1.0);
	//v.z = sin(waveWidth * v.x + waveTime) * cos(waveWidth * v.y + waveTime) * waveHeight;
	//v.z *= (1.0 - attr_TexCoord0.y) * 12.0;
	//v.z /= attr_Normal.z;

	//v *= rotateZ;
 	//gl_Position = u_ModelViewProjectionMatrix * v;
	
	var_TexCoords = attr_TexCoord0.xy;


	//vec3 vnormal = normalize(u_ModelViewProjectionMatrix * vec4(attr_Normal.xyz, 1.0)).xyz;
    
	//rotateY (v/*gl_Vertex*/, YRot);
    //gl_Position = ftransform();
	gl_Position = u_ModelViewProjectionMatrix * v;

	/*
	vec4 newVertexPos;
	vec4 dv;
	float df;
	
	var_TexCoords.xy = attr_TexCoord0.xy;
	
	dv = texture2D( u_NormalMap, var_TexCoords.xy );
	
	df = 0.30*dv.x + 0.59*dv.y + 0.11*dv.z;
	
	newVertexPos = vec4(gl_Normal * df * 100.0, 0.0) + gl_Vertex;
	
	gl_Position = u_ModelViewProjectionMatrix * newVertexPos;
	*/
}

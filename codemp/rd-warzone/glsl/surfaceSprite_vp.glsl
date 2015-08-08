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
uniform vec4	u_Local5; // rotationx, rotationy, rotationz, 0

varying vec2	var_TexCoords;
varying vec4	var_Local0; // (1=water, 2=lava), 0, 0, 0
varying vec4	var_Local1; // parallaxScale, haveSpecular, specularScale, materialType
varying vec2	var_Dimensions;

#if 0
varying float	time;

float waveTime = u_Time;
//float waveWidth = 1.0;
//float waveHeight = 1.0;

//float waveTime = 0.5;
float waveWidth = 0.6;
float waveHeight = 1.0;
//waveFreq = 0.1;

void main(void)
{
/*
	//float cosAngle = cos(YRot);
	//float sinAngle = sin(YRot);
	//float negSinAngle = -sinAngle;
	//mat4 rotateZ; rotateZ[0].x = cosAngle; rotateZ[0].y = negSinAngle; rotateZ[0].z = 0.0; rotateZ[0].w = 0.0; rotateZ[1].x = sinAngle; rotateZ[1].y = cosAngle; rotateZ[1].z = 0.0; rotateZ[1].w = 0.0; rotateZ[2].x = 0.0; rotateZ[2].y = 0.0; rotateZ[2].z = 1.0; rotateZ[2].w = 0.0; rotateZ[3].x = 0.0; rotateZ[3].y = 0.0; rotateZ[3].z = 0.0; rotateZ[3].w = 1.0; 

	vec3 rotation = u_Local5.xyz;

	vec4 v = vec4(attr_Position.xyz, 1.0);
	//v.z = sin(waveWidth * v.x + waveTime) * cos(waveWidth * v.y + waveTime) * waveHeight;
	//v.z *= (1.0 - attr_TexCoord0.y) * 12.0;
	//v.z /= attr_Normal.z;

	//v *= rotateZ;
 	//gl_Position = u_ModelViewProjectionMatrix * v;
	
	//v.xyz *= rotate_vertex_position(v.xyz, vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, YRot));
	v *= rotationX(rotation.x) * rotationY(rotation.y) * rotationZ(rotation.z);
	var_TexCoords = attr_TexCoord0.xy;


	//vec3 vnormal = normalize(u_ModelViewProjectionMatrix * vec4(attr_Normal.xyz, 1.0)).xyz;
    
	gl_Position = u_ModelViewProjectionMatrix * v;
	*/

	vec4 v = vec4(attr_Position.xyz, 1.0);
	//v *= rotateZ(u_Local5.z);
	v.z += (v.z*u_Local5.z);
	gl_Position = u_ModelViewProjectionMatrix * v;
	var_TexCoords = attr_TexCoord0.xy * (1.0 / u_Local5.z);

	/*
	float timer = u_Local5.z;
	mat3 rotation = mat3(
        vec3( cos(timer),  sin(timer),  0.0),
        vec3(-sin(timer),  cos(timer),  0.0),
        vec3(        0.0,         0.0,  1.0)
    );
    gl_Position = u_ModelViewProjectionMatrix * vec4(rotation * attr_Position.xyz, 1.0);
	var_TexCoords = attr_TexCoord0.xy;
	*/

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


#else



mat4 modelViewProjection = u_ModelViewProjectionMatrix;

varying vec4 vColor;
varying vec2 vTexCoord;

vec4 position = vec4(attr_Position.xyz, 1.0);
vec4 color = attr_Color;
vec2 texCoord = attr_TexCoord0;
float time = u_Time;

// Noise
float hash( float n ) {
    return fract(sin(n)*43758.5453);
}
float noise( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0;
    float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x), mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
    return res;
} 
float fbm( vec2 p ) {
        float f = 0.0;
        f += 0.50000*noise( p ); p = p*2.02;
 //       f += 0.25000*noise( p ); p = p*2.03;
  //      f += 0.12500*noise( p ); p = p*2.01;
  //      f += 0.06250*noise( p ); p = p*2.04;
        f += 0.03125*noise( p );
        return f/0.984375;
}

void main()
{
	vec2 offset = vec2(1.0 / u_Dimensions.x, 1.0 / u_Dimensions.y);

    vColor = color;
    vTexCoord = vec2(texCoord.x, texCoord.y);
    vec4 v = vec4(position.xyz, 1.0);
	v.z += (360.0 * texCoord.y) + (offset.y * texCoord.y * 360.0);
	vec4 p =  modelViewProjection * v;
    if(vTexCoord.y > .9) {
        float n = fbm(p.xy*time*.2)*4. - 2.;
        p = p + vec4(n,0.,0.,0.);
    }
    lowp vec4 nor = normalize(p);
    lowp float vShade = (dot(nor.xyz,vec3(-1.,0.,0.))+0.2)/1.2;
    if (vShade >1.0) {vShade=1.0;}
    if (vShade <0.2) {vShade=0.2;}
    vColor.rgb = vColor.rgb * vShade;
    gl_Position = p;
}

#endif

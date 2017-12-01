attribute vec3	attr_OceanPosition;
attribute vec2	attr_OceanTexCoord;

uniform mat4	u_ModelViewProjectionMatrix;

uniform vec4	u_Local10;

uniform float	u_Time;

uniform vec3	u_ViewOrigin;

out vec3	vertPosition;
out vec3	Binormal;/* Tangent basis */
out vec3	Tangent;
out vec3	Normal;
out vec3	View;	/* View vector */

/* Multiple bump coordinates for animated bump mapping */
out vec2	bumpCoord0;
out vec2	bumpCoord1;
out vec2	bumpCoord2;


void main()
{
	vec4 P = vec4(attr_OceanPosition, 1.0);
	
	/* TODO: Add waves to P, set 2 waves */
	float A[2] = float[](1.0,0.5);		//amplitude
	float Dx[2] = float[](-1.0,-0.7);	//(DX,DZ)direction of travel
	float Dz[2] = float[](0.0,0.7);
	float f[2] = float[](0.2,0.4);		//frequency
	float p[2] = float[](0.5,1.3);		//phase
	float k[2] = float[](2.0,2.0);		//sharpness

    //waves:y=G(x,z,t)

    float wave1 = A[0] * pow((sin((Dx[0] * P.x + Dz[0] * P.y)*f[0] + u_Time *p[0])*0.5 + 0.5),k[0]);
    float dGx1  = 0.5 * k[0] * f[0] * A[0] * pow((sin((Dx[0] * P.x + Dz[0] * P.y)*f[0] + u_Time *p[0])*0.5+0.5),k[0]-1) * cos((Dx[0] * P.x + Dz[0] * P.y)*f[0] + u_Time *p[0])*Dx[0];
    float dGz1  = 0.5 * k[0] * f[0] * A[0] * pow((sin((Dx[0] * P.x + Dz[0] * P.y)*f[0] + u_Time *p[0])*0.5+0.5),k[0]-1) * cos((Dx[0] * P.x + Dz[0] * P.y)*f[0] + u_Time *p[0])*Dz[0];

	float wave2 = A[1] * pow((sin((Dx[1] * P.x + Dz[1] * P.y)*f[1] + u_Time *p[1])*0.5 + 0.5),k[1]);
    float dGx2  = 0.5 * k[1] * f[1] * A[1] * pow((sin((Dx[1] * P.x + Dz[1] * P.y)*f[1] + u_Time *p[1])*0.5+0.5),k[1]-1) * cos((Dx[1] * P.x + Dz[1] * P.y)*f[1] + u_Time *p[1])*Dx[1];
    float dGz2  = 0.5 * k[1] * f[1] * A[1] * pow((sin((Dx[1] * P.x + Dz[1] * P.y)*f[1] + u_Time *p[1])*0.5+0.5),k[1]-1) * cos((Dx[1] * P.x + Dz[1] * P.y)*f[1] + u_Time *p[1])*Dz[1];


    //sum of waves
    //P.y = wave1+wave2;
	P.z = wave1+wave2;
    float dHx = dGx1 + dGx2;
    float dHz = dGz1 + dGz2;

	/* TODO: Compute B, T, N */
    Binormal = vec3(1,dHx,0);
    Tangent = vec3(0,dHz,1);
    Normal = vec3(-dHx,1,-dHz);
    View = attr_OceanPosition - u_ViewOrigin;  //don't normalize it

	/* TODO: Compute bumpmap coordinates */
	vec2 texScale = vec2(8,4);
	float bumpTime = mod(u_Time,100.0);
	vec2 bumpSpeed = vec2(-0.05,0);

	bumpCoord0.xy = attr_OceanTexCoord.xy * texScale + bumpTime * bumpSpeed;
	bumpCoord1.xy = attr_OceanTexCoord.xy * texScale *2+bumpTime * bumpSpeed*4;
	bumpCoord2.xy = attr_OceanTexCoord.xy * texScale *4+bumpTime * bumpSpeed*8;

	gl_Position = u_ModelViewProjectionMatrix * P;

	vertPosition = P.xyz;
}

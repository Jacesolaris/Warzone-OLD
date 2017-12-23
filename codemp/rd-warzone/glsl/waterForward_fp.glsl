/* Cube and Bumpmaps */
//uniform samplerCube u_CubeMap;
uniform sampler2D   u_NormalMap;

uniform vec4		u_Local9;

in vec3	vertPosition;
in vec3	Binormal;/* Tangent basis */
in vec3	Tangent;
in vec3	Normal;
in vec3	View;/* View vector */

/* Multiple bump coordinates for animated bump mapping */
in vec2	bumpCoord0;
in vec2	bumpCoord1;
in vec2	bumpCoord2;

out vec4 out_Glow;
out vec4 out_Normal;
out vec4 out_Position;
out vec4 out_NormalDetail;

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

void main()
{
	/* TODO: add color, reflection, animated bump mapping, refraction and fresnel */
	vec3 n = normalize(Normal);
	vec3 V = normalize(View);
	//fresnel terms air to water
	float R0 = 0.02037;

	vec4 deepColor = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 shallowColor = vec4(0.0, 0.5, 0.5, 1.0);
	float facing = 1- max(dot(-V,n),0.0);
	vec4 waterColor = mix(deepColor,shallowColor,facing);
 
  	//animated bump mapping
  	vec4 n0 = texture(u_NormalMap, bumpCoord0) * 2.0 - 1.0;
	vec4 n1 = texture(u_NormalMap, bumpCoord1) * 2.0 - 1.0;
	vec4 n2 = texture(u_NormalMap, bumpCoord2) * 2.0 - 1.0;
	vec3 nBump = normalize(n0.xyz+n1.xyz+n2.xyz);
	//vec3 nBump = normalize(Normal);//n0.xyz+n1.xyz+n2.xyz;

	//
	//tangent space-> world space(nbump->newN)
	mat3 BTN = mat3(Binormal,Tangent,Normal); 
	vec3 newN = BTN * nBump;
	newN= normalize(newN);
	//vec3 newN = n;

	//Reflection Mapping
    vec3 reflection = reflect(-V,newN);
    vec4 reflecColor = vec4(0.0);//texture(u_CubeMap, reflection);
  	
  	//fresnel:how much light reflects at glancing angle and how much refracts
  	float fastFresnel = R0 + (1-R0) * pow( (1-dot(-V,newN)) , 5);
	
	//refraction
	/*opengl func:refract 
	genType refract(genType I, 	genType N, 	float eta);*/
	
	float eta = 1.0/1.33;
	vec3 refract = refract(V,newN,eta);//air to water
	vec4 refraction = vec4(0.0);//texture(u_CubeMap, refract);

	if (u_Local9.r > 5.0)
	{
		out_Color = vec4(waterColor.xyz, 1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(EncodeNormal(newN), 0.0, 1.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(vertPosition.xyz, MATERIAL_WATER+1.0);
		return;
	}
	else if (u_Local9.r > 4.0)
	{
		out_Color = vec4(shallowColor.xyz, 1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(EncodeNormal(newN), 0.0, 1.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(vertPosition.xyz, MATERIAL_WATER+1.0);
		return;
	}
	else if (u_Local9.r > 3.0)
	{
		out_Color = vec4(deepColor.xyz, 1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(EncodeNormal(newN), 0.0, 1.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(vertPosition.xyz, MATERIAL_WATER+1.0);
		return;
	}
	else if (u_Local9.r > 2.0)
	{
		out_Color = vec4(nBump.xyz * 0.5 + 0.5, 1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(EncodeNormal(newN), 0.0, 1.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(vertPosition.xyz, MATERIAL_WATER+1.0);
		return;
	}
	else if (u_Local9.r > 1.0)
	{
		out_Color = vec4(newN.xyz * 0.5 + 0.5, 1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(EncodeNormal(newN), 0.0, 1.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(vertPosition.xyz, MATERIAL_WATER+1.0);
		return;
	}
	else if (u_Local9.r > 0.0)
	{
		out_Color = vec4(n.xyz * 0.5 + 0.5, 1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(EncodeNormal(newN), 0.0, 1.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(vertPosition.xyz, MATERIAL_WATER+1.0);
		return;
	}
	
	out_Color = waterColor + reflecColor * fastFresnel + refraction * (1-fastFresnel);
	out_Glow = vec4(0.0);
	out_Normal = vec4(EncodeNormal(newN), 0.0, 1.0);
	out_NormalDetail = vec4(0.0);
	out_Position = vec4(vertPosition.xyz, MATERIAL_WATER+1.0);
}

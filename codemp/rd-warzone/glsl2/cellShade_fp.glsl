uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin

varying vec2		var_TexCoords;


vec2 pixelSize = vec2(1.0) / u_Dimensions;

vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N*4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);

	return vec3(encoded * g, 1.0 - f * 0.5);
}

vec3 normals(vec2 tex)//get normal vector from normalmap
{
	vec3 norm = textureLod(u_NormalMap, tex, 0.0).xyz;// * 2.0 - 1.0;
	//norm.z = sqrt(1.0-dot(norm.xy, norm.xy)); // reconstruct Z from X and Y
	norm.xyz = DecodeNormal(norm.xy);
	return norm.xyz;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//cel shader
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void main (void)
{
	vec2 tex = var_TexCoords.xy;
	
	vec4 res = textureLod(u_DiffuseMap, tex, 0.0);
	
	vec3 Gx[3];
	Gx[0] = vec3(-1.0, 0.0, 1.0);
	Gx[1] = vec3(-2.0, 0.0, 2.0);
	Gx[2] = vec3(-1.0, 0.0, 1.0);

	vec3 Gy[3]; 
	Gy[0] = vec3( 1.0, 2.0, 1.0);
	Gy[1] = vec3( 0.0, 0.0, 0.0);
	Gy[2] = vec3( -1.0, -2.0, -1.0);

	vec3 dotx = vec3(0.0);
	vec3 doty = vec3(0.0);

	for(int i = 0; i < 3; i ++)
	{		
		dotx += Gx[i].x * normals(vec2((tex.x - pixelSize.x), (tex.y + ( (-1.0 + float(i)) * pixelSize.y))));
		dotx += Gx[i].y * normals(vec2( tex.x, (tex.y + ( (-1.0 + float(i)) * pixelSize.y))));
		dotx += Gx[i].z * normals(vec2((tex.x + pixelSize.x), (tex.y + ( (-1.0 + i) * pixelSize.y))));
		
		doty += Gy[i].x * normals(vec2((tex.x - pixelSize.x), (tex.y + ( (-1.0 + i) * pixelSize.y))));
		doty += Gy[i].y * normals(vec2( tex.x, (tex.y + ( (-1.0 + float(i)) * pixelSize.y))));
		doty += Gy[i].z * normals(vec2((tex.x + pixelSize.x), (tex.y + ( (-1.0 + float(i)) * pixelSize.y))));
	}
	
	res.xyz -= step(1.0, sqrt( dot(dotx, dotx) + dot(doty, doty) ) * 1.7/*u_ViewInfo.a*/ );
	
	gl_FragColor = res;
}

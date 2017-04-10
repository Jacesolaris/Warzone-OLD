uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_DepthMap;
uniform sampler2D	u_NormalMap;

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin

varying vec2		var_TexCoords;


vec2 pixelSize = vec2(1.0) / u_Dimensions;

#if 0
float linearlizeDepth(float nonlinearDepth)
{
/*
	vec2 dofProj=vec2(u_ViewInfo.x, u_ViewInfo.y);
	vec2 dofDist=vec2(0.0, u_ViewInfo.x);

	vec4 depth=vec4(nonlinearDepth);

	depth.y=-dofProj.x + dofProj.y;
	depth.y=1.0/depth.y;
	depth.z=depth.y * dofProj.y;
	depth.z=depth.z * -dofProj.x;
	depth.x=dofProj.y * -depth.y + depth.x;
	depth.x=1.0/depth.x;

	depth.y=depth.z * depth.x;

	depth.x=depth.z * depth.x - dofDist.y;
	depth.x+=dofDist.x * -0.5;

	depth.x=max(depth.x, 0.0);

	return depth.x;
*/
	return 1.0 / mix(u_ViewInfo.x, 1.0, nonlinearDepth);
}


vec3 normals(vec2 tex)//get normal vector from depthmap
{
	float deltax = linearlizeDepth( textureLod(u_DepthMap, vec2((tex.x + pixelSize.x), tex.y), 0.0).x) - linearlizeDepth( textureLod(u_DepthMap, vec2((tex.x - pixelSize.x), tex.y), 0.0).x);
	float deltay = linearlizeDepth( textureLod(u_DepthMap, vec2( tex.x, (tex.y + pixelSize.y)), 0.0).x) - linearlizeDepth( textureLod(u_DepthMap, vec2( tex.x, (tex.y - pixelSize.y)), 0.0).x);	
	return normalize(vec3( (deltax / 2.0 / pixelSize.x), (deltay / 2.0 / pixelSize.y), 1.0));
}
#else
vec3 normals(vec2 tex)//get normal vector from normalmap
{
	return textureLod(u_NormalMap, tex, 0.0).xyz * 2.0 - 1.0;
}
#endif

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

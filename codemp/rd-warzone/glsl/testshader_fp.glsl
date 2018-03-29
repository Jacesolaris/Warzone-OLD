uniform sampler2D	u_DiffuseMap; // screen/texture map
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_SpecularMap; // color random image

uniform vec4		u_ViewInfo;
uniform vec2		u_Dimensions;
uniform vec4		u_Local0; // r_testvalue's
uniform vec4		u_Local1; // r_testshadervalue's


uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

// Position of the camera
uniform vec3		u_ViewOrigin;
uniform float		u_Time;

varying vec2		var_TexCoords;

//
// Drawing effect...
//

#define Res0 u_Dimensions//textureSize(iChannel0,0)
#define Res1 vec2(2048.0)//textureSize(iChannel1,0)

#define Time u_Time

vec4 getRand(vec2 pos)
{
	return textureLod(u_SpecularMap, pos / Res1 / Res0.y*1080., 0.0);
}

vec4 getCol(vec2 pos)
{
	// take aspect ratio into account
	vec2 uv = ((pos - Res0.xy*.5) / Res0.y*Res0.y) / Res0.xy + .5;
	vec4 c1 = texture(u_DiffuseMap, uv);
	vec4 e = smoothstep(vec4(-0.05), vec4(-0.0), vec4(uv, vec2(1) - uv));
	c1 = mix(vec4(1, 1, 1, 0), c1, e.x*e.y*e.z*e.w);
	float d = clamp(dot(c1.xyz, vec3(-.5, 1., -.5)), 0.0, 1.0);
	vec4 c2 = vec4(.7);
	return min(mix(c1, c2, 1.8*d), .7);
}

vec4 getColHT(vec2 pos)
{
	return smoothstep(.95, 1.05, getCol(pos)*.8 + .2 + getRand(pos*.7));
}

float getVal(vec2 pos)
{
	vec4 c = getCol(pos);
	return pow(dot(c.xyz, vec3(.333)), 1.)*1.;
}

vec2 getGrad(vec2 pos, float eps)
{
	vec2 d = vec2(eps, 0);
	return vec2(
		getVal(pos + d.xy) - getVal(pos - d.xy),
		getVal(pos + d.yx) - getVal(pos - d.yx)
	) / eps / 2.;
}

#define AngleNum 3

#define SampNum 6//int(u_Local0.r)//16
#define PI2 6.28318530717959

void Drawing(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 pos = fragCoord + 4.0*sin(Time*1.*vec2(1, 1.7))*Res0.y / 400.;
	vec3 col = vec3(0);
	vec3 col2 = vec3(0);
	float sum = 0.;
	for (int i = 0; i<AngleNum; i++)
	{
		float ang = PI2 / float(AngleNum)*(float(i) + .8);
		vec2 v = vec2(cos(ang), sin(ang));
		for (int j = 0; j<SampNum; j++)
		{
			vec2 dpos = v.yx*vec2(1, -1)*float(j)*Res0.y / 400.;
			vec2 dpos2 = v.xy*float(j*j) / float(SampNum)*.5*Res0.y / 400.;
			vec2 g;
			float fact;
			float fact2;

			for (float s = -1.; s <= 1.; s += 2.)
			{
				vec2 pos2 = pos + s*dpos + dpos2;
				vec2 pos3 = pos + (s*dpos + dpos2).yx*vec2(1, -1)*2.;
				g = getGrad(pos2, .4);
				fact = dot(g, v) - .5*abs(dot(g, v.yx*vec2(1, -1)))/**(1.-getVal(pos2))*/;
				fact2 = dot(normalize(g + vec2(.0001)), v.yx*vec2(1, -1));

				fact = clamp(fact, 0., .05);
				fact2 = abs(fact2);

				fact *= 1. - float(j) / float(SampNum);
				col += fact;
				col2 += fact2*getColHT(pos3).xyz;
				sum += fact2;
			}
		}
	}
	col /= float(SampNum*AngleNum)*.75 / sqrt(Res0.y);
	col2 /= sum;
	col.x *= (.6 + .8*getRand(pos*.7).x);
	col.x = 1. - col.x;
	col.x *= col.x*col.x;

	float r = length(pos - Res0.xy*.5) / Res0.x;
	float vign = 1. - r*r*r;

	fragColor = vec4(vec3(col.x*col2*vign), 1.0);

	//vec2 s=sin(pos.xy*.1/sqrt(Res0.y/400.));
	//vec3 karo=vec3(1);
	//karo-=.5*vec3(.25,.1,.1)*dot(exp(-s*s*80.),vec2(1));
	//fragColor = vec4(vec3(col.x*col2*karo*vign),1);
}


void main()
{
	Drawing(gl_FragColor, var_TexCoords * Res0);
}


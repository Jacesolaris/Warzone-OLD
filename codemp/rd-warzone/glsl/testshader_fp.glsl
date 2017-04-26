uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;

uniform vec4		u_ViewInfo;
uniform vec2		u_Dimensions;
uniform vec4		u_Local0; // r_testvalue's

varying vec2		var_TexCoords;

vec2 fvTexelSize = vec2(1.0) / u_Dimensions.xy;

#if 0
// 'threshold ' is constant , 'value ' is smoothly varying
float aastep ( float threshold , float value )
{
	float afwidth = 0.7 * length ( vec2 ( dFdx ( value ) , dFdy ( value ))) ;
	// GLSL 's fwidth ( value ) is abs ( dFdx ( value )) + abs ( dFdy ( value ))
	return smoothstep ( threshold - afwidth , threshold + afwidth , value ) ;
}

void main(void)
{
	gl_FragColor = texture(u_DiffuseMap, var_TexCoords);
}


#else // Test shader...


// Experimental screen space geometry smoothing effect...
#define		C_HBAO_ZNEAR u_ViewInfo.x//0.0509804
#define		C_HBAO_ZFAR u_ViewInfo.y//2048.0

float LinDepth ( vec2 coord )
{
	float depth = texture(u_ScreenDepthMap, coord).x;
	float d = depth;
	d /= C_HBAO_ZFAR - depth * C_HBAO_ZFAR + depth;
	return clamp(d, 0.0, 1.0);
}

vec4 PS_ProcessGeometryEnhance(vec2 inCoord)
{
	vec2 Tex = inCoord.xy;
	vec4 color = texture(u_DiffuseMap, Tex.xy);
	vec2 offset = vec2(fvTexelSize.x, fvTexelSize.y);

	color *= u_Local0.a;//1.05; // Brighten the whole screen a little to compensate for some darkening...
  
	float originalDepth = LinDepth(Tex);
	
	if (originalDepth > 0.99999)
	{// Never do skies...
		//return vec4(1.0, 0.0, 0.0, 1.0);
		return color;
	}
	
	float surroundDepth = 0.0;
	float numSamples = 0.0;
	
	for (float x = -1.0; x <= 1.0; x += 0.2)
	{
		for (float y = -1.0; y <= 1.0; y += 0.2)
		{
			vec2 coord = Tex + ((vec2(x, y) * length(u_Local0.g/*200.0*/ * vec2(x, y))) * offset * (1.0 - originalDepth));
      
			if (coord.x >= 0.0 && coord.x <= 1.0 && coord.y >= 0.0 && coord.y <= 1.0)
			{// Never sample off screen...
				float thisDepth = LinDepth(coord);
        
				if (thisDepth <= 0.99999 && thisDepth > originalDepth)
				{// We only use close depth from original pixels...
					surroundDepth += thisDepth;
					numSamples += 1.0;
				}
			}
		}
	}

	if (numSamples > 0.0)
	{
		surroundDepth /= numSamples;
	
		float origDiff = surroundDepth - originalDepth;
      
		float diff = clamp(origDiff * u_Local0.r/*5.0*/, 0.0, 1.0);
		diff = pow(diff, u_Local0.b);
		float mult = clamp(1.0 - diff, 0.25/*0.75*/, 1.0);
		color.rgb *= mult;
	}
	
	return color;
}

void main(void)
{
	gl_FragColor = PS_ProcessGeometryEnhance(var_TexCoords);
}

#endif

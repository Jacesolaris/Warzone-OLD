/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define tex2D(tex, coord) texture2D(tex, coord)
#define tex2Dlod(tex, coord) texture2D(tex, coord)
#define lerp(a, b, t) mix(a, b, t)
#define saturate(a) clamp(a, 0.0, 1.0)
#define mad(a, b, c) (a * b + c)
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define bool2 bvec2
#define bool3 bvec3
#define bool4 bvec4
#define frac fract

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

uniform sampler2D u_TextureMap;
//uniform sampler2D u_LevelsMap;
uniform sampler2D u_ScreenDepthMap;

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;

varying vec4		var_Local0; // CURRENT_PASS_NUMBER, 0, 0, 0

float				CURRENT_PASS_NUMBER = var_Local0.x;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

vec2 texCoord = var_TexCoords;
vec4 ScreenSize = vec4(var_Dimensions.x, 1.0 / var_Dimensions.x, var_Dimensions.y, 1.0 / var_Dimensions.y);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

float4	Timer;
float	EInteriorFactor = 1.0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool	shader_off = false;
bool	show_edges = false;

bool	smooth_edges = true;
//float	smooth_strength = 0.52;
float	smooth_strength = 0.37;
//float	farDepth = 100.0;
float	farDepth = 2500.0;
//float	farDepth = 1.0;
//float	limiterE = 0.1;
//float	limiterI = 0.1;
float	limiterE = 0.068;
float	limiterI = 0.072;

bool	luma_sharpen = true;
//float	BlurSigmaE = 0.87;
//float	BlurSigmaI = 0.98;
//float	SharpeningE = 1.67;
//float	SharpeningI = 1.87;
//float	ThresholdE =  1.0;
//float	ThresholdI = 0.0;
float	BlurSigmaE = 0.61;
float	BlurSigmaI = 0.82;
float	SharpeningE = 2.83;
float	SharpeningI = 2.75;
float	ThresholdE =  0.0;
float	ThresholdI = 0.0;

bool	use_letterbox = false;
float	letterbox_size = 7.8;

bool	use_noise = false;
float	GrainMotion = 0.004;
float	GrainSaturation = 1.0;
float	GrainIntensity = 0.004;

//////////////////
// HELPER FUNCS //
//////////////////

float3 RGBToHSL(float3 color)
{
	float3 hsl; // init to 0 to avoid warnings (and reverse if + remove first part)
	
	float fmin = min(min(color.r, color.g), color.b);
	float fmax = max(max(color.r, color.g), color.b);
	float delta = fmax - fmin;

	hsl.z = (fmax + fmin) / 2.0;

	if (delta == 0.0) //No chroma
	{
		hsl.x = 0.0;	// Hue
		hsl.y = 0.0;	// Saturation
	}
	else //Chromatic data
	{
		if (hsl.z < 0.5)
			hsl.y = delta / (fmax + fmin); // Saturation
		else
			hsl.y = delta / (2.0 - fmax - fmin); // Saturation
		
		float deltaR = (((fmax - color.r) / 6.0) + (delta / 2.0)) / delta;
		float deltaG = (((fmax - color.g) / 6.0) + (delta / 2.0)) / delta;
		float deltaB = (((fmax - color.b) / 6.0) + (delta / 2.0)) / delta;

		if (color.r == fmax )
			hsl.x = deltaB - deltaG; // Hue
		else if (color.g == fmax)
			hsl.x = (1.0 / 3.0) + deltaR - deltaB; // Hue
		else if (color.b == fmax)
			hsl.x = (2.0 / 3.0) + deltaG - deltaR; // Hue

		if (hsl.x < 0.0)
			hsl.x += 1.0; // Hue
		else if (hsl.x > 1.0)
			hsl.x -= 1.0; // Hue
	}

	return hsl;
}

float HueToRGB(float f1, float f2, float hue)
{
	if (hue < 0.0)
		hue += 1.0;
	else if (hue > 1.0)
		hue -= 1.0;
	float res;
	if ((6.0 * hue) < 1.0)
		res = f1 + (f2 - f1) * 6.0 * hue;
	else if ((2.0 * hue) < 1.0)
		res = f2;
	else if ((3.0 * hue) < 2.0)
		res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
	else
		res = f1;
	return res;
}

float3 HSLToRGB(float3 hsl)
{
	float3 rgb;
	
	if (hsl.y == 0.0)
		rgb = float3(hsl.z, hsl.z, hsl.z); // Luminance
	else
	{
		float f2;
		
		if (hsl.z < 0.5)
			f2 = hsl.z * (1.0 + hsl.y);
		else
			f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);
			
		float f1 = 2.0 * hsl.z - f2;
		
		rgb.r = HueToRGB(f1, f2, hsl.x + (1.0/3.0));
		rgb.g = HueToRGB(f1, f2, hsl.x);
		rgb.b= HueToRGB(f1, f2, hsl.x - (1.0/3.0));
	}
	
	return rgb;
}

// PI, required to calculate weight
const float PI = 3.1415926535897932384626433832795;

// Luminance Blend
float3 BlendLuma( float3 base, float3 blend )
{
	float3 HSLBase 	= RGBToHSL( base );
	float3 HSLBlend	= RGBToHSL( blend );
	return HSLToRGB( float3( HSLBase.x, HSLBase.y, HSLBlend.z ));
}

// Pseudo Random Number generator. 
float random(float2 uv)
{
	float noise1 = (frac(sin(dot(uv.x, 12.9898 * 2.0)) * 43758.5453));
	float noise2 = (frac(sin(dot(uv.y, 78.233 * 2.0)) * 43758.5453));
	return abs(noise1 + noise2) * 0.5;
}

// Linear depth
float linearDepth(float d, float n, float f)
{
	return (2.0 * n)/(f + n - d * (f - n));
}

//////////////////
//	 SHADERS	//
//////////////////

float4 PS_ProcessGaussianH()
{
	float px 			= ScreenSize.y; 
	float4 color		= vec4(0.0);
	float Depth			= 1.0 / tex2D( u_ScreenDepthMap, texCoord.xy ).x;
	float linDepth		= linearDepth( Depth, 0.5f, farDepth );
	
	float SigmaSum		= 0.0f;
	float sampleOffset	= 1.0f;
	
	//Gaussian
	float BlurSigma		= lerp( BlurSigmaE, BlurSigmaI, EInteriorFactor );
	BlurSigma			= max( BlurSigma * ( 1.0f - linDepth ), 0.3f );
	float3 Sigma;
	Sigma.x				= 1.0f / ( sqrt( 2.0f * PI ) * BlurSigma );
	Sigma.y				= exp( -0.5f / ( BlurSigma * BlurSigma ));
	Sigma.z				= Sigma.y * Sigma.y;
	
	//Center weight
	color				= tex2D(u_TextureMap, texCoord.xy);
	color				*= Sigma.x;
	SigmaSum			+= Sigma.x;
	Sigma.xy			*= Sigma.yz;

	for(int i = 0; i < 7; ++i) {
		color 			+= tex2D(u_TextureMap, texCoord.xy + float2(sampleOffset*px, 0.0)) * Sigma.x;
		color 			+= tex2D(u_TextureMap, texCoord.xy - float2(sampleOffset*px, 0.0)) * Sigma.x;
		SigmaSum		+= ( 2.0f * Sigma.x );
		sampleOffset	= sampleOffset + 1.0f;
		Sigma.xy		*= Sigma.yz;
		}
		
	color.xyz			/= SigmaSum;
	color.w				= 1.0f;
	return color;
}

float4 PS_ProcessGaussianV()
{
	//float sHeight		= ScreenSize.x * ScreenSize.w;
	float sHeight		= ScreenSize.z;
	float py 			= 1.0 / sHeight;
	//float py 			= ScreenSize.w;
	float4 color		= vec4(0.0);
	float Depth			= 1.0 / tex2D( u_ScreenDepthMap, texCoord.xy ).x;
	float linDepth		= linearDepth( Depth, 0.5f, farDepth );
	
	float SigmaSum		= 0.0f;
	float sampleOffset	= 1.0f;
	
	//Gaussian
	float BlurSigma		= lerp( BlurSigmaE, BlurSigmaI, EInteriorFactor );
	BlurSigma			= max( BlurSigma * ( 1.0f - linDepth ), 0.3f );
	float3 Sigma;
	Sigma.x				= 1.0f / ( sqrt( 2.0f * PI ) * BlurSigma );
	Sigma.y				= exp( -0.5f / ( BlurSigma * BlurSigma ));
	Sigma.z				= Sigma.y * Sigma.y;
	
	//Center weight
	color				= tex2D(u_TextureMap, texCoord.xy);
	color				*= Sigma.x;
	SigmaSum			+= Sigma.x;
	Sigma.xy			*= Sigma.yz;

	for(int i = 0; i < 7; ++i) {
		color 			+= tex2D(u_TextureMap, texCoord.xy + float2(0.0, sampleOffset*py)) * Sigma.x;
		color 			+= tex2D(u_TextureMap, texCoord.xy - float2(0.0, sampleOffset*py)) * Sigma.x;
		SigmaSum		+= ( 2.0f * Sigma.x );
		sampleOffset	= sampleOffset + 1.0f;
		Sigma.xy		*= Sigma.yz;
		}
	
	color.xyz			/= SigmaSum;
	color.w				= 1.0f;
	return color;
}

float4 PS_ProcessEdges()
{
	float Sharpening	= lerp( SharpeningE, SharpeningI, EInteriorFactor );
	float Threshold		= lerp( ThresholdE, ThresholdI, EInteriorFactor ) / 255;
	
	float4 color;
	//float4 orig			= tex2D(u_LevelsMap, texCoord.xy);
	float4 orig			= tex2D(u_TextureMap, texCoord.xy);
	float4 blurred		= tex2D(u_TextureMap, texCoord.xy) * 0.5;
	
	//Find edges
	orig.xyz			= saturate( orig.xyz );
	blurred.xyz			= saturate( blurred.xyz );
	float3 Edges		= max( saturate( orig.xyz - blurred.xyz ) - Threshold, 0.0f );
	float3 invBlur		= saturate( 1.0f - blurred.xyz );
	float3 originvBlur	= saturate( orig.xyz + invBlur.xyz );
	float3 invOrigBlur	= max( saturate( 1.0f - originvBlur.xyz ) - Threshold, 0.0f );
	
	float3 edges		= max(( saturate( Sharpening * Edges.xyz )) - ( saturate( Sharpening * invOrigBlur.xyz )), 0.0f );
	
	color.xyz			= edges.xyz;
	color.w				= 1.0f;
	return color;
}

float4 PS_ProcessSharpen1()
{
	//Smooth out edges with extremely light gaussian
	float4 edges		= tex2D(u_TextureMap, texCoord.xy);
	
	if (smooth_edges==false) return edges;
	
	float px 			= ScreenSize.y; 
	float4 color		= vec4(0.0);
	
	float SigmaSum		= 0.0f;
	float sampleOffset	= 1.0f;
	
	//Gaussian
	float BlurSigma		= smooth_strength;
	float3 Sigma;
	Sigma.x				= 1.0f / ( sqrt( 2.0f * PI ) * BlurSigma );
	Sigma.y				= exp( -0.5f / ( BlurSigma * BlurSigma ));
	Sigma.z				= Sigma.y * Sigma.y;
	
	//Center weight
	edges				*= Sigma.x;
	SigmaSum			+= Sigma.x;
	Sigma.xy			*= Sigma.yz;

	for(int i = 0; i < 5; ++i) {
		edges 			+= tex2D(u_TextureMap, texCoord.xy + float2(sampleOffset*px, 0.0)) * Sigma.x;
		edges 			+= tex2D(u_TextureMap, texCoord.xy - float2(sampleOffset*px, 0.0)) * Sigma.x;
		SigmaSum		+= ( 2.0f * Sigma.x );
		sampleOffset	= sampleOffset + 1.0f;
		Sigma.xy		*= Sigma.yz;
		}
		
	color.xyz			= edges.xyz / SigmaSum;
	color.w				= 1.0f;
	return color;

}

float4 PS_ProcessSharpen2()
{
	//float4 orig			= tex2D(u_LevelsMap, texCoord.xy);
	float4 orig			= tex2D(u_TextureMap, texCoord.xy);
	float4 edges		= tex2D(u_TextureMap, texCoord.xy) * 0.5;
	float limiter		= lerp( limiterE, limiterI, EInteriorFactor );
	
	//Smooth out edges (reduce aliasing) - expensive, likely
	//float sHeight		= ScreenSize.x * ScreenSize.w;
	float sHeight		= ScreenSize.z;
	float py 			= 1.0 / sHeight;
	float4 color		= vec4(0.0);
	
	if (smooth_edges==true) {
	
		float SigmaSum		= 0.0f;
		float sampleOffset	= 1.0f;
		
		//Gaussian
		float BlurSigma		= smooth_strength;
		float3 Sigma;
		Sigma.x				= 1.0f / ( sqrt( 2.0f * PI ) * BlurSigma );
		Sigma.y				= exp( -0.5f / ( BlurSigma * BlurSigma ));
		Sigma.z				= Sigma.y * Sigma.y;
		
		//Center weight
		edges				*= Sigma.x;
		SigmaSum			+= Sigma.x;
		Sigma.xy			*= Sigma.yz;

		for(int i = 0; i < 5; ++i) {
			edges 			+= tex2D(u_TextureMap, texCoord.xy + float2(0.0, sampleOffset*py)) * Sigma.x;
			edges 			+= tex2D(u_TextureMap, texCoord.xy - float2(0.0, sampleOffset*py)) * Sigma.x;
			SigmaSum		+= ( 2.0f * Sigma.x );
			sampleOffset	= sampleOffset + 1.0f;
			Sigma.xy		*= Sigma.yz;
			}
		
		edges.xyz			/= SigmaSum;
	}
	
	if (show_edges==true) {
		color.w 		= 1.0f;
		if(luma_sharpen==true) {
			//color.xyz 	= min( dot( edges.xyz, float3( 0.2126, 0.7152, 0.0722 )), limiter );
			color.x 	= min( dot( edges.x, 0.2126), limiter );
			color.y 	= min( dot( edges.y, 0.7152), limiter );
			color.z 	= min( dot( edges.z, 0.0722), limiter );
			} else {
			color.xyz 	= edges.xyz * limiter;
			}
		return color;
	}

	if (luma_sharpen==true) {
		float3 blend	= saturate( orig.xyz + min( dot( edges.xyz, float3( 0.2126, 0.7152, 0.0722 )), limiter ));
		color.xyz		= BlendLuma( orig.xyz, blend.xyz );
		} else {
		color.xyz		= saturate( orig.xyz + ( edges.xyz * limiter ));
	}
	
	if (shader_off==true) color.xyz = orig.xyz;

	color.w				= 1.0f;
	return color;

}
		
float4 PS_ProcessAfterFX()
{
	float4 color		= tex2D(u_TextureMap, texCoord.xy);
	
	if (use_noise==true)
		{
		float GrainTimerSeed 		= Timer.x * GrainMotion;
		float2 GrainTexCoordSeed 	= texCoord.xy * 1.0;
		
		//Generate grain seeds
		float2 GrainSeed1 	= GrainTexCoordSeed + float2( 0.0, GrainTimerSeed );
		float2 GrainSeed2 	= GrainTexCoordSeed + float2( GrainTimerSeed, 0.0 );
		float2 GrainSeed3 	= GrainTexCoordSeed + float2( GrainTimerSeed, GrainTimerSeed );
		
		//Generate pseudo random noise
		float GrainNoise1 	= random( GrainSeed1 );
		float GrainNoise2 	= random( GrainSeed2 );
		float GrainNoise3 	= random( GrainSeed3 );
		float GrainNoise4 	= ( GrainNoise1 + GrainNoise2 + GrainNoise3 ) * 0.333333333;
		
		//Combine results
		float3 GrainNoise 	= float3( GrainNoise4, GrainNoise4, GrainNoise4 );
		float3 GrainColor 	= float3( GrainNoise1, GrainNoise2, GrainNoise3 );
		
		//Add noise to color
		color.xyz 			+= ( lerp( GrainNoise, GrainColor, GrainSaturation ) * GrainIntensity ) - ( GrainIntensity * 0.5);
		}
	
	if (use_letterbox==true)
		{
			float offset 	= letterbox_size * 0.01;
			if (texCoord.y <= offset || texCoord.y >= (1.0 - offset)) color.xyzw = vec4(0.0);
		}
	
	color.w				= 1.0f;
	return color;
}

const vec3 dtt = vec3(65536.0,255.0,1.0);

int GET_RESULT(float A, float B, float C, float D)
{
	int x = 0; int y = 0; int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r+=1; 
	if (y <= 1) r-=1;
	return r;
} 


float reduce(vec3 color)
{ 
	return dot(color, dtt);
}

vec4 PS_SuperEagle()                                            
{
	// get texel size   	
	vec2 ps = vec2(0.999/var_Dimensions.x, 0.999/var_Dimensions.y);
  	
	// calculating offsets, coordinates
	vec2 dx = vec2( ps.x, 0.0); 
	vec2 dy = vec2( 0.0, ps.y);
	vec2 g1 = vec2( ps.x,ps.y);
	vec2 g2 = vec2(-ps.x,ps.y);	
	
	vec2 pixcoord  = texCoord/ps;	//VAR.CT
	vec2 fp        = fract(pixcoord);
	vec2 pC4       = texCoord-fp*ps;
	vec2 pC8       = pC4+g1;		//VAR.CT

	// Reading the texels
	vec3 C0 = texture2D(u_TextureMap,pC4-g1).xyz; 
	vec3 C1 = texture2D(u_TextureMap,pC4-dy).xyz;
	vec3 C2 = texture2D(u_TextureMap,pC4-g2).xyz;
	vec3 D3 = texture2D(u_TextureMap,pC4-g2+dx).xyz;
	vec3 C3 = texture2D(u_TextureMap,pC4-dx).xyz;
	vec3 C4 = texture2D(u_TextureMap,pC4   ).xyz;
	vec3 C5 = texture2D(u_TextureMap,pC4+dx).xyz;
	vec3 D4 = texture2D(u_TextureMap,pC8-g2).xyz;
	vec3 C6 = texture2D(u_TextureMap,pC4+g2).xyz;
	vec3 C7 = texture2D(u_TextureMap,pC4+dy).xyz;
	vec3 C8 = texture2D(u_TextureMap,pC4+g1).xyz;
	vec3 D5 = texture2D(u_TextureMap,pC8+dx).xyz;
	vec3 D0 = texture2D(u_TextureMap,pC4+g2+dy).xyz;
	vec3 D1 = texture2D(u_TextureMap,pC8+g2).xyz;
	vec3 D2 = texture2D(u_TextureMap,pC8+dy).xyz;
	vec3 D6 = texture2D(u_TextureMap,pC8+g1).xyz;

	vec3 p00,p10,p01,p11;

	// reducing vec3 to float	
	float c0 = reduce(C0);float c1 = reduce(C1);
	float c2 = reduce(C2);float c3 = reduce(C3);
	float c4 = reduce(C4);float c5 = reduce(C5);
	float c6 = reduce(C6);float c7 = reduce(C7);
	float c8 = reduce(C8);float d0 = reduce(D0);
	float d1 = reduce(D1);float d2 = reduce(D2);
	float d3 = reduce(D3);float d4 = reduce(D4);
	float d5 = reduce(D5);float d6 = reduce(D6);

	/*              SuperEagle code               */
	/*  Copied from the Dosbox source code        */
	/*  Copyright (C) 2002-2007  The DOSBox Team  */
	/*  License: GNU-GPL                          */
	/*  Adapted by guest(r) on 16.4.2007          */       
	if (c4 != c8) {
		if (c7 == c5) {
			p01 = p10 = C7;
			if ((c6 == c7) || (c5 == c2)) {
					p00 = 0.25*(3.0*C7+C4);
			} else {
					p00 = 0.5*(C4+C5);
			}

			if ((c5 == d4) || (c7 == d1)) {
					p11 = 0.25*(3.0*C7+C8);
			} else {
					p11 = 0.5*(C7+C8);
			}
		} else {
			p11 = 0.125*(6.0*C8+C7+C5);
			p00 = 0.125*(6.0*C4+C7+C5);

			p10 = 0.125*(6.0*C7+C4+C8);
			p01 = 0.125*(6.0*C5+C4+C8);
		}
	} else {
		if (c7 != c5) {
			p11 = p00 = C4;

			if ((c1 == c4) || (c8 == d5)) {
					p01 = 0.25*(3.0*C4+C5);
			} else {
					p01 = 0.5*(C4+C5);
			}

			if ((c8 == d2) || (c3 == c4)) {
					p10 = 0.25*(3.0*C4+C7);
			} else {
					p10 = 0.5*(C7+C8);
			}
		} else {
			int r = 0;
			r += GET_RESULT(c5,c4,c6,d1);
			r += GET_RESULT(c5,c4,c3,c1);
			r += GET_RESULT(c5,c4,d2,d5);
			r += GET_RESULT(c5,c4,c2,d4);

			if (r > 0) {
					p01 = p10 = C7;
					p00 = p11 = 0.5*(C4+C5);
			} else if (r < 0) {
					p11 = p00 = C4;
					p01 = p10 = 0.5*(C4+C5);
			} else {
					p11 = p00 = C4;
					p01 = p10 = C7;
			}
		}
	}



	// Distributing the four products
	
	if (fp.x < 0.50)
		{ if (fp.y < 0.50) p10 = p00;}
	else
		{ if (fp.y < 0.50) p10 = p01; else p10 = p11;}


	// OUTPUT
	return vec4(p10, 1);	
}

void main()
{
	vec4 color;

	if (CURRENT_PASS_NUMBER == 0) {
		//color = PS_ProcessGaussianH();
		color = tex2D(u_TextureMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 1) {
		//color = PS_ProcessGaussianV();
		color = tex2D(u_TextureMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 2) {
		//color = PS_ProcessEdges();
		color = tex2D(u_TextureMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 3) {
		//color = PS_ProcessSharpen1();
		color = tex2D(u_TextureMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 4) {
		//color = PS_ProcessSharpen2();
		color = tex2D(u_TextureMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 5) {
		//color = PS_ProcessAfterFX();
		//color = tex2D(u_TextureMap, texCoord.xy);
		color = PS_SuperEagle();
	}

	gl_FragColor.rgb = color.rgb;
	gl_FragColor.a = 1.0;
}

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

bool	use_vibrance = true;
float	Vibrance = 1.0;

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

float4 PS_ProcessVibrance()
{
	float4	res;
	//float3	origcolor = tex2D(u_LevelsMap, texCoord.xy).rgb;
	float3	origcolor = tex2D(u_TextureMap, texCoord.xy).rgb;
	float3	lumCoeff = float3(0.212656, 0.715158, 0.072186);  				//Calculate luma with these values
	
	float	max_color = max(origcolor.r, max(origcolor.g,origcolor.b)); 	//Find the strongest color
	float	min_color = min(origcolor.r, min(origcolor.g,origcolor.b)); 	//Find the weakest color

	float	color_saturation = max_color - min_color; 						//Saturation is the difference between min and max

	float	luma = dot(lumCoeff, origcolor.rgb); 							//Calculate luma (grey)

	res.rgb = lerp(vec3(luma), origcolor.rgb, (1.0 + (Vibrance * (1.0 - (sign(Vibrance) * color_saturation))))); 	//Extrapolate between luma and original by 1 + (1-saturation) - current
	
	res.w=1.0;
	return res;
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

void main()
{
	vec4 color;

	if (CURRENT_PASS_NUMBER == 0) {
		color = PS_ProcessGaussianH();
		//color = tex2D(u_TextureMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 1) {
		color = PS_ProcessGaussianV();
		//color = tex2D(u_TextureMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 2) {
		color = PS_ProcessEdges();
		//color = tex2D(u_TextureMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 3) {
		color = PS_ProcessSharpen1();
	} else if (CURRENT_PASS_NUMBER == 4) {
		color = PS_ProcessSharpen2();
	} else if (CURRENT_PASS_NUMBER == 5) {
		//color = PS_ProcessVibrance();
		color = tex2D(u_TextureMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 6) {
		//color = PS_ProcessAfterFX();
		color = tex2D(u_TextureMap, texCoord.xy);
	}

	gl_FragColor.rgb = color.rgb;
	gl_FragColor.a = 1.0;
}

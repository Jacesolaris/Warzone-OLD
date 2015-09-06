//#extension GL_ARB_texture_rectangle : enable
uniform sampler2D u_TextureMap;
uniform sampler2D u_ScreenDepthMap;
uniform vec4      u_Color;
uniform vec2      u_AutoExposureMinMax;
uniform vec3      u_ToneMinAvgMaxLinear;

uniform vec4	u_ViewInfo; // zfar / znear, zfar
uniform vec2	u_Dimensions;
uniform vec4	u_Local0; // depthScale, 0, 0, 0

varying vec2   var_TexCoords;
varying vec4	var_ViewInfo; // zfar / znear, zfar
varying vec2   var_Dimensions;
varying vec4   var_Local0; // depthScale, 0, 0, 0
varying vec4   var_Local1; // AnaglyphType, AnaglyphMinDistance, AnaglyphMaxDistance, AnaglyphParallax

vec2 texCoord = var_TexCoords;

float near = u_ViewInfo.x;
float far = u_ViewInfo.y;
float viewWidth = var_Dimensions.x;
float viewHeight = var_Dimensions.y;

float AnaglyphSeperation = var_Local0.r;
float AnaglyphRed = var_Local0.g;
float AnaglyphGreen = var_Local0.b;
float AnaglyphBlue = var_Local0.a;

float AnaglyphType = var_Local1.r;
float AnaglyphMinDistance = var_Local1.g;
float AnaglyphMaxDistance = var_Local1.b;
float AnaglyphParallax = var_Local1.a;

//#define float4x4 row_major float4x4 mul
//#define float4x4 row_major float4x4 mul(pos,transformMatrix);
//#define mul(a,b) mul((b),transpose(a))
//#define mul(a,b) a*b

float DepthToZPosition(float depth) {
	vec2 camerarange = vec2(AnaglyphMinDistance, AnaglyphMaxDistance);
	return camerarange.x / (camerarange.y - depth * (camerarange.y - camerarange.x)) * camerarange.y;
}

void main(void)
{
	if (AnaglyphType <= 1.0)
	{// Simple anaglyph 3D not using depth map...
		vec2 Depth = vec2(1.0f / viewWidth, 1.0f / viewHeight); // calculated from screen size now
		vec4 Anaglyph = texture2D(u_TextureMap, texCoord).rgba;

#if 1
		// Dubois anaglyph method. best option - not working...
		// Authors page: http://www.site.uottawa.ca/~edubois/anaglyph/
		// This method depends on the characteristics of the display device
		// and the anaglyph glasses.
		// The matrices below are taken from "A Uniform Metric for Anaglyph Calculation"
		// by Zhe Zhang and David F. McAllister, Proc. Electronic Imaging 2006, available
		// here: http://research.csc.ncsu.edu/stereographics/ei06.pdf
		// These matrices are supposed to work fine for LCD monitors and most red-cyan
		// glasses. See also the remarks in http://research.csc.ncsu.edu/stereographics/LS.pdf
		// (where slightly different values are used).
    
		vec4 color_l = texture2D(u_TextureMap, vec2(texCoord + (vec2(-AnaglyphParallax,0)*Depth))).rgba;

		// Right Eye (Cyan)
		vec4 color_r = texture2D(u_TextureMap, vec2(texCoord + (vec2(AnaglyphParallax,0)*Depth))).rgba;

		mat3x3 m0 = mat3x3(
                0.4155, -0.0458, -0.0545,
                0.4710, -0.0484, -0.0614,
                0.1670, -0.0258,  0.0128);
 
		mat3x3 m1 = mat3x3(
                -0.0109, 0.3756, -0.0651,
                -0.0365, 0.7333, -0.1286,
                -0.0060, 0.0111,  1.2968);
                    
		// calculate resulting pixel
		gl_FragColor = vec4((m0 * color_r.rgb) + (m1 * color_l.rgb), 1.0);

#else

		// Setting RGB channel colors
		float red = dot(Anaglyph.rgb, vec3(2.55, 0, 0));
		float green = dot(Anaglyph.rgb, vec3(0, 2.55, 0));
		float blue = dot(Anaglyph.rgb, vec3(0, 0, 2.55));

		// Setting the RGB channel powers
		vec4 red2 = vec4(red) * 0.111;
		vec4 green2 = vec4(green) * 0.111;
		vec4 blue2 = vec4(blue) * 0.111;

		// Left Eye (Red)
		vec4 LeftEye = texture2D(u_TextureMap, vec2(texCoord + (vec2(-AnaglyphSeperation,0)*Depth))).rgba;
		red2 = max(red2, LeftEye) - vec4(AnaglyphRed);

		// Right Eye (Cyan)
		vec4 RightEye = texture2D(u_TextureMap, vec2(texCoord + (vec2(AnaglyphSeperation,0)*Depth))).rgba;
		green2 = max(green2, RightEye) - vec4(AnaglyphGreen);
		blue2 = max(blue2, RightEye) - vec4(AnaglyphBlue);
		vec4 cyan = (green2 + blue2) / 2.0;

		// Combine
		Anaglyph.r = cyan.r;
		Anaglyph.g = red2.g;
		Anaglyph.b = red2.b;
//		Anaglyph.a = max(red2.a,cyan.a);
		Anaglyph.a = 1.0;
	
		gl_FragColor = Anaglyph;
//		gl_FragColor = red2;
#endif

	} 
	else 
	{// Real anaglyph 3D using real depth map...
		vec2 ps = vec2(1.0f / viewWidth, 1.0f / viewHeight); // calculated from screen size now
		vec2 tc = texCoord;
	
		float depth = texture2D( u_ScreenDepthMap, tc ).x;
		depth=pow(depth, 255);
		//gl_FragColor = vec4(vec3((depth*AnaglyphParallax)*(depth*AnaglyphParallax)), 1.0);
		//gl_FragColor = vec4(vec3(depth), 1.0);
		//return;

		//float lineardepth = DepthToZPosition( depth );
		float lineardepth = depth;
	
		lineardepth*=AnaglyphParallax;
		float shift=lineardepth;

		vec4 color_l = texture2D(u_TextureMap, vec2(tc.x-ps.x*shift,tc.y));
		vec4 color_r = texture2D(u_TextureMap, vec2(tc.x+ps.x*shift,tc.y));

#if 1
		// Dubois anaglyph method. Best option...
		// Authors page: http://www.site.uottawa.ca/~edubois/anaglyph/
		// This method depends on the characteristics of the display device
		// and the anaglyph glasses.
		// The matrices below are taken from "A Uniform Metric for Anaglyph Calculation"
		// by Zhe Zhang and David F. McAllister, Proc. Electronic Imaging 2006, available
		// here: http://research.csc.ncsu.edu/stereographics/ei06.pdf
		// These matrices are supposed to work fine for LCD monitors and most red-cyan
		// glasses. See also the remarks in http://research.csc.ncsu.edu/stereographics/LS.pdf
		// (where slightly different values are used).
    
		mat3x3 m0 = mat3x3(
                0.4155, -0.0458, -0.0545,
                0.4710, -0.0484, -0.0614,
                0.1670, -0.0258,  0.0128);
 
		mat3x3 m1 = mat3x3(
                -0.0109, 0.3756, -0.0651,
                -0.0365, 0.7333, -0.1286,
                -0.0060, 0.0111,  1.2968);
                    
		gl_FragColor = vec4((m0 * color_r.rgb) + (m1 * color_l.rgb), 1.0);
#else
		// Optimized Anaglyph...
		vec4 out_Color = vec4(0.0, 0.0, 0.0, 1.0);
    
		//if (AnaglyphOptimizedGammaCorrect==false)
		//{
		//	out_Color.r = 0.7*color_r.g + 0.3*color_r.b;            //without gamma correction
		//}
		//else
		//{
			out_Color.r = pow(0.7*color_r.g + 0.3*color_r.b, 1.5);  //with gamma correction
		//}
    
		out_Color.g = color_l.g;
		out_Color.b = color_l.b;
		gl_FragColor = out_Color;
#endif
	}
}

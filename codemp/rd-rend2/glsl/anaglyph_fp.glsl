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
varying vec4   var_Local1; // AnaglyphType, AnaglyphPower, AnaglyphMax, 0

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
float AnaglyphPower = var_Local1.g;
float AnaglyphMax = var_Local0.r;//var_Local1.b;

float DepthToZPosition(float depth) {
	vec2 camerarange = vec2(AnaglyphPower, AnaglyphPower);
	return camerarange.x / (camerarange.y - depth * (camerarange.y - camerarange.x)) * camerarange.y;
}

void main(void)
{
	if (AnaglyphType <= 1.0)
	{// Simple anaglyph 3D not using depth map...
		vec2 Depth = vec2(1.0f / viewWidth, 1.0f / viewHeight); // calculated from screen size now
		vec4 Anaglyph = texture2D(u_TextureMap, texCoord).rgba;

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
	} 
	else 
	{// Real anaglyph 3D using real depth map...
		vec2 ps = vec2(1.0f / viewWidth, 1.0f / viewHeight); // calculated from screen size now
		vec2 tc = texCoord;
	
		float depth = texture2D( u_ScreenDepthMap, tc ).x;
		float lineardepth = DepthToZPosition( depth );
	
		float shift=clamp(lineardepth,0,AnaglyphMax);

		vec4 color_l = texture2D(u_TextureMap, vec2(tc.x-ps.x*shift,tc.y));
		vec4 color_r = texture2D(u_TextureMap, vec2(tc.x+ps.x*shift,tc.y));

		vec4 left = color_l * vec4(0,1,1,1);
		vec4 right = color_r * vec4(1,0,0,1);

		gl_FragColor = left + right;
	}
}

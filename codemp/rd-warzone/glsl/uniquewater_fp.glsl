#define USE_WATERMAP

uniform vec4		u_Local6; // blah, blah, blah, MAP_WATER_LEVEL
uniform vec4		u_Local10;

varying vec2		var_TexCoords;
varying vec3		var_vertPos;
varying vec3		var_Normal;

out vec4 out_Glow;
out vec4 out_Normal;
out vec4 out_Position;

// Maximum waves amplitude
const float maxAmplitude = 6.0;//4.0;

void main()
{
	out_Glow = vec4(0.0);
	out_Normal = vec4(var_Normal * 0.5 + 0.5, 0.75);
#if defined(USE_WATERMAP)
	out_Color = vec4(0.0059, 0.3096, 0.445, 0.5);
	out_Position = vec4(var_vertPos.xyz, 1.0);
#else //!defined(USE_WATERMAP)
	out_Color = vec4(0.0059, 0.3096, 0.445, 0.1);
	out_Position = vec4(var_vertPos, MATERIAL_LAST / 13.0);
#endif //defined(USE_WATERMAP)
}

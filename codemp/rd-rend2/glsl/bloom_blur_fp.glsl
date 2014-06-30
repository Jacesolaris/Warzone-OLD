uniform sampler2D u_DiffuseMap;

varying vec4	var_Local0;
varying vec2	var_Dimensions;
varying vec2	var_TexCoords;

void main(void)
{
/*
  vec3 col0 = texture2D(u_DiffuseMap, var_TexCoords.xy).rgb;
  vec3 col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + (var_Local0.st / var_Dimensions)).rgb;
  vec3 col2 = texture2D(u_DiffuseMap, var_TexCoords.xy + ((var_Local0.st * 2.0) / var_Dimensions)).rgb;
  vec3 col3 = texture2D(u_DiffuseMap, var_TexCoords.xy - (var_Local0.st / var_Dimensions)).rgb;
  vec3 col4 = texture2D(u_DiffuseMap, var_TexCoords.xy - ((var_Local0.st * 2.0) / var_Dimensions)).rgb;
  gl_FragColor.rgb = (col0 / 2) + (col1 + col2 + col3 + col4) / 8.5;
  gl_FragColor.a	= 1.0;
*/

  vec2 X_AXIS = vec2(1.0, 0.0);
  vec2 Y_AXIS = vec2(0.0, 1.0);

  vec3 color = vec3(0.0, 0.0, 0.0);

  for (float width = 0; width < var_Local0.z; width += 1.0)
  {
	vec3 col0 = texture2D(u_DiffuseMap, var_TexCoords.xy).rgb;
	vec3 col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + ((X_AXIS.xy * width) / var_Dimensions)).rgb;
	vec3 col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - ((X_AXIS.xy * width) / var_Dimensions)).rgb;
	color.rgb += (col0 / 2) + (col1 + col2) / 4;
  }

  color.rgb /= var_Local0.z;

  vec3 color2 = vec3(0.0, 0.0, 0.0);

  for (float width = 0; width < var_Local0.z; width += 1.0)
  {
	vec3 col0 = texture2D(u_DiffuseMap, var_TexCoords.xy).rgb;
	vec3 col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + ((Y_AXIS.xy * width) / var_Dimensions)).rgb;
	vec3 col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - ((Y_AXIS.xy * width) / var_Dimensions)).rgb;
	color2.rgb += (col0 / 2) + (col1 + col2) / 4;
  }

  color2.rgb /= var_Local0.z;

  gl_FragColor.rgb = (color + color2) / 2.0;
  gl_FragColor.a	= 1.0;
}

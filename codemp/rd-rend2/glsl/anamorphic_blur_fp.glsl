uniform sampler2D u_DiffuseMap;

varying vec4	var_Local0; // scan_pixel_size_x, scan_pixel_size_y, scan_width, is_ssgi
varying vec2	var_Dimensions;
varying vec2	var_TexCoords;

void main(void)
{
	float NUM_VALUES = 1.0;
	vec2 PIXEL_OFFSET = vec2(1.0 / var_Dimensions.x, 1.0 / var_Dimensions.y);

	vec3 col0 = texture2D(u_DiffuseMap, var_TexCoords.xy).rgb;
	//gl_FragColor.rgb = vec3(0.0, 0.0, 0.0);
	gl_FragColor.rgb = col0.rgb;

	if (var_Local0.a >= 1.0 && var_Local0.a <= 2.0)
	{// New SSGI blur...
		vec2 AXIS = vec2(1.0, 0.0);

		for (float width = 1.0; width <= var_Local0.z; width += 1.0)
		{
			vec3 col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + ((AXIS.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - ((AXIS.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 color = (col0 / 2) + (col1 + col2) / 4;

			if (var_Local0.a >= 2.0)
			{// Final pass...
				gl_FragColor.rgb += color;
				NUM_VALUES += 1.0;
			}
			else if (length(color) > length(gl_FragColor.rgb/NUM_VALUES))
			{// Special SSGI blur...
				if (length(color) <= 0.2)
				{// Amplify non-white colors - the idea of this SSGI blur is to add color to the scene (sabers, weapon bolts, explosions, etc)...
					gl_FragColor.rgb += clamp(color * 20.0, 0.0, 1.0);
					NUM_VALUES += 1.0;
				}
				else
				{// Accept a tiny bit of white???
					gl_FragColor.rgb += (color * 0.15);
					NUM_VALUES += 1.0;
				}
			}
		}

		AXIS = vec2(0.0, 1.0);

		for (float width = 1.0; width <= var_Local0.z; width += 1.0)
		{
			vec3 col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + ((AXIS.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - ((AXIS.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 color = (col0 / 2) + (col1 + col2) / 4;

			if (var_Local0.a >= 2.0)
			{// Final pass...
				gl_FragColor.rgb += color;
				NUM_VALUES += 1.0;
			}
			else if (length(color) > length(gl_FragColor.rgb/NUM_VALUES))
			{// Special SSGI blur...
				if (length(color) <= 0.2)
				{// Amplify non-white colors - the idea of this SSGI blur is to add color to the scene (sabers, weapon bolts, explosions, etc)...
					gl_FragColor.rgb += clamp(color * 20.0, 0.0, 1.0);
					NUM_VALUES += 1.0;
				}
				else
				{// Accept a tiny bit of white???
					gl_FragColor.rgb += (color * 0.15);
					NUM_VALUES += 1.0;
				}
			}
		}

		AXIS = vec2(1.0, 1.0);

		for (float width = 1.0; width <= var_Local0.z; width += 1.0)
		{
			vec3 col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + ((AXIS.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - ((AXIS.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 color = (col0 / 2) + (col1 + col2) / 4;

			if (var_Local0.a >= 2.0)
			{// Final pass...
				gl_FragColor.rgb += color;
				NUM_VALUES += 1.0;
			}
			else if (length(color) > length(gl_FragColor.rgb/NUM_VALUES))
			{// Special SSGI blur...
				if (length(color) <= 0.2)
				{// Amplify non-white colors - the idea of this SSGI blur is to add color to the scene (sabers, weapon bolts, explosions, etc)...
					gl_FragColor.rgb += clamp(color * 20.0, 0.0, 1.0);
					NUM_VALUES += 1.0;
				}
				else
				{// Accept a tiny bit of white???
					gl_FragColor.rgb += (color * 0.15);
					NUM_VALUES += 1.0;
				}
			}
		}

		AXIS = vec2(-1.0, 1.0);

		for (float width = 1.0; width <= var_Local0.z; width += 1.0)
		{
			vec3 col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + ((AXIS.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - ((AXIS.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 color = (col0 / 2) + (col1 + col2) / 4;

			if (var_Local0.a >= 2.0)
			{// Final pass...
				gl_FragColor.rgb += color;
				NUM_VALUES += 1.0;
			}
			else if (length(color) > length(gl_FragColor.rgb/NUM_VALUES))
			{// Special SSGI blur...
				if (length(color) <= 0.2)
				{// Amplify non-white colors - the idea of this SSGI blur is to add color to the scene (sabers, weapon bolts, explosions, etc)...
					gl_FragColor.rgb += clamp(color * 20.0, 0.0, 1.0);
					NUM_VALUES += 1.0;
				}
				else
				{// Accept a tiny bit of white???
					gl_FragColor.rgb += (color * 0.15);
					NUM_VALUES += 1.0;
				}
			}
		}
	}
	else
	{// Normal anamorphic blur (or old SSGI blur)...
		for (float width = 1.0; width <= var_Local0.z; width += 1.0)
		{
			float dist_mult = clamp(1.0 - (width / var_Local0.z), 0.0, 1.0);
			vec3 col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + ((var_Local0.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - ((var_Local0.xy * width) * PIXEL_OFFSET)).rgb;
			vec3 add_color = ((col0 / 2) + ((col1 + col2) * (dist_mult * 2.0))) / 4;

			if (var_Local0.a == 0.0)
			{// Normal anamorphic or SSGI old mode final pass...
				vec3 BLUE_SHIFT_MOD = vec3(0.333, 0.333, 3.0);
				vec3 add_color_blue = clamp(add_color * (BLUE_SHIFT_MOD * (1.0 - clamp(dist_mult*3.5, 0.0, 1.0))), 0.0, 1.0);
				add_color.rgb += clamp(((add_color + add_color + add_color_blue) * 0.37), 0.0, 1.0);

				gl_FragColor.rgb += add_color;
				NUM_VALUES += 1.0;
			}
			else if (length(add_color) > length(gl_FragColor.rgb/NUM_VALUES))
			{// Special SSGI blur...
				if (length(add_color) <= 0.2)
				{// Amplify non-white colors - the idea of this SSGI blur is to add color to the scene (sabers, weapon bolts, explosions, etc)...
					gl_FragColor.rgb += clamp(add_color * 10.0, 0.0, 1.0);
					NUM_VALUES += 1.0;
				}
				else
				{// Accept a tiny bit of white???
					gl_FragColor.rgb += (add_color * 0.15);
					NUM_VALUES += 1.0;
				}
			}
		}
	}

	gl_FragColor.rgb /= NUM_VALUES;//var_Local0.z;
	gl_FragColor.a	= 1.0;
}

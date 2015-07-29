//some stuff needed for kami-batch
varying vec2		var_TexCoords;
 
//make sure to have a u_Dimensions uniform set to the image size
uniform vec2		u_Dimensions;

uniform sampler2D	u_DiffuseMap;

vec4 generateEnhancedNormal( vec2 fragCoord )
{// Generates a normal map with enhanced edges... Not so good for parallax...
	vec2 uv = fragCoord.xy;
    float u = uv.x;
    float v = uv.y;
    
    float threshold = 0.085;
    vec2 px = vec2(1.0/u_Dimensions.x, 1.0/u_Dimensions.y);
    
    vec3 rgb = texture2D(u_DiffuseMap, uv).rgb;
    vec3 bw = vec3(1);
    vec3 bw2 = vec3(1);

    vec3 rgbUp = texture2D(u_DiffuseMap, vec2(u,v+px.y)).rgb;
    vec3 rgbDown = texture2D(u_DiffuseMap, vec2(u,v-px.y)).rgb;
    vec3 rgbLeft = texture2D(u_DiffuseMap, vec2(u+px.x,v)).rgb;
    vec3 rgbRight = texture2D(u_DiffuseMap, vec2(u-px.x,v)).rgb;

    float rgbAvr = (rgb.r + rgb.g + rgb.b) / 3.;
    float rgbUpAvr = (rgbUp.r + rgbUp.g + rgbUp.b) / 3.;
    float rgbDownAvr = (rgbDown.r + rgbDown.g + rgbDown.b) / 3.;
    float rgbLeftAvr = (rgbLeft.r + rgbLeft.g + rgbLeft.b) / 3.;
    float rgbRightAvr = (rgbRight.r + rgbRight.g + rgbRight.b) / 3.;

    float dx = abs(rgbRightAvr - rgbLeftAvr);
    float dy = abs(rgbUpAvr - rgbDownAvr);
    
    if (dx > threshold)
        bw = vec3(1);
    else if (dy > threshold)
        bw = vec3(1);
    else
        bw = vec3(0);
    
    // o.5 + 0.5 * acts as a remapping function
    bw = 0.5 + 0.5*normalize( vec3(rgbRightAvr - rgbLeftAvr, 100.0*px.x, rgbUpAvr - rgbDownAvr) ).xzy;
    
    return vec4(bw,0);
}

vec4 generateBumpyNormal( vec2 fragCoord )
{// Generates an extra bumpy normal map...
	vec2 tex_offset = vec2(1.0 / u_Dimensions.x, 1.0 / u_Dimensions.y);
	vec2 uv = fragCoord;
	//uv.y=1.0-uv.y;
	
	float x=1.;
	float y=1.;
	
	float M =abs(texture2D(u_DiffuseMap, uv + vec2(0., 0.)*tex_offset).r); 
	float L =abs(texture2D(u_DiffuseMap, uv + vec2(x, 0.)*tex_offset).r);
	float R =abs(texture2D(u_DiffuseMap, uv + vec2(-x, 0.)*tex_offset).r);	
	float U =abs(texture2D(u_DiffuseMap, uv + vec2(0., y)*tex_offset).r);
	float D =abs(texture2D(u_DiffuseMap, uv + vec2(0., -y)*tex_offset).r);
	float X = ((R-M)+(M-L))*.5;
	float Y = ((D-M)+(M-U))*.5;
	
	float strength =.01;
	vec4 N = vec4(normalize(vec3(X, Y, strength)), 1.0);
//	vec4 N = vec4(normalize(vec3(X, Y, .01))-.5, 1.0);

	vec4 col = vec4(N.xyz * 0.5 + 0.5,1.);
	return col;
}

float SampleHeight(vec2 t)
{// Provides enhanced parallax depths without stupid distortions... Also provides a nice backup specular map...
	vec3 color = texture2D(u_DiffuseMap, t).rgb;
#define const_1 ( 16.0 / 255.0)
#define const_2 (255.0 / 219.0)
	vec3 color2 = ((color - const_1) * const_2);
#define const_3 ( 125.0 / 255.0)
#define const_4 (255.0 / 115.0)
	color = ((color - const_3) * const_4);

	// 1st half "color * color" darkens, 2nd half "* color * 5.0" increases the mids...
	color = clamp(color * color * (color * 5.0), 0.0, 1.0);

	vec3 orig_color = color + color2;

	// Lightens the new mixed version...
	orig_color = clamp(orig_color * 2.5, 0.0, 1.0);

	// Darkens the whole thing a litttle...
	float combined_color2 = orig_color.r + orig_color.g + orig_color.b;
	combined_color2 /= 4.0;

	// Returns inverse of the height. Result is mostly around 1.0 (so we don't stand on a surface far below us), with deep dark areas (cracks, edges, etc)...
	float height = clamp(1.0 - combined_color2, 0.0, 1.0);
	return height;

	/*
	// Mix it with an extra-bumpy normal map... (UQ1: decided not worth the fps loss)
	float norm = generateBumpyNormal( t ).r;
	// I don't want this much bumpiness (and this is to be used as a multipier), so move the whole thing closer to 1.0...
	norm *= 0.5;
	norm += 0.5;
	norm *= 0.5;
	norm += 0.5;
	return (height + (norm * height)) / 2.0;
	*/
}

void main ( void )
{
	vec4 enhanced = generateEnhancedNormal(var_TexCoords.xy);
	//vec4 bumpy = generateBumpyNormal(var_TexCoords.xy);
	vec4 normal = enhanced;
	//vec4 normal = (enhanced + bumpy) / 2.0;
	//normal = 1.0 - normal;
	normal.a = SampleHeight(var_TexCoords.xy);
	//vec4 normal = vec4(var_TexCoords.xy,var_TexCoords.xy);
	//normal.a = 1.0;
	gl_FragColor = normal;
	//gl_FragColor = vec4(normal.a,normal.a,normal.a,1.0);
}

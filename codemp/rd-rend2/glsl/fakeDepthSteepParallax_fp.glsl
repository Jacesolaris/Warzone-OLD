uniform sampler2D	u_TextureMap;
uniform sampler2D	u_LevelsMap;
uniform vec4		u_Color;
uniform vec2		u_AutoExposureMinMax;
uniform vec3		u_ToneMinAvgMaxLinear;

varying vec2		var_TexCoords;
varying vec4		var_ViewInfo; // zfar / znear, zfar
varying vec2		var_Dimensions;
varying vec4		var_Local0; // depthScale, depthMultiplier, 0, 0
varying vec4		var_Local1; // eyex, eyey, eyexz, grassSway

vec2 texCoord = var_TexCoords;

float near = var_ViewInfo.x;
float far = var_ViewInfo.y;
float viewWidth = var_Dimensions.x;
float viewHeight = var_Dimensions.y;

//float depthScale = 12.0
float depthScale = var_Local0.r;
float depthMultiplier = var_Local0.g;
float grassSway = var_Local1.a;

vec4 GetHDR( vec3 col2, vec2 coord )
{
	vec3 lumask = vec3(0.2125, 0.7154, 0.0721);
	float USE_FILTER = 1.0f;
	vec3 color = col2.rgb;

	if (USE_FILTER > 0.0)
	{
		vec2 recipres = vec2(1.0f / viewWidth, 1.0f / viewHeight);
		vec2 tc = coord.st, add = recipres;

		color = 0.25*(
			texture2D(u_TextureMap, tc+add.xy)+
			texture2D(u_TextureMap, tc-add.xy)+
			texture2D(u_TextureMap, tc+add.yx)+
			texture2D(u_TextureMap, tc-add.yx)
		).rgb;
	}

	float bright = log(dot(color, lumask) + 0.25);
	color += 0.2*sin(-10.0*color/(color+2.0));
	return vec4(color, bright);
}

vec4 GetEmboss( vec3 col2, float embossScale, float embossFactor_OLD, vec2 coord )
{
	vec2 embossFactor = vec2(1.0f / viewWidth, 1.0f / viewHeight); // calculated from screen size now
	vec4 out_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec3 col1 = texture2D(u_TextureMap, coord.xy - vec2(embossFactor)).rgb;
//	vec3 col2 = texture2D(u_TextureMap, coord.xy).rgb;
	vec3 col3 = texture2D(u_TextureMap, coord.xy + vec2(embossFactor)).rgb;
	vec3 colEmboss = col1 * 2.0 - col2 - col3;
	float colDot = max( 0.0, dot( colEmboss, vec3( 0.333, 0.333, 0.333 ) ) ) * embossScale;
	vec3 colFinal = col2 - vec3( colDot, colDot, colDot ) ;
	float luminance = dot( col2, vec3( 0.6, 0.2, 0.2 ) );
	out_color.rgb = mix( colFinal, col2, luminance * luminance );
	out_color.a	= 1.0;
	return out_color;
}

float GetHeightFor( vec4 colorInput )
{
    float scale = 0.0025;
	float SteepParallaxScale = 10.0;
    float height = ((colorInput.r + colorInput.g + colorInput.b) * 0.33333333f);
	height = (height * 2.0) - 1.0;
    return height;
}

float GetNormalFor( vec4 colorInput )
{
    // Grey scale...
    return ((colorInput.r + colorInput.g + colorInput.b) * 0.33333333f);
}

vec3 GetEnhancedColorAt( vec2 coord )
{
		vec4 colorInput = texture2D(u_TextureMap, coord);
		return colorInput.rgb;
} 

vec3 GetNormal( vec2 coord )
{
	vec4 colorInput = texture2D(u_TextureMap, coord);
	vec4 embossColor = GetEmboss( colorInput.rgb, 30.0, 0.001, coord ); // add black borders
	embossColor = GetHDR( embossColor.rgb, coord ); // for more contrast
	vec3 ret = embossColor.xyz;
	(ret.xy = ret.xy * 2.0) - 1.0;
	ret.y *= -1.0;
	return normalize( ret );
}
 
float current_height = 0.0;

/*
float GetHeight( vec2 coord )
{
	float out_color;
    vec4 sum = vec4(0);
	float thresh = 1.2;
	float scale = depthMultiplier;
    int x=0;
    int y=0;

	vec2 recipres = vec2(1.0f / viewWidth, 1.0f / viewHeight);

    // mess of for loops due to gpu compiler/hardware limitations
	for(y=-2; y<=2; y+=2)
	{
		for(x=-2; x<=2; x+=2) sum+=texture2D(u_TextureMap, coord + (vec2(x,y) * recipres));
	}

	sum/=((5*5)*0.5);

    vec4 s=texture2D(u_TextureMap, coord);

    // use the blurred colour if it's bright enough
	vec4 diff = s - sum;

	if (diff.r < 0.0) diff.r = 0.0 - diff.r;
	if (diff.g < 0.0) diff.g = 0.0 - diff.g;
	if (diff.b < 0.0) diff.b = 0.0 - diff.b;

	float fdiff = (diff.r + diff.g + diff.b) * 0.33333;
	if (fdiff < 0.0) fdiff = 0.0 - fdiff;

	out_color = (scale*fdiff);
	out_color = 1.0 - out_color;
	out_color *= 1.0 - (sum.r + sum.g + sum.b);

	return (out_color * 2.0) - 1.0;
}
*/

float GetHeight(vec2 coord)
{
#ifdef METHOD1
	vec3 color = texture2D(u_TextureMap, coord).rgb * 2.0;
	color = clamp(color, 0.0, 1.0);
	
	float combined_color = color.r + color.g + color.b;
	
	if (combined_color > 3.0) combined_color /= 4.0;
	else if (combined_color > 2.0) combined_color /= 3.0;
	else if (combined_color > 1.0) combined_color /= 2.0;
  
	return clamp(1.0 - combined_color, 0.0, 1.0);
#else

	vec3 color = texture2D(u_TextureMap, coord).rgb * 1.33333;
	color = clamp(color, 0.0, 1.0);
	
	float combined_color = color.r + color.g + color.b;
  
	if (combined_color > 3.0) combined_color /= 10.0;
	else if (combined_color > 2.0) combined_color /= 7.0;
	else if (combined_color > 1.0) combined_color /= 5.0;
  
	float height = (((0.0 - (combined_color)) * 2.0) - 1.0);
  
	//return clamp(height * 0.33333, 0.0, 1.0);
	return height * 0.33333;


/*
	vec3 color = texture2D(u_TextureMap, coord).rgb * 1.33333;
	float maximum = max(color.r, color.g);
	maximum = max(maximum, color.b);
	return (maximum * 2.0) + 1.0;
*/
#endif
}

void main (void)
{
#ifdef METHOD1
	//
	// Original Steep Parallax Method...
	//

	vec4 color;
	vec2 Depth = vec2(1.0f / viewWidth, 1.0f / viewHeight); // calculated from screen size now
	vec3 Normal;
	vec3 Tangent;
	vec3 Binormal;
    vec2 startCoord = texCoord.st;
    vec2 newCoord = startCoord;
	float is_grass = 0.0;

	 // Check for grass/foliage - we can amplify this more...
    vec4 amplifyCheck = vec4( texture2D( u_TextureMap, startCoord ).xyz, 1.0 );

	if (amplifyCheck.g * 0.9 > amplifyCheck.r && amplifyCheck.g * 0.9 > amplifyCheck.b)
	{
		is_grass = 1.0;
	}

    float height = GetHeight( texCoord.st );
 
	 vec3 LightDirectionTS = normalize( vec3(-0.0001, -0.0001, -0.0001) );
    vec3 EyeDirectionTS = vec3(var_Local1.r, var_Local1.g, var_Local1.b);

	 // Special direction for grass/foliage - straight up!
	 if (is_grass > 0)
	 {
		EyeDirectionTS = vec3(0.0001, 0.0002, 0.0004);
		EyeDirectionTS.x *= grassSway;
	 }

    vec3 LightDirection = normalize( LightDirectionTS );
    vec3 EyeDirection = normalize( EyeDirectionTS );

    vec3 EyeRay = -EyeDirection;
 
    // Common for Parallax
    vec2 ParallaxXY = ( EyeRay ).xy/-EyeRay.z * Depth * depthScale;

	 // Amplify height of grass/foliage...
	 if (is_grass > 0)
		ParallaxXY *= 20;
 
    // Steep Parallax
//    float Step = 0.01;
//    float Step = 0.05; // hmmm
    float Step = 0.03;
    vec2 dt = ParallaxXY * Step;

    float Height = 0.5;
    float oldHeight = 0.5;
    vec2 Coord = startCoord;
    vec2 oldCoord = Coord;
    float HeightMap = GetHeight( Coord );
    float oldHeightMap = HeightMap;
 
    while( HeightMap < Height )
    {
        oldHeightMap = HeightMap;
        oldHeight = Height;
        oldCoord = Coord;
 
        Height -= Step;
        Coord += dt;
        HeightMap = GetHeight( Coord );
    }

    Coord = (Coord + oldCoord) * 0.5;

    if( Height < 0.0 )
    {
        Coord = oldCoord;
        Height = 0.0;
    }
//#define PARALLAX_INTERPOLATION
#ifdef PARALLAX_INTERPOLATION
     else // interpolation
     {
		vec2 StepXYlength = Depth;
		float ds = (GetHeight( oldCoord ) - oldHeight) * StepXYlength / ( -Step - oldHeightMap + HeightMap );
		Coord = oldCoord + dt * ds;
		Height = oldHeight + -Step/StepXYlength * ds;
     }
#endif //PARALLAX_INTERPOLATION

    newCoord = Coord;
 
    vec4 colorMap = vec4( texture2D( u_TextureMap, newCoord ).xyz, 1.0 );

    float heightMap = GetHeight( newCoord );
    vec3 normalMap = GetNormal( newCoord );
 
    // Ambient
//    vec4 ambient_color = (gl_FrontLightModelProduct.sceneColor) + (colorMap * gl_LightSource[0].ambient * gl_FrontMaterial.ambient);
    vec4 ambient_color = (colorMap * colorMap * colorMap);
 
    // Diffuse
    float lambertTerm = max( dot( normalMap, LightDirection ), 0.0 );
//    vec4 diffuse_color = colorMap * gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * lambertTerm;
    vec4 diffuse_color = colorMap * colorMap * colorMap * lambertTerm;
 
    // Specular
    vec3 reflectDirection = 2.0 * dot( normalMap, LightDirection ) * normalMap - LightDirection; // in Tangen Space
 
//    float specular = pow( max( dot(reflectDirection, EyeDirectionTS), 0.1 ), gl_FrontMaterial.shininess );
    float specular = pow( max( dot(reflectDirection, EyeDirectionTS), 0.1 ), colorMap.r );
//    vec4 specular_color = gl_LightSource[0].specular * gl_FrontMaterial.specular * specular;
    vec4 specular_color = colorMap * colorMap * specular;
 
    // if( newCoord.s < 0.0 || newCoord.t < 0.0 || newCoord.s > 1.0 || newCoord.t > 1.0 )
    // {
    // discard;
    // }
 
//    gl_FragColor = vec4( ambient_color.xyz + diffuse_color.xyz + specular_color.xyz, 1.0 );
    color = vec4( ( ambient_color.xyz + diffuse_color.xyz + specular_color.xyz + (colorMap.xyz * 3.0)) * 0.33333, 1.0 );
//	 color.rgb = (color.rgb + color.rgb + GetEmboss( color.rgb, 2.0, 0.0013, texCoord ).rgb) * 0.333;
	 gl_FragColor = vec4(GetEmboss( color.rgb, 0.1, 0.0013, texCoord ).rgb, 1.0);
	 return;

	gl_FragColor = color;

#else
	//
	// No loops required!
	//

	vec2 offset = vec2(1.0f / viewWidth, 1.0f / viewHeight);
	vec2 dt = offset;
    bool skip_parallax = false;
    vec4 color;
    vec2 startCoord = texCoord.st;
    vec2 newCoord = startCoord;
    float Height = 0.0;
    float oldHeight = Height;
    vec2 Coord = startCoord;
    vec2 oldCoord = Coord;
    
    float HeightMap = GetHeight( Coord );
    float oldHeightMap = HeightMap;
    //float PARALLAX_MULTIPLIER = 1.5;
	//float PARALLAX_MULTIPLIER = 3.0;
	float PARALLAX_MULTIPLIER = 5.0;
 
    //Coord -= (dt * GetHeight( Coord ) * PARALLAX_MULTIPLIER);
	Coord += (dt * GetHeight( Coord ) * PARALLAX_MULTIPLIER);

    newCoord = Coord;
 
    vec3 colorMap = texture2D( u_TextureMap, newCoord ).rgb;
    vec3 normalMap = colorMap.rgb;
    float heightMap = GetHeight( newCoord );
 
    // Ambient
    vec3 ambient_color = (colorMap.rgb * colorMap.rgb * colorMap.rgb);
 
    // Diffuse
    //vec3 LightDirection = vec3(-0.5, -0.5, -0.5, -0.5);
    //vec3 EyeDirectionTS = vec3(0.5, 0.5, 0.5, 0.5);
	vec3 LightDirectionTS = normalize( vec3(-0.0001, -0.0001, -0.0001) );
    vec3 EyeDirectionTS = vec3(var_Local1.r, var_Local1.g, var_Local1.b);
	vec3 LightDirection = normalize( LightDirectionTS );
    vec3 EyeDirection = normalize( EyeDirectionTS );
    float lambertTerm = max(dot( normalMap, LightDirection ), 0.0);
    vec3 diffuse_color = colorMap.rgb * colorMap.rgb * colorMap.rgb * lambertTerm;
 
    // Specular
    vec3 reflectDirection = 2.0 * dot( normalMap, LightDirection ) * normalMap.rgb - LightDirection; // in Tangen Space
 
    float specular = pow( max( dot(reflectDirection, EyeDirectionTS), 0.1 ), colorMap.r );
    vec3 specular_color = colorMap.rgb * colorMap.rgb * specular;
 
	//color = vec4( ( ambient_color.rgb + diffuse_color.rgb + specular_color.rgb + (colorMap.rgb * 2.0)) * 0.33333, 1.0 );
	color = vec4( ( ambient_color.rgb + diffuse_color.rgb + specular_color.rgb + (colorMap.rgb * 2.0)) * 0.44444, 1.0 );
	
	gl_FragColor = color;
#endif
}

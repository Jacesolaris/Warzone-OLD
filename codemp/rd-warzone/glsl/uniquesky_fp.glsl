uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_ViewOrigin;

in vec3 fWorldPos;
in float playerLookingAtSun;	// the dot of the player looking at the sun - should be the same for all verts
in vec3 var_Position;
in vec3 var_Normal;
in vec3 var_SunDir;
in vec3 var_ViewDir;

// OUT
out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;
#ifdef USE_SSDO
out vec4 out_PureNormal;
#endif //USE_SSDO

float tween(float t)
{
	return clamp(t*t*t*(t*(t*6-15)+10),0,1);
}

float sunfade(float t)
{
	return clamp(t*t, 0, 1);
}

float sunglow(float t)
{
	return clamp(t*t*t*t*t, 0, 1);
}

void main()
{
	// to get here, we need to draw a sphere around the player
	// this is the frag shader for that sphere

	//calculated vars - all need to be normalized
	//vec3 SunDir = normalize(u_PrimaryLightOrigin.xyz - (fWorldPos.xyz * u_PrimaryLightOrigin.w));
	//vec3 SunDir = normalize(u_PrimaryLightOrigin.xyz - fWorldPos.xyz);
	vec3 SunDir = normalize(var_SunDir);//normalize(u_PrimaryLightOrigin.xyz - u_ViewOrigin.xyz);
	vec3 FragDir = normalize(var_ViewDir);//normalize(fWorldPos - u_ViewOrigin.xyz);
	vec3 UpDir = normalize(vec3(0.0, 1.0, 0.0)/*playerPos*/);

	float dotSU = dot(SunDir, UpDir);
	float dotSF = dot(SunDir, FragDir);
	float dotFU = dot(FragDir, UpDir);
	
	//colors
	vec3 DaySkyColor =   mix( vec3(0.25, 0.45, 0.80), vec3(0.00f, 0.25f, 0.60f), max(dotFU,0));     // Day sky color
	vec3 NightSkyColor = vec3(0.00f, 0.01f, 0.05f);     // night sky color
	//vec3 TransColor =    vec3(1.00f, 0.46f, 0.00f);     // sunrise/sunset color
	vec3 TransColor =    vec3(1.00f, 0.50f, 0.25f);     // sunrise/sunset color
	vec3 SunColor =      vec3(1.00f, 1.00f, 0.90f);		// color of the sun 
	vec3 SunGlowColor =  vec3(1.00f, 0.99f, 0.87f);		// color of the sun's glow
	vec3 HorizonGlowColor = TransColor;  // the horizon's glow at rise/set
	//vec3 HorizonGlowColor = vec3(1.00f, 0.80f, 0.40f);  // the horizon's glow at rise/set
	  
	// Things that could be done with the colors:
	// 1. Change sun color depending on sun's closeness to horizon (i.e. dotSU closeness to 0)
	// 2. Change all colors depending on elevation (aka thinner atmosphere at heights)

	vec3 tColor = vec3(0.0f, 1.0f, 0.0f);  // temporary result color

	// controls when the sunset/sunrise sky starts
	float Trans =  0.45;  // the closer to 0, the less time for those events
	float Night = -0.10;  // determines when total nighttime occurs
	float NightTransFade = 0.1; // how much before night the sun's glow starts diminishing
	float transBlendFactor  = 0.35;   // 0 < this < 1 - larger makes the sunset/rise glow closer to sky
	float horizonGlow = 0.33;       // the size of the glow of the horizon near sunrise/sunset
	float horizonSunGlow = 0.0;    // how far from the sun the horizon glow extends
	float sunHorizonFade = 0.02;    // how far up from the horizon does the sun blend
	float underHorizonFade = -0.1;  // from horizon to this line, there's a fade - below this line is solid color
	float SkyLitOffset = 0.2;		// determines how much before 'night' the sun gets brighter or stays brighter
	
	// controls how big the sun will appear in the sky
	float SunDisk = 0.999;  // closer to 1 means smaller the disk
	// sun glow is dependant on how close the player is looking at the sun
	float sunGlowScale = mix(0, playerLookingAtSun, tween(clamp( 1 - (0.4 - dotSU)/(0.4 - 0.25), 0, 1)));
	float SunGlow = mix(0.997, 0.940, sunGlowScale);  // closer to SunDisk, smaller the glow (always should be < SunDisk)
	float MoonDisk = -0.9995;  //experimental moon disk

	// controls the size of the reddish glow near the sun at sunrise/sunset
	float SunTransGlow = 0.5;  // smaller num means larger glow (should not exceed SunGlow, and actually be a lot below it)
	
	// determine sky color beforehand
	tColor = mix(NightSkyColor, DaySkyColor, clamp((dotSU - (Night - SkyLitOffset)) / (Trans - (Night - SkyLitOffset)), 0 ,1) );
	
	if (dotSU > Trans)
	{

	}
	else if (dotSU > Night + NightTransFade)
	{  // we're at sunrise/sunset
	  vec3 tBeforeSun = tColor;
	  // also, set the sky glow of the sunrise/sunset
	  if (dotSF > SunTransGlow)
	  {  // the fragment falls inside the glow side
    	// recalc transcolor based on var:
        TransColor = mix(TransColor, tColor, transBlendFactor);
		// blend smooth glow
		vec3 tTransGlowColor = mix(tColor, TransColor, sunfade((dotSF - SunTransGlow) / (1 - SunTransGlow)) );
		// and now blend depending on dotSU so the sunrise/sunset glow doesn't just 'pop' into existence
		tTransGlowColor = mix(tTransGlowColor, tColor, sunfade((dotSU - Night - NightTransFade) / (Trans - Night - NightTransFade)));
		tColor = tTransGlowColor;
	  }
	  // now if fragment is near horizon, add a glow
	  if (abs(dotFU) < horizonGlow)
	  {
	    // first make the horizon glow color less intrusive
	    HorizonGlowColor = mix(HorizonGlowColor, tBeforeSun, 0.4);
	    // then do some fun stuff with it
	    HorizonGlowColor = mix(HorizonGlowColor, tColor, (horizonSunGlow - dotSF)/(1 - horizonSunGlow));
	    HorizonGlowColor = mix(HorizonGlowColor, tColor, (abs(dotFU))/(horizonGlow));
	    HorizonGlowColor = mix(HorizonGlowColor, tColor, abs(dotFU)/(horizonGlow));
	    tColor = mix(HorizonGlowColor, tColor, (dotSU - Night - NightTransFade) / (Trans - Night - NightTransFade));
	  }
	}
	else if (dotSU > Night)
	{  // we're at sunrise/sunset
	  vec3 tBeforeSun = tColor;
	  // also, set the sky glow of the sunrise/sunset
	  if (dotSF > SunTransGlow)
	  {  // the fragment falls inside the glow side
    	// recalc transcolor based on var:
        TransColor = mix(TransColor, tColor, transBlendFactor);
		// blend smooth glow
		vec3 tTransGlowColor = mix(tColor, TransColor, tween((dotSF - SunTransGlow) / (1 - SunTransGlow)) );
		// and now blend depending on dotSU so the sunrise/sunset glow doesn't just 'pop' into existence
		tTransGlowColor = mix(tColor, tTransGlowColor, (dotSU - Night) / (NightTransFade));
		tColor = tTransGlowColor;
	  }
	  // now if fragment is near horizon, add a glow
	  if (abs(dotFU) < horizonGlow)
	  {
	    // first make the horizon glow color less intrusive
	    HorizonGlowColor = mix(HorizonGlowColor, tBeforeSun, 0.4);
	    // then do some fun stuff with it
	    HorizonGlowColor = mix(HorizonGlowColor, tColor, (horizonSunGlow - dotSF)/(1 - horizonSunGlow));
	    HorizonGlowColor = mix(HorizonGlowColor, tColor, (abs(dotFU))/(horizonGlow));
	    HorizonGlowColor = mix(HorizonGlowColor, tColor, abs(dotFU)/(horizonGlow));
	    tColor = mix(tColor, HorizonGlowColor, (dotSU - Night) / (NightTransFade));
	  }
	}
	else
	{  // it's night time
	  // TODO: lighten up near the horizon (and below) (lessening the lightening up closer to night we are)
	}

	// draw the spot of the sun on the sky, only if the fragment is above the 'horizon'
    if (dotFU > 0)
    {
        vec3 tSunColor = tColor;
        if (dotSF > SunDisk)
        {
            tSunColor = SunColor;
        }
        else if (dotSF > SunGlow)
        {
			SunGlowColor = mix(SunColor, SunGlowColor, sunglow((dotSF - SunGlow) / (SunDisk - SunGlow)) );
            tSunColor = mix(tColor, SunGlowColor, sunglow((dotSF - SunGlow) / (SunDisk - SunGlow)));
        }
        else if (dotSF < MoonDisk)
        {
            tSunColor = vec3(0.89, 0.89, 0.89);
        }
        if (dotFU < sunHorizonFade)
            tColor = mix(tColor, tSunColor, dotFU / sunHorizonFade); // performs a mix to blend the sun with the horizon
        else
            tColor = tSunColor;
    }
	else
	{// Fragment is below the horizon line 
	    vec3 darkColor = tColor/2.0;
		if (dotFU > underHorizonFade)
		    tColor = mix(darkColor, tColor, (dotFU - underHorizonFade) / abs(underHorizonFade));	// darken it a bit
		else
		    tColor = darkColor;
	}

	gl_FragColor = vec4(tColor.rgb, 1.0);
	out_Glow = vec4(0.0);
	out_Position = vec4(var_Position.xyz, 1024.0);
	out_Normal = vec4(var_Normal.xyz * 0.5 + 0.5, 0.0);
#ifdef USE_SSDO
	out_PureNormal = vec4(var_Normal.rgb * 0.5 + 0.5, 0.0);
#endif //USE_SSDO
}

varying vec2	texCoord1;
varying vec2	var_Dimensions;
varying float	time;
varying vec3	pPos;
varying vec3	viewPos;
varying vec3	viewAngles;
vec2 resolution = var_Dimensions;


#if 0


vec3 sunLight  = normalize( vec3(  0.35, 0.17,  0.3 ) );
vec3 sunColour = vec3(1.0, .5, .24);
float gTime;
float cloudy;

#define cloudLower 2000.0
#define cloudUpper 2800.0
//#define TEXTURE_NOISE


//--------------------------------------------------------------------------
float Hash( float n )
{
	return fract(sin(n)*43758.5453);
}

//--------------------------------------------------------------------------
float Noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0;
    float res = mix(mix( Hash(n+  0.0), Hash(n+  1.0),f.x),
                    mix( Hash(n+ 57.0), Hash(n+ 58.0),f.x),f.y);
    return res;
}
//--------------------------------------------------------------------------
float Hash(in vec3 p)
{
    return fract(sin(dot(p,vec3(37.1,61.7, 12.4)))*3758.5453123);
}

//--------------------------------------------------------------------------
#ifdef TEXTURE_NOISE
float Noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel0, (uv+ 0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.z );
}
#else
float Noise(in vec3 p)
{
    vec3 i = floor(p);
	vec3 f = fract(p); 
	f *= f * (3.0-2.0*f);

    return mix(
		mix(mix(Hash(i + vec3(0.,0.,0.)), Hash(i + vec3(1.,0.,0.)),f.x),
			mix(Hash(i + vec3(0.,1.,0.)), Hash(i + vec3(1.,1.,0.)),f.x),
			f.y),
		mix(mix(Hash(i + vec3(0.,0.,1.)), Hash(i + vec3(1.,0.,1.)),f.x),
			mix(Hash(i + vec3(0.,1.,1.)), Hash(i + vec3(1.,1.,1.)),f.x),
			f.y),
		f.z);
}
#endif

//--------------------------------------------------------------------------
float FBM( vec3 p )
{
	p.xz *= .5;
    float f;
    f  = 0.5000   * Noise(p); p =  p * 3.52;
    f += 0.2500   * Noise(p); p =  p * 3.53;
    f += 0.1250   * Noise(p); p =  p * 3.51;
    f += 0.0625   * Noise(p); p =  p * 3.515;
	f += 0.03125  * Noise(p); p =  p * 3.52;
	f += 0.015625 * Noise(p);
    return f;
}

//--------------------------------------------------------------------------
float SeaFBM( vec2 p )
{
    float f;
	f = sin(sin(p.x *.22) + cos(p.y *.24)+p.x*.05+p.y*.03);
    f += 0.5000 * Noise(p); p =  p * 2.12;
    f += 0.2500 * Noise(p); p =  p * 2.27;
    f += 0.1250 * Noise(p); p =  p * 2.13;
    f += 0.0625 * Noise(p); p =  p * 2.03;

	return f;
}

//--------------------------------------------------------------------------
float Map(vec3 p)
{
	p.y -= gTime *.05 - 3.5;
	float h = FBM(p);
	return h-cloudy-.45;
}

//--------------------------------------------------------------------------
float SeaMap(in vec2 pos)
{
	pos *= .008;
	return SeaFBM(pos) * 15.0;
}

//--------------------------------------------------------------------------
vec3 SeaNormal( in vec3 pos, in float d)
{
	float p = .01 * d * d / resolution.y;
	vec3 nor  	= vec3(0.0,		    SeaMap(pos.xz), 0.0);
	vec3 v2		= nor-vec3(p,		SeaMap(pos.xz+vec2(p,0.0)), 0.0);
	vec3 v3		= nor-vec3(0.0,		SeaMap(pos.xz+vec2(0.0,-p)), -p);
	nor = cross(v2, v3);
	return normalize(nor);
}

//--------------------------------------------------------------------------
// Grab all sky information for a given ray from camera
vec3 GetSky(in vec3 pos,in vec3 rd)
{
	float sunAmount = max( dot( rd, sunLight), 0.0 );
	// Do the blue and sun...	
	vec3  sky = vec3(.2, .5, .75);
	sky = sky + sunColour * min(pow(sunAmount, 1000.0) * 2.0, 1.0);
	sky = sky + sunColour * min(pow(sunAmount, 10.0) * .85, 1.0);
	
	// Find the start and end of the cloud layer...
	float beg = ((cloudLower-pos.y)/rd.y);
	float end = ((cloudUpper-pos.y)/rd.y);
	// Start position...
	vec3 p = vec3(pos.x + rd.x * beg, cloudLower, pos.z + rd.z * beg);

	// Trace clouds through that layer...
	float d = 0.0;
	float add = (end-beg) / 40.0;
	vec4 sum = vec4(0.0);
	// Horizon fog is just thicker clouds...
	vec4 col = vec4(0, 0, 0, pow(1.0-rd.y,8.) * .1);
	for (int i = 0; i < 40; i++)
	{
		if (col.a >= 1.0) continue;
		vec3 pos = p + rd * d;
		float h = Map(pos * .001);
		col.a += max(-h, 0.0) * .09; 
		col.rgb = mix(vec3((pos.y-cloudLower)/((cloudUpper-cloudLower) * .8)) * col.a, sunColour, max(.5-col.a, 0.0) * .05);
		sum = sum + col*(1.0 - sum.a);
		d += add;
	}
	sum.xyz += min((1.-sum.a) * pow(sunAmount, 3.0), 1.0);
	sky = mix(sky, sum.xyz, sum.a);

	return clamp(sky, 0.0, 1.0);
}

//--------------------------------------------------------------------------
vec3 GetSea(in vec3 pos,in vec3 rd)
{
	vec3 sea;
	float d = -pos.y/rd.y;
	vec3 p = vec3(pos.x + rd.x * d, 0.0, pos.z + rd.z * d);
	
	float dis = length(p-pos);
	vec3 nor = SeaNormal(p, dis);

	vec3 ref = reflect(rd, nor);
	sea = GetSky(p, ref);
	
	sea = mix(sea*.6, vec3(.15, .3, .4), .2);
	
	float glit = max(dot(ref, sunLight), 0.0);
	sea += sunColour * pow(glit, 220.0) * max(-cloudy*100.0, 0.0);
	
	return sea;
}

//--------------------------------------------------------------------------
vec3 CameraPath( float t )
{
	//t = time + t;
    vec2 p = vec2(4000.0 * sin(.16*t), 4000.0 * cos(.155*t) );
	return vec3(p.x+5.0,  0.0, -94.0+p.y);
} 

//--------------------------------------------------------------------------
void main(void)
{
#if 0
	gTime = time*.65 + 70. + 0.07*length(texCoord1)/length(resolution);
	cloudy = cos(gTime * .27+.15) * .2;
	
    vec2 xy = texCoord1.xy;// / resolution.xy;
	vec2 uv = (-1.0 + 2.0 * xy) * vec2(resolution.x/resolution.y,1.0);
	
	//vec3 cameraPos	   = vec3(1.0, 1.0, 1.0);
	//vec3 cameraPos = CameraPath(gTime);
	vec3 cameraPos = viewAngles.zxy;
	//vec3 camTar	   = CameraPath(gTime + 1.0);
	vec3 camTar	   = CameraPath(gTime + 1.0);
	//camTar.z = viewAngles.z;
	//vec3 camTar	   = vec3(0.0, 0.0, 0.0);
	//camTar.y = cameraPos.y = sin(gTime) * 200.0 + 300.0;
	camTar.y = sin(gTime) * 200.0 + 300.0;
	//camTar.x = cameraPos.x = sin(gTime) * 200.0 + 300.0;
	//camTar.z = cameraPos.z = sin(gTime) * 200.0 + 300.0;
	//camTar.y = cameraPos.y = sin(viewAngles.y*gTime) * 200.0 + 300.0;
	camTar.y += 300.0;
	
	//float roll = .1 * sin(gTime * .25);
	float roll = 0.0;
	//vec3 cw = normalize(camTar-cameraPos);
	vec3 cw = normalize(viewAngles);
	vec3 cp = vec3(sin(roll), cos(roll),0.0);
	vec3 cu = cross(cw,cp);
	vec3 cv = cross(cu,cw);
	vec3 dir = normalize(uv.x*cu + uv.y*cv + 1.3*cw);
	mat3 camMat = mat3(cu, cv, cw);

	vec3 col;
	float distance = 1e20;
	float type = 0.0;

	//if (dir.y > 0.0)
	//{
		col = GetSky(cameraPos, dir);
	//}else
	//{
	//	col = GetSea(cameraPos, dir);
	//}

	// Don't gamma too much to keep the moody look...
	col = pow(col, vec3(.7));
	gl_FragColor=vec4(col, 1.0);
#else
	gTime = time*.65 + 70. + 0.07*length(texCoord1)/length(resolution);
	cloudy = cos(gTime * .27+.15) * .2;

	vec3 col = GetSky(viewPos, viewAngles);

	// Don't gamma too much to keep the moody look...
	col = pow(col, vec3(.7));
	gl_FragColor=vec4(col, 1.0);
#endif
}

#else


// Clouds: slice based volumetric height-clouds with god-rays, density, sun-radiance/shadow
// and 
// Water: simple reflecting sky/sun and cloud shaded height-modulated waves
//
// Created by Frank Hugenroth 03/2013
//
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
// noise and raymarching based on concepts and code from shaders by inigo quilez
//

// some variables to change :)

float iGlobalTime = time;
vec2 iResolution = resolution;
vec2 fragCoord = texCoord1 * resolution;

//#define RENDER_GODRAYS    1    // set this to 1 to enable god-rays
//#define RENDER_GODRAYS    0    // disable god-rays

#define RENDER_CLOUDS 1
//#define RENDER_WATER   0

float waterlevel = 70.0;        // height of the water
float wavegain   = 1.0;       // change to adjust the general water wave level
float large_waveheight = 1.0; // change to adjust the "heavy" waves (set to 0.0 to have a very still ocean :)
float small_waveheight = 1.0; // change to adjust the small waves

vec3 fogcolor    = vec3( 0.5, 0.7, 1.1 );              
vec3 skybottom   = vec3( 0.6, 0.8, 1.2 );
vec3 skytop      = vec3(0.05, 0.2, 0.5);
vec3 reflskycolor= vec3(0.025, 0.10, 0.20);
vec3 watercolor  = vec3(0.2, 0.25, 0.3);

vec3 light       = normalize( vec3(  0.1, 0.25,  0.9 ) );









// random/hash function              
float hash( float n )
{
  return fract(cos(n)*41415.92653);
}

// 2d noise function
float noise(vec2 p)
{
  //return texture2D(iChannel0,p*vec2(1./256.)).x;
  return 0.0;
}


// 3d noise function
float noise( in vec3 x )
{
  vec3 p  = floor(x);
  vec3 f  = smoothstep(0.0, 1.0, fract(x));
  float n = p.x + p.y*57.0 + 113.0*p.z;

  return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
    mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}


mat3 m = mat3( 0.00,  1.60,  1.20, -1.60,  0.72, -0.96, -1.20, -0.96,  1.28 );

// Fractional Brownian motion
float fbm( vec3 p )
{
  float f = 0.5000*noise( p ); p = m*p*1.1;
  f += 0.2500*noise( p ); p = m*p*1.2;
  f += 0.1666*noise( p ); p = m*p;
  f += 0.0834*noise( p );
  return f;
}

mat2 m2 = mat2(1.6,-1.2,1.2,1.6);

// Fractional Brownian motion
float fbm( vec2 p )
{
  float f = 0.5000*noise( p ); p = m2*p;
  f += 0.2500*noise( p ); p = m2*p;
  f += 0.1666*noise( p ); p = m2*p;
  f += 0.0834*noise( p );
  return f;
}


// this calculates the water as a height of a given position
float water( vec2 p )
{
  float height = waterlevel;

  vec2 shift1 = 0.001*vec2( iGlobalTime*160.0*2.0, iGlobalTime*120.0*2.0 );
  vec2 shift2 = 0.001*vec2( iGlobalTime*190.0*2.0, -iGlobalTime*130.0*2.0 );

  // coarse crossing 'ocean' waves...
  float wave = 0.0;
  wave += sin(p.x*0.021  + shift2.x)*4.5;
  wave += sin(p.x*0.0172+p.y*0.010 + shift2.x*1.121)*4.0;
  wave -= sin(p.x*0.00104+p.y*0.005 + shift2.x*0.121)*4.0;
  // ...added by some smaller faster waves...
  wave += sin(p.x*0.02221+p.y*0.01233+shift2.x*3.437)*5.0;
  wave += sin(p.x*0.03112+p.y*0.01122+shift2.x*4.269)*2.5 ;
  wave *= large_waveheight;
  wave -= fbm(p*0.004-shift2*.5)*small_waveheight*24.;
  // ...added by some distored random waves (which makes the water looks like water :)

  float amp = 6.*small_waveheight;
  shift1 *= .3;
  for (int i=0; i<7; i++)
  {
    wave -= abs(sin((noise(p*0.01+shift1)-.5)*3.14))*amp;
    amp *= .51;
    shift1 *= 1.841;
    p *= m2*0.9331;
  }
  
  height += wave;
  return height;
}


// cloud intersection raycasting
float trace_fog(in vec3 rStart, in vec3 rDirection )
{
#if RENDER_CLOUDS
  // makes the clouds moving...
  vec2 shift = vec2( iGlobalTime*80.0, iGlobalTime*60.0 );
  float sum = 0.0;
  // use only 12 cloud-layers ;)
  // this improves performance but results in "god-rays shining through clouds" effect (sometimes)...
  float q2 = 0., q3 = 0.;
  for (int q=0; q<10; q++)
  {
    float c = (q2+350.0-rStart.y) / rDirection.y;// cloud distance
    vec3 cpos = rStart + c*rDirection + vec3(831.0, 321.0+q3-shift.x*0.2, 1330.0+shift.y*3.0); // cloud position
    float alpha = smoothstep(0.5, 1.0, fbm( cpos*0.0015 )); // cloud density
	sum += (1.0-sum)*alpha; // alpha saturation
    if (sum>0.98)
        break;
    q2 += 120.;
    q3 += 0.15;
  }
  
  return clamp( 1.0-sum, 0.0, 1.0 );
#else
  return 1.0;
#endif
}

// fog and water intersection function.
// 1st: collects fog intensity while traveling
// 2nd: check if hits the water surface and returns the distance
bool trace(in vec3 rStart, in vec3 rDirection, in float sundot, out float fog, out float dist)
{
  float h = 20.0;
  float t = 0.0;
  float st = 1.0;
  float alpha = 0.1;
  float asum = 0.0;
  vec3 p = rStart;
	
  for( int j=1000; j<1120; j++ )
  {
    // some speed-up if all is far away...
    if( t>500.0 ) 
      st = 2.0;
    else if( t>800.0 ) 
      st = 5.0;
    else if( t>1000.0 ) 
      st = 12.0;

    p = rStart + t*rDirection; // calc current ray position

#if RENDER_GODRAYS
    if (rDirection.y>0. && sundot > 0.001 && t>400.0 && t < 2500.0)
    {
      alpha = sundot * clamp((p.y-waterlevel)/waterlevel, 0.0, 1.0) * st * 0.024*smoothstep(0.80, 1.0, trace_fog(p,light));
      asum  += (1.0-asum)*alpha;
      if (asum > 0.9)
        break;
    }
#endif

    h = p.y - water(p.xz);

    if( h<0.1 ) // hit the water?
    {
      dist = t; 
      fog = asum;
      return true;
    }

    if( p.y>450.0 ) // lost in space? quit...
      break;
    
    // speed up ray if possible...    
    if(rDirection.y > 0.0) // look up (sky!) -> make large steps
      t += 30.0 * st;
    else
      t += max(1.0,1.0*h)*st;
  }

  dist = t; 
  fog = asum;
  if (h<10.0)
   return true;
  return false;
}


vec3 camera( float time )
{
  return vec3( 500.0 * sin(1.5+1.57*time), 0.0, 1200.0*time );
}


void main()
{
  vec2 xy = -1.0 + 2.0*fragCoord.xy / iResolution.xy;
  vec2 s = xy*vec2(1.75,1.0);

  // get camera position and view direction
  float time = (iGlobalTime+13.5+44.)*.05;
  vec3 campos = normalize(vec3(viewPos.x, viewPos.z, viewPos.y));// l/r, up/d, up/d
  vec3 camtar = vec3(0.0, 100.0, 100.0); //viewAngles; l/r, up/d, up/d
  campos.y = max(waterlevel+30.0, waterlevel+90.0 + 60.0*sin(time*2.0));
  //camtar.y = campos.y*0.5;

  float roll = 0.0;//0.14*sin(time*1.2);
  vec3 cw = normalize(vec3(0.0, 0.0, 0.0));//normalize(viewAngles.xyz/*camtar-campos*/); // l/r, u/d
  vec3 cp = vec3(sin(roll), cos(roll),0.0);
  vec3 cu = normalize(cross(cw,cp));
  vec3 cv = normalize(cross(cu,cw));
  vec3 rd = normalize( s.x*cu + s.y*cv + 1.6*cw );

  float sundot = clamp(dot(rd,light),0.0,1.0);

  vec3 col;
  float fog=0.0, dist=0.0;

  if (!trace(campos,rd,sundot, fog, dist))
  {
    // render sky
    float t = pow(1.0-0.7*rd.y, 15.0);
    col = 0.8*(skybottom*t + skytop*(1.0-t));
    // sun
    col += 0.47*vec3(1.6,1.4,1.0)*pow( sundot, 350.0 );
    // sun haze
    col += 0.4*vec3(0.8,0.9,1.0)*pow( sundot, 2.0 );

#if RENDER_CLOUDS
    // CLOUDS
    vec2 shift = vec2( iGlobalTime*80.0, iGlobalTime*60.0 );
    vec4 sum = vec4(0,0,0,0); 
    for (int q=1000; q<1100; q++) // 100 layers
    {
      float c = (float(q-1000)*12.0+350.0-campos.y) / rd.y; // cloud height
      vec3 cpos = campos + c*rd + vec3(831.0, 321.0+float(q-1000)*.15-shift.x*0.2, 1330.0+shift.y*3.0); // cloud position
      float alpha = smoothstep(0.5, 1.0, fbm( cpos*0.0015 ))*.9; // fractal cloud density
      vec3 localcolor = mix(vec3( 1.1, 1.05, 1.0 ), 0.7*vec3( 0.4,0.4,0.3 ), alpha); // density color white->gray
      alpha = (1.0-sum.w)*alpha; // alpha/density saturation (the more a cloud layer's density, the more the higher layers will be hidden)
      sum += vec4(localcolor*alpha, alpha); // sum up weightened color
      
      if (sum.w>0.98)
        break;
    }
    float alpha = smoothstep(0.7, 1.0, sum.w);
    sum.rgb /= sum.w+0.0001;

    // This is an important stuff to darken dense-cloud parts when in front (or near)
    // of the sun (simulates cloud-self shadow)
    sum.rgb -= 0.6*vec3(0.8, 0.75, 0.7)*pow(sundot,13.0)*alpha;
    // This brightens up the low-density parts (edges) of the clouds (simulates light scattering in fog)
    sum.rgb += 0.2*vec3(1.3, 1.2, 1.0)* pow(sundot,5.0)*(1.0-alpha);

    col = mix( col, sum.rgb , sum.w*(1.0-t) );
#endif

    // add god-rays
    col += vec3(0.5, 0.4, 0.3)*fog;
  }
  else
  {
#if RENDER_WATER        
    //  render water
    
    vec3 wpos = campos + dist*rd; // calculate position where ray meets water

    // calculate water-mirror
    vec2 xdiff = vec2(0.1, 0.0)*wavegain*4.;
    vec2 ydiff = vec2(0.0, 0.1)*wavegain*4.;

    // get the reflected ray direction
    rd = reflect(rd, normalize(vec3(water(wpos.xz-xdiff) - water(wpos.xz+xdiff), 1.0, water(wpos.xz-ydiff) - water(wpos.xz+ydiff))));  
    float refl = 1.0-clamp(dot(rd,vec3(0.0, 1.0, 0.0)),0.0,1.0);
  
    float sh = smoothstep(0.2, 1.0, trace_fog(wpos+20.0*rd,rd))*.7+.3;
    // water reflects more the lower the reflecting angle is...
    float wsky   = refl*sh;     // reflecting (sky-color) amount
    float wwater = (1.0-refl)*sh; // water-color amount

    float sundot = clamp(dot(rd,light),0.0,1.0);

    // watercolor

    col = wsky*reflskycolor; // reflecting sky-color 
    col += wwater*watercolor;
    col += vec3(.003, .005, .005) * (wpos.y-waterlevel+30.);

    // Sun
    float wsunrefl = wsky*(0.5*pow( sundot, 10.0 )+0.25*pow( sundot, 3.5)+.75*pow( sundot, 300.0));
    col += vec3(1.5,1.3,1.0)*wsunrefl; // sun reflection

#endif

    // global depth-fog
    float fo = 1.0-exp(-pow(0.0003*dist, 1.5));
    vec3 fco = fogcolor + 0.6*vec3(0.6,0.5,0.4)*pow( sundot, 4.0 );
    col = mix( col, fco, fo );

    // add god-rays
    col += vec3(0.5, 0.4, 0.3)*fog; 
  }

  gl_FragColor=vec4(col,1.0);
}


#endif

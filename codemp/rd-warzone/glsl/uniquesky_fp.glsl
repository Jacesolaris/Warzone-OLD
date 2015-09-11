
//#version 120
 

varying vec3 rayleigh;
varying vec3 mie;
varying vec3 eye;
varying vec3 hazeColor;
varying float ct;
varying float cphi;
varying float delta_z;
varying float alt;
varying float earthShade;

/*
uniform float overcast;
uniform float saturation;
uniform float visibility;
uniform float avisibility;
uniform float scattering;
uniform float terminator;
uniform float cloud_self_shading;
uniform float horizon_roughness;
*/

const float terminator = 100000.0;
const float avisibility = 100000.0;
const float visibility = 100000.0;

const float overcast = 0.3;
const float saturation = 0.15;
const float scattering = 0.15;
const float cloud_self_shading = 0.5;
const float horizon_roughness = 0.15;

const float EarthRadius = 5800000.0;

float hash( vec2 p ) {
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}
float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

float Noise2D(in vec2 coord, in float wavelength)
{
	return noise(coord);
}

float fog_backscatter(in float avisibility)
{
	//return 0.5;
	return fract(sin(avisibility)*43758.5453123);
}

float light_func (in float x, in float a, in float b, in float c, in float d, in float e)
{
x = x - 0.5;

// use the asymptotics to shorten computations
if (x > 30.0) {return e;}
if (x < -15.0) {return 0.0;}

return e / pow((1.0 + a * exp(-b * (x-c)) ),(1.0/d));
}

float miePhase(in float cosTheta, in float g)
{
  float g2 = g*g;
  float a = 1.5 * (1.0 - g2);
  float b = (2.0 + g2);
  float c = 1.0 + cosTheta*cosTheta;
  float d = pow(1.0 + g2 - 2.0 * g * cosTheta, 0.6667);
 
  return (a*c) / (b*d);
}
 
float rayleighPhase(in float cosTheta)
{
  //return 1.5 * (1.0 + cosTheta*cosTheta);
  return 1.5 * (2.0 + 0.5*cosTheta*cosTheta);
}
 

void main()
{

  //vec3 shadedFogColor = vec3(0.65, 0.67, 0.78);
   vec3 shadedFogColor = vec3(0.55, 0.67, 0.88);
  float cosTheta = dot(normalize(eye), gl_LightSource[0].position.xyz);
 
  // position of the horizon line

  float lAltitude = alt + delta_z;
  float radiusEye = EarthRadius + alt;
  float radiusLayer = EarthRadius + lAltitude;
  float cthorizon;
  float ctterrain;

  if (radiusEye > radiusLayer) cthorizon = -sqrt(radiusEye * radiusEye - radiusLayer * radiusLayer)/radiusEye;
  else cthorizon = sqrt(radiusLayer * radiusLayer - radiusEye * radiusEye)/radiusLayer;

  ctterrain = -sqrt(radiusEye * radiusEye - EarthRadius * EarthRadius)/radiusEye;

  vec3 color = rayleigh * rayleighPhase(cosTheta);
  color += mie * miePhase(cosTheta, -0.8);

  vec3 black = vec3(0.0,0.0,0.0);

  
  float ovc = overcast;



  float sat = 1.0 - ((1.0 - saturation) * 2.0);
  if (sat < 0.3) sat = 0.3;


  

if (color.r > 0.58) color.r = 1.0 - exp(-1.5 * color.r);
if (color.g > 0.58) color.g = 1.0 - exp(-1.5 * color.g);
if (color.b > 0.58) color.b = 1.0 - exp(-1.5 * color.b);
  


// fog computations for a ground haze layer, extending from zero to lAltitude



float transmission;
float vAltitude;
float delta_zv;

float costheta = ct;

float vis = min(visibility, avisibility);


 if (delta_z > 0.0) // we're inside the layer
	{
  	if (costheta>0.0 + ctterrain) // looking up, view ray intersecting upper layer edge
		{
		transmission  = exp(-min((delta_z/max(costheta,0.1)),25000.0)/vis);
		//transmission = 1.0;
		vAltitude = min(vis * costheta, delta_z);
  		delta_zv = delta_z - vAltitude;
		}

	else // looking down, view range intersecting terrain (which may not be drawn)
		{
		transmission = exp(alt/vis/costheta);
		vAltitude = min(-vis * costheta, alt);
  		delta_zv = delta_z + vAltitude;
		}
	}
  else // we see the layer from above
	{	
	if (costheta < 0.0 + cthorizon) 
		{
		transmission = exp(-min(lAltitude/abs(costheta),25000.0)/vis);
		transmission = transmission * exp(-alt/avisibility/abs(costheta));
		transmission = 1.0 - (1.0 - transmission) * smoothstep(0+cthorizon, -0.02+cthorizon, costheta);
   		vAltitude = min(lAltitude, -vis * costheta);
		delta_zv = vAltitude; 
		}
	else
		{	
		transmission = 1.0;
		delta_zv = 0.0;
		}
	}

// combined intensity reduction by cloud shading and fog self-shading, corrected for Weber-Fechner perception law
float eqColorFactor = 1.0 - 0.1 * delta_zv/vis - (1.0 - min(scattering,cloud_self_shading));


// there's always residual intensity, we should never be driven to zero
if (eqColorFactor < 0.2) eqColorFactor = 0.2;


// postprocessing of haze color
vec3 hColor = hazeColor;


// high altitude desaturation
float intensity = length(hColor);
hColor = intensity * normalize (mix(hColor, intensity * vec3 (1.0,1.0,1.0), 0.7 * smoothstep(5000.0, 50000.0, alt)));

hColor = clamp(hColor,0.0,1.0);

// blue hue
hColor.x = 0.83 * hColor.x;
hColor.y = 0.9 * hColor.y;



// further blueshift when in shadow, either cloud shadow, or self-shadow or Earth shadow, dependent on indirect 
// light

float fade_out = max(0.65 - 0.3 *overcast, 0.45);
intensity = length(hColor);
vec3 oColor = hColor;
oColor = intensity * normalize(mix(oColor,  shadedFogColor, (smoothstep(0.1,1.0,ovc)))); 
oColor = clamp(oColor,0.0,1.0);
color = ovc *  mix(color, oColor * earthShade ,smoothstep(-0.1+ctterrain, 0.0+ctterrain, ct)) + (1.0-ovc) * color; 


hColor = intensity * normalize(mix(hColor,  1.5 * shadedFogColor, 1.0 -smoothstep(0.25, fade_out,earthShade) ));
hColor = intensity * normalize(mix(hColor,  shadedFogColor, (1.0 - smoothstep(0.5,0.9,eqColorFactor)))); 
hColor = hColor * earthShade;

// accounting for overcast and saturation 



color = sat * color + (1.0 - sat) * mix(color, black, smoothstep(0.4+cthorizon,0.2+cthorizon,ct));


// the terrain below the horizon gets drawn in one optical thickness
vec3 terrainHazeColor = eqColorFactor * hColor;	

// determine a visibility-dependent angle for how smoothly the haze blends over the skydome

float hazeBlendAngle = max(0.01,1000.0/avisibility + 0.3 * (1.0 - smoothstep(5000.0, 30000.0, avisibility)));
float altFactor = smoothstep(-300.0, 0.0, delta_z);
float altFactor2 =  0.2 + 0.8 * smoothstep(-3000.0, 0.0, delta_z);
hazeBlendAngle = hazeBlendAngle + 0.1 * altFactor;
hazeBlendAngle = hazeBlendAngle +  (1.0-horizon_roughness) * altFactor2 * 0.1 *  Noise2D(vec2(0.0,cphi), 0.3);

terrainHazeColor = clamp(terrainHazeColor,0.0,1.0);


// don't let the light fade out too rapidly
float lightArg = (terminator + 200000.0)/100000.0;
float minLightIntensity = min(0.2,0.16 * lightArg + 0.5);
vec3 minLight = minLightIntensity * vec3 (0.2, 0.3, 0.4);

// this is for the bare Rayleigh and Mie sky, highly altitude dependent
color.rgb = max(color.rgb, minLight.rgb * (1.0- min(alt/100000.0,1.0)) * (1.0 - costheta));

// this is for the terrain drawn
terrainHazeColor = max(terrainHazeColor.rgb, minLight.rgb);

color = mix(color, terrainHazeColor ,smoothstep(hazeBlendAngle + ctterrain, 0.0+ctterrain, ct));


// add the brightening of fog by lights

    vec3 secondary_light = vec3 (0.0,0.0,0.0);


// mix fog the skydome with the right amount of haze

hColor *= eqColorFactor;
hColor = max(hColor.rgb, minLight.rgb);

hColor = clamp(hColor,0.0,1.0);

color = mix(hColor+secondary_light * fog_backscatter(avisibility),color, transmission);



  gl_FragColor = vec4(color, 1.0);
  gl_FragDepth = 0.1;
}


#if 0

varying vec2	texCoord1;
varying vec2	var_Dimensions;
varying float	time;
varying vec3	pPos;
varying vec3	viewPos;
varying vec3	viewAngles;
vec2 resolution = var_Dimensions;


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

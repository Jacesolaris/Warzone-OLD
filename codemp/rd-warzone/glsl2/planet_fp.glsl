uniform sampler2D u_DiffuseMap;
uniform sampler2D u_SpecularMap;

uniform vec2	u_Dimensions;
uniform vec4	u_Local1; // parallaxScale, haveSpecular, specularScale, materialType
uniform vec4	u_Local2; // ExtinctionCoefficient
uniform vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower, cubemapScale
uniform vec4	u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4	u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4	u_Local6; // useSunLightSpecular, 0, 0, shadowPass
uniform vec4	u_Local9;

uniform float	u_Time;

varying vec3	var_Normal;
varying vec3	var_vertPos;
varying vec2    var_TexCoords;


out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;
out vec4 out_NormalDetail;

#define m_Normal		var_Normal
#define m_TexCoords		var_TexCoords
#define m_vertPos		var_vertPos
#define m_ViewDir		var_ViewDir


#if 0
float snoise(vec3 uv, float res)
{
	const vec3 s = vec3(1e0, 1e2, 1e4);
	
	uv *= res;
	
	vec3 uv0 = floor(mod(uv, res))*s;
	vec3 uv1 = floor(mod(uv+vec3(1.), res))*s;
	
	vec3 f = fract(uv); f = f*f*(3.0-2.0*f);
	
	vec4 v = vec4(uv0.x+uv0.y+uv0.z, uv1.x+uv0.y+uv0.z,
		      	  uv0.x+uv1.y+uv0.z, uv1.x+uv1.y+uv0.z);
	
	vec4 r = fract(sin(v*1e-3)*1e5);
	float r0 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	r = fract(sin((v + uv1.z - uv0.z)*1e-3)*1e5);
	float r1 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	return mix(r0, r1, f.z)*2.-1.;
}

float freqs[4];

void planet( out vec4 fragColor, in vec2 fragCoord )
{
	freqs[0] = 1.0;//texture2D( iChannel1, vec2( 0.01, 0.25 ) ).x;
	freqs[1] = 1.0;//texture2D( iChannel1, vec2( 0.07, 0.25 ) ).x;
	freqs[2] = 1.0;//texture2D( iChannel1, vec2( 0.15, 0.25 ) ).x;
	freqs[3] = 1.0;//texture2D( iChannel1, vec2( 0.30, 0.25 ) ).x;

	float brightness	= freqs[1] * 0.25 + freqs[2] * 0.25;
	float radius		= 0.24 + brightness * 0.2;
	float invRadius 	= 1.0/radius;
	
	vec3 orange			= vec3( 0.2, 0.2, 0.2 );
	vec3 orangeRed		= vec3( 0.2, 0.2, 0.2 );
	float time		= u_Time * 0.1;
	float aspect	= u_Dimensions.x/u_Dimensions.y;
	vec2 uv			= fragCoord.xy / u_Dimensions.xy;
	vec2 p 			= -0.5 + uv;
	p.x *= aspect;

	float fade		= pow( length( 3.0 * p ), 0.5 );
	float fVal1		= 1.1 - fade;
	float fVal2		= 1.1 - fade;
	
	float angle		= atan( p.x, p.y )/6.2832;
	float dist		= length(p);
	vec3 coord		= vec3( angle, dist, time * 0.1 );
	
	float newTime1	= abs( snoise( coord + vec3( 0.0, -time * ( 0.35 + brightness * 0.001 ), time * 0.015 ), 15.0 ) );
	float newTime2	= abs( snoise( coord + vec3( 0.0, -time * ( 0.15 + brightness * 0.001 ), time * 0.015 ), 45.0 ) );	
	for( int i=1; i<=7; i++ ){
		float power = pow( 2.0, float(i + 1) );
		fVal1 += ( 0.1 / power ) * snoise( coord + vec3( 0.0, -time, time * 0.2 ), ( power * ( 10.0 ) * ( newTime1 + 1.0 ) ) );
		fVal2 += ( 0.1 / power ) * snoise( coord + vec3( 0.0, -time, time * 0.2 ), ( power * ( 25.0 ) * ( newTime2 + 1.0 ) ) );
	}
	
	float corona		= pow( fVal1 * max( 1.1 - fade, 0.0 ), 2.0 ) * 50.0;
	corona				+= pow( fVal2 * max( 1.1 - fade, 0.0 ), 2.0 ) * 50.0;
	corona				*= 1.2 - newTime1;
	vec3 sphereNormal 	= vec3( 0.0, 0.0, 1.0 );
	vec3 dir 			= vec3( 0.0 );
	vec3 center			= vec3( 0.5, 0.5, 1.0 );
	vec3 starSphere		= vec3( 0.0 );
	
	vec2 sp = -1.0 + 2.0 * uv;
	sp.x *= aspect;
	sp *= ( 2.0 - brightness );
  	float r = dot(sp,sp);
	float f = (1.0-sqrt(abs(1.0-r)))/(r) + brightness * 0.5;
	if( dist < radius ){
		corona			*= pow( dist * invRadius, 24.0 );
  		vec2 newUv;
 		newUv.x = sp.x*f;
  		newUv.y = sp.y*f;
		newUv += vec2( time, 0.0 );
		
		vec3 texSample 	= texture2D( u_DiffuseMap, newUv ).rgb;
		float uOff		= ( texSample.g * brightness * 4.5 + time );
		vec2 starUV		= newUv + vec2( uOff, 0.0 );
		starSphere		= texture2D( u_DiffuseMap, starUV ).rgb;
	}
	
	float starGlow	= min( max( 1.0 - dist * ( 1.0 - brightness ), 0.0 ), 1.0 );
	fragColor.rgb	= vec3( f * ( 0.75 + brightness * 0.3 ) * orange ) + starSphere + corona * orange + starGlow * orangeRed;
	fragColor.a		= 1.0;//starGlow * orangeRed.x;
}
#elif 0
// rendering params
const float sphsize=.7; // planet size
const float dist=.27; // distance for glow and distortion
const float perturb=.3; // distortion amount of the flow around the planet
const float displacement=.015; // hot air effect
const float windspeed=.4; // speed of wind flow
const float steps=110.; // number of steps for the volumetric rendering
const float stepsize=.025; 
const float brightness=.43;
const vec3 planetcolor=vec3(0.55,0.4,0.3);
const float fade=.005; //fade by distance
const float glow=3.5; // glow amount, mainly on hit side


// fractal params
const int iterations=13; 
const float fractparam=.7;
const vec3 offset=vec3(1.5,2.,-1.5);


float wind(vec3 p) {
	float d=max(0.,dist-max(0.,length(p)-sphsize)/sphsize)/dist; // for distortion and glow area
	float x=max(0.2,p.x*2.); // to increase glow on left side
	p.y*=1.+max(0.,-p.x-sphsize*.25)*1.5; // left side distortion (cheesy)
	p-=d*normalize(p)*perturb; // spheric distortion of flow
	p+=vec3(u_Time*windspeed,0.,0.); // flow movement
	p=abs(fract((p+offset)*.1)-.5); // tile folding 
	for (int i=0; i<iterations; i++) {  
		p=abs(p)/dot(p,p)-fractparam; // the magic formula for the hot flow
	}
	return length(p)*(1.+d*glow*x)+d*glow*x; // return the result with glow applied
}

void planet( out vec4 fragColor, in vec2 fragCoord )
{
	// get ray dir	
	vec2 uv = fragCoord.xy / u_Dimensions.xy-.5;
	vec3 dir=vec3(uv,1.);
	dir.x*=u_Dimensions.x/u_Dimensions.y;
	vec3 from=vec3(0.,0.,-2.+texture(u_SpecularMap,uv*.5+u_Time).x*stepsize); //from+dither

	// volumetric rendering
	float v=0., l=-0.0001, t=u_Time*windspeed*.2;
	for (float r=10.;r<steps;r++) {
		vec3 p=from+r*dir*stepsize;
		float tx=0.0;//texture(u_SpecularMap,uv*.2+vec2(t,0.)).x*displacement; // hot air effect
		if (length(p)-sphsize-tx>0.)
		// outside planet, accumulate values as ray goes, applying distance fading
			v+=0.;//min(50.,wind(p))*max(0.,1.-r*fade); 
		else if (l<0.) 
		//inside planet, get planet shading if not already 
		//loop continues because of previous problems with breaks and not always optimizes much
			l=pow(max(.53,dot(normalize(p),normalize(vec3(-1.,.5,-0.3)))),4.)
			*(.5+texture(u_DiffuseMap,uv*vec2(2.,1.)*(1.+p.z*.5)+vec2(tx+t*.5,0.)).x*2.);
		}
	v/=steps; v*=brightness; // average values and apply bright factor
	vec3 col=vec3(v*1.25,v*v,v*v*v)+l*planetcolor; // set color
	col*=1.-length(pow(abs(uv),vec2(5.)))*14.; // vignette (kind of)
	fragColor = vec4(col,1.0);
}
#else
// all noise from iq!

#define time u_Time

// INNER ATMOS PROPIETIES:
    // inner atmos inner strenght
    #define in_inner 0.2
    // inner atmos outer strenght
    #define in_outer 0.2

// OUTER ATMOS PROPIETIES:
    // inner atmos inner strenght
    #define out_inner 0.2 
    // inner atmos outer strenght
    #define out_outer 0.4 // 0.01 is nice too

float noise3D(vec3 p)
{
	return fract(sin(dot(p ,vec3(12.9898,78.233,128.852))) * 43758.5453)*2.0-1.0;
}

float simplex3D(vec3 p)
{
	float f3 = 1.0/3.0;
	float s = (p.x+p.y+p.z)*f3;
	int i = int(floor(p.x+s));
	int j = int(floor(p.y+s));
	int k = int(floor(p.z+s));
	
	float g3 = 1.0/6.0;
	float t = float((i+j+k))*g3;
	float x0 = float(i)-t;
	float y0 = float(j)-t;
	float z0 = float(k)-t;
	x0 = p.x-x0;
	y0 = p.y-y0;
	z0 = p.z-z0;
	int i1,j1,k1;
	int i2,j2,k2;
	if(x0>=y0)
	{
		if		(y0>=z0){ i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
		else if	(x0>=z0){ i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
		else 			{ i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Z order
	}
	else 
	{ 
		if		(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
		else if	(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
		else 			{ i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
	}
	float x1 = x0 - float(i1) + g3; 
	float y1 = y0 - float(j1) + g3;
	float z1 = z0 - float(k1) + g3;
	float x2 = x0 - float(i2) + 2.0*g3; 
	float y2 = y0 - float(j2) + 2.0*g3;
	float z2 = z0 - float(k2) + 2.0*g3;
	float x3 = x0 - 1.0 + 3.0*g3; 
	float y3 = y0 - 1.0 + 3.0*g3;
	float z3 = z0 - 1.0 + 3.0*g3;			 
	vec3 ijk0 = vec3(i,j,k);
	vec3 ijk1 = vec3(i+i1,j+j1,k+k1);	
	vec3 ijk2 = vec3(i+i2,j+j2,k+k2);
	vec3 ijk3 = vec3(i+1,j+1,k+1);	     
	vec3 gr0 = normalize(vec3(noise3D(ijk0),noise3D(ijk0*2.01),noise3D(ijk0*2.02)));
	vec3 gr1 = normalize(vec3(noise3D(ijk1),noise3D(ijk1*2.01),noise3D(ijk1*2.02)));
	vec3 gr2 = normalize(vec3(noise3D(ijk2),noise3D(ijk2*2.01),noise3D(ijk2*2.02)));
	vec3 gr3 = normalize(vec3(noise3D(ijk3),noise3D(ijk3*2.01),noise3D(ijk3*2.02)));
	float n0 = 0.0;
	float n1 = 0.0;
	float n2 = 0.0;
	float n3 = 0.0;
	float t0 = 0.5 - x0*x0 - y0*y0 - z0*z0;
	if(t0>=0.0)
	{
		t0*=t0;
		n0 = t0 * t0 * dot(gr0, vec3(x0, y0, z0));
	}
	float t1 = 0.5 - x1*x1 - y1*y1 - z1*z1;
	if(t1>=0.0)
	{
		t1*=t1;
		n1 = t1 * t1 * dot(gr1, vec3(x1, y1, z1));
	}
	float t2 = 0.5 - x2*x2 - y2*y2 - z2*z2;
	if(t2>=0.0)
	{
		t2 *= t2;
		n2 = t2 * t2 * dot(gr2, vec3(x2, y2, z2));
	}
	float t3 = 0.5 - x3*x3 - y3*y3 - z3*z3;
	if(t3>=0.0)
	{
		t3 *= t3;
		n3 = t3 * t3 * dot(gr3, vec3(x3, y3, z3));
	}
	return 96.0*(n0+n1+n2+n3);
}

float fbm(vec3 p)
{
	float f;
    f  = 0.50000*(simplex3D( p )); p = p*2.01;
    f += 0.25000*(simplex3D( p )); p = p*2.02;
    f += 0.12500*(simplex3D( p )); p = p*2.03;
    f += 0.06250*(simplex3D( p )); p = p*2.04;
    f += 0.03125*(simplex3D( p )); p = p*2.05;
    f += 0.015625*(simplex3D( p ));
	return f;
}

void planet( out vec4 fragColor, in vec2 fragCoord )
{
    vec3 pos = vec3(fragCoord.xy - u_Dimensions.xy/2.0,0.0); // planet center
    
    // LIGHT
    vec3 l = normalize(vec3(sin(time), sin(time*0.5), (cos(time))));
    
    // PLANET
    float r = u_Dimensions.y/3.0; // radius
    float z_in = sqrt(r*r - pos.x*pos.x - pos.y*pos.y);
    float z_out = sqrt(-r*r + pos.x*pos.x + pos.y*pos.y);
    
    // NORMALS
    vec3 norm = normalize(vec3(pos.x, pos.y, z_in)); // normals from sphere
    vec3 norm_out = normalize(vec3(pos.x, pos.y, z_out)); // normals from outside sphere
    float e = 0.05; // planet rugosity
    float nx = fbm(vec3(norm.x+e, norm.y,   norm.z  ))*0.5+0.5; // x normal displacement
    float ny = fbm(vec3(norm.x,   norm.y+e, norm.z  ))*0.5+0.5; // y normal displacement
    float nz = fbm(vec3(norm.x,   norm.y,   norm.z+e))*0.5+0.5; // z normal displacement
    norm = normalize(vec3(norm.x*nx, norm.y*ny, norm.z*nz));
    //norm = (norm+1.)/2.; // for normals visualization
	
    // TEXTURE
    float n = 1.0-(fbm(vec3(norm.x, norm.y, norm.z))*0.5+0.5); // noise for every pixel in planet
    
    // ATMOS
    float z_in_atm  = (r * in_outer)  / z_in - in_inner;   // inner atmos
    float z_out_atm = (r * out_inner) / z_out - out_outer; // outer atmos
    z_in_atm = max(0.0, z_in_atm);
    z_out_atm = max(0.0, z_out_atm);
    
    // DIFFUSE LIGHT
    float diffuse = max(0.0, dot(norm, l));
    float diffuse_out = max(0.0, dot(norm_out, l)+0.3); // +0.3 because outer atmosphere stills when inner doesn't
    
    fragColor = vec4(vec3(n * diffuse + z_in_atm * diffuse + z_out_atm * diffuse_out),1.0);
	fragColor.rgb *= 4.0;//8.0;
}
#endif

void main()
{
	vec2 texCoords = m_TexCoords.xy * u_Dimensions.xy;
	planet( gl_FragColor, texCoords );
	out_Glow = clamp(gl_FragColor * 2.0, 0.0, 1.0);
	out_Glow.rgb *= out_Glow.rgb;
	out_Glow.rgb *= 0.75;
	
	if (length(gl_FragColor.rgb) > 0.1)
	{
		out_Position = vec4(m_vertPos.xyz, MATERIAL_SUN+1.0);
		out_Normal = vec4( m_Normal.xyz * 0.5 + 0.5, 1.0 );
		out_NormalDetail = vec4(0.0);
	}
	else
	{
		out_Position = vec4(0.0);
		out_Normal = vec4(0.0);
		out_NormalDetail = vec4(0.0);
	}
}

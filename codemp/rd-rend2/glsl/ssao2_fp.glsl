uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;

varying vec2		var_ScreenTex;
varying vec2		var_Dimensions;
varying vec4		var_Local0;

float depthMult = 255.0;

vec3 normal_from_depth(float depth, vec2 texcoords) {
  
  vec2 offset1 = vec2(0.0, 1.0 / var_Dimensions.y);
  vec2 offset2 = vec2(1.0 / var_Dimensions.x, 0.0);
  
  float depth1 = texture2D(u_ScreenDepthMap, texcoords + offset1).r * depthMult;
  float depth2 = texture2D(u_ScreenDepthMap, texcoords + offset2).r * depthMult;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
}

vec3 readNormal(in vec2 coord)  
{  
	 float depth = texture2D(u_ScreenDepthMap, var_ScreenTex).r * depthMult;
	 return normal_from_depth(depth, coord);
}

vec3 posFromDepth(vec2 coord)
{
     float d = texture2D(u_ScreenDepthMap, coord).r * depthMult;
     vec3 tray = mat3x3(gl_ProjectionMatrixInverse)*vec3((coord.x-0.5)*2.0,(coord.y-0.5)*2.0,1.0);
     return tray*d;
}

    //Ambient Occlusion form factor:
float aoFF(in vec3 ddiff,in vec3 cnorm, in float c1, in float c2)
{
          vec3 vv = normalize(ddiff);
          float rd = length(ddiff);
          return (1.0-clamp(dot(readNormal(var_ScreenTex+vec2(c1,c2)),-vv),0.0,1.0)) *
           clamp(dot( cnorm,vv ),0.0,1.0)* 
                 (1.0 - 1.0/sqrt(1.0/(rd*rd) + 1.0));
}
    //GI form factor:
float giFF(in vec3 ddiff,in vec3 cnorm, in float c1, in float c2)
{
          vec3 vv = normalize(ddiff);
          float rd = length(ddiff);
          return 1.0*clamp(dot(readNormal(var_ScreenTex+vec2(c1,c2)),-vv),0.0,1.0)*
                     clamp(dot( cnorm,vv ),0.0,1.0)/
                     (rd*rd+1.0);  
}

float Hash(in vec3 p)
{
    return fract(sin(dot(p,vec3(37.1,61.7, 12.4)))*3758.5453123);
}

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

void main()
{
	float NUM_PASSES = var_Local0.x;

	if (NUM_PASSES < 1) NUM_PASSES = 1.0;

    //read current normal,position and color.
    vec3 n = readNormal(var_ScreenTex);
    vec3 p = posFromDepth(var_ScreenTex);
    vec3 col = texture2D(u_DiffuseMap, var_ScreenTex).rgb;

    //randomization texture
    //vec2 fres = vec2(var_Dimensions.x/128.0*5,var_Dimensions.y/128.0*5);
    //vec3 random = texture2D(grandom, var_ScreenTex*fres.xy).rgb;
	//vec3 random = texture2D(u_DiffuseMap, var_ScreenTex*fres.xy).rgb;
    //random = random*2.0-vec3(1.0);
	//vec3 random = vec3(Noise(p));
	vec3 random = vec3(1.0);
	//random = random*2.0-vec3(1.0);

    //initialize variables:
    float ao = 0.0;
    vec3 gi = vec3(0.0,0.0,0.0);
    float incx = 1.0/var_Dimensions.x;
    float incy = 1.0/var_Dimensions.y;
    float pw = incx;
    float ph = incy;
    float cdepth = texture2D(u_ScreenDepthMap, var_ScreenTex).r * depthMult;

    //3 rounds of 8 samples each. 
    for(float i=0.0; i<NUM_PASSES; ++i) 
    {
       float npw = (pw+0.0007*random.x)/cdepth;
       float nph = (ph+0.0007*random.y)/cdepth;

       vec3 ddiff = posFromDepth(var_ScreenTex+vec2(npw,nph))-p;
       vec3 ddiff2 = posFromDepth(var_ScreenTex+vec2(npw,-nph))-p;
       vec3 ddiff3 = posFromDepth(var_ScreenTex+vec2(-npw,nph))-p;
       vec3 ddiff4 = posFromDepth(var_ScreenTex+vec2(-npw,-nph))-p;
       vec3 ddiff5 = posFromDepth(var_ScreenTex+vec2(0,nph))-p;
       vec3 ddiff6 = posFromDepth(var_ScreenTex+vec2(0,-nph))-p;
       vec3 ddiff7 = posFromDepth(var_ScreenTex+vec2(npw,0))-p;
       vec3 ddiff8 = posFromDepth(var_ScreenTex+vec2(-npw,0))-p;

       ao+=  aoFF(ddiff,n,npw,nph);
       ao+=  aoFF(ddiff2,n,npw,-nph);
       ao+=  aoFF(ddiff3,n,-npw,nph);
       ao+=  aoFF(ddiff4,n,-npw,-nph);
       ao+=  aoFF(ddiff5,n,0,nph);
       ao+=  aoFF(ddiff6,n,0,-nph);
       ao+=  aoFF(ddiff7,n,npw,0);
       ao+=  aoFF(ddiff8,n,-npw,0);

       gi+=  giFF(ddiff,n,npw,nph)*texture2D(u_DiffuseMap, var_ScreenTex+vec2(npw,nph)).rgb;
       gi+=  giFF(ddiff2,n,npw,-nph)*texture2D(u_DiffuseMap, var_ScreenTex+vec2(npw,-nph)).rgb;
       gi+=  giFF(ddiff3,n,-npw,nph)*texture2D(u_DiffuseMap, var_ScreenTex+vec2(-npw,nph)).rgb;
       gi+=  giFF(ddiff4,n,-npw,-nph)*texture2D(u_DiffuseMap, var_ScreenTex+vec2(-npw,-nph)).rgb;
       gi+=  giFF(ddiff5,n,0,nph)*texture2D(u_DiffuseMap, var_ScreenTex+vec2(0,nph)).rgb;
       gi+=  giFF(ddiff6,n,0,-nph)*texture2D(u_DiffuseMap, var_ScreenTex+vec2(0,-nph)).rgb;
       gi+=  giFF(ddiff7,n,npw,0)*texture2D(u_DiffuseMap, var_ScreenTex+vec2(npw,0)).rgb;
       gi+=  giFF(ddiff8,n,-npw,0)*texture2D(u_DiffuseMap, var_ScreenTex+vec2(-npw,0)).rgb;

       //increase sampling area:
       pw += incx;  
       ph += incy;    
    } 
    //ao/=24.0;
    //gi/=24.0;
	ao/=(8*NUM_PASSES);
	gi/=(8*NUM_PASSES);

	//gl_FragColor = vec4(vec3(ao)+gi*5.0,1.0);
    //gl_FragColor = vec4(col-vec3(ao)+gi*5.0,1.0);
	gl_FragColor = vec4((col-vec3(ao)+gi*5.0)*1.15,1.0); // UQ1: *1.15 to compensate for darkening...
}




/*
uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;

varying vec2		var_ScreenTex;
varying vec2		var_Dimensions;

#define saturate(a) clamp(a, 0.0, 1.0)

float depthMult = 255.0;

vec3 normal_from_depth(float depth, vec2 texcoords) {
  
  vec2 offset1 = vec2(0.0, 1.0 / var_Dimensions.y);//vec2(0.0,0.001);
  vec2 offset2 = vec2(1.0 / var_Dimensions.x, 0.0);//vec2(0.001,0.0);
  
  float depth1 = texture2D(u_ScreenDepthMap, texcoords + offset1).r * depthMult;
  float depth2 = texture2D(u_ScreenDepthMap, texcoords + offset2).r * depthMult;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
}

void main ( void )
{ 
  const float total_strength = 1.0;
  //const float base = 0.2;
  const float base = 0.0;
  
  const float area = 0.0075;
  const float falloff = 0.000001;
  
  const float radius = 0.0002;
  
  const int samples = 16;
  const float fsamples = 16.0;

  vec3 sample_sphere[samples] = {
      vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
      vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
      vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
      vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
      vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
      vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
      vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
      vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271)
  };
  
  //vec3 random = normalize( texture2D(RandomTextureSampler, var_ScreenTex * 4.0).rgb );
  vec3 random = vec3(1.0);
  
  float depth = texture2D(u_ScreenDepthMap, var_ScreenTex).r * depthMult;
 
  vec3 position = vec3(var_ScreenTex, depth);
  vec3 normal = normal_from_depth(depth, var_ScreenTex);
  
  float radius_depth = radius/depth;
  float occlusion = 0.0;
  for(int i=0; i < samples; i++) {
    vec3 ray = radius_depth * reflect(sample_sphere[i], random);
    vec3 hemi_ray = position + sign(dot(ray,normal)) * ray;
    
    float occ_depth = texture2D(u_ScreenDepthMap, saturate(hemi_ray.xy)).r * depthMult;
    float difference = depth - occ_depth;
    
    occlusion += step(falloff, difference) * (1.0-smoothstep(falloff, area, difference));
  }
  
  float ao = 1.0 - total_strength * occlusion * (1.0 / fsamples);
  gl_FragColor = vec4(vec3(saturate(ao + base)), 1.0);
}
*/

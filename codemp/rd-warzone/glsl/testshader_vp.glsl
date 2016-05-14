#define SSDM

attribute vec3 attr_Position;
attribute vec4 attr_TexCoord0;

uniform mat4	u_ModelViewProjectionMatrix;
uniform mat4	u_ModelViewMatrix;
uniform mat4	u_ViewProjectionMatrix;

uniform vec2	u_Dimensions;
uniform vec4	u_ViewInfo; // zfar / znear, zfar

uniform vec4	u_Local0;		// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4	u_Local1;		// testshadervalue1, testshadervalue2, testshadervalue3, testshadervalue4
uniform vec4	u_MapInfo;		// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], 0.0

varying vec2   var_TexCoords;
varying vec2   var_Dimensions;
varying vec4   var_ViewInfo; // zfar / znear, zfar
varying vec4   var_Local0; // PASS_NUM, 0, 0, 0

#if defined(SSDM)
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;

uniform vec3		u_ViewOrigin;


//uniform vec3 NegViewDirection;

//uniform float MaxDisplacementDistance;
//uniform float DisplacementFactor;
//uniform float DistanceDisplacementValue1;
//uniform float DisplacementViewDirectionValue1;
//uniform float DisplacementViewDirectionValue2;

#define MaxDisplacementDistance u_Local0.r
#define DisplacementFactor u_Local0.g
#define DistanceDisplacementValue1 u_Local0.b
#define DisplacementViewDirectionValue1 u_Local0.a
#define DisplacementViewDirectionValue2 (u_Local0.a*2.0)

//uniform float screenwidth;
//uniform float screenheight;
#define screenwidth u_Dimensions.x
#define screenheight u_Dimensions.y

vec4 GetPosAndDepth(sampler2D tex, ivec2 coord, int num)
{
	vec4 pos = texelFetch(tex, coord, num);
	float depth = length((pos.xyz - u_ViewOrigin) / u_MapInfo.xyz);
	pos = u_ModelViewMatrix * vec4(pos.xyz, 1.0);
	pos.w = depth;
	return pos;
}

vec4 GetNormalAndHeight(sampler2D tex, ivec2 coord, int num)
{
	vec4 norm = texelFetch(tex, coord, num);
	float height = length(norm.xyz / 3.0);
	norm.w = height;
	return norm;
}

#endif //defined(SSDM)

void main()
{
#if defined(SSDM)
	vec3 NegViewDirection = attr_Position.xyz - u_ViewOrigin;

	if (u_Local1.r >= 3.0)
		NegViewDirection = u_ViewOrigin - attr_Position.xyz;
	else if (u_Local1.r >= 2.0)
		NegViewDirection = -(attr_Position.xyz - u_ViewOrigin);
	else if (u_Local1.r >= 1.0)
		NegViewDirection = -(u_ViewOrigin - attr_Position.xyz);

    vec4 Projection;
   	ivec2 iTexcoord1 = ivec2(attr_TexCoord0.s*screenwidth, attr_TexCoord0.t*screenheight);
    vec4 SceneCameraSpacePosAndDepth = GetPosAndDepth(u_PositionMap, iTexcoord1, 0);

    if(SceneCameraSpacePosAndDepth.w > 0.0)
    {
        if(SceneCameraSpacePosAndDepth.w < MaxDisplacementDistance)
		{
            vec4 CameraViewNormalAndHeight = GetNormalAndHeight(u_NormalMap, iTexcoord1, 0);
            if(!(CameraViewNormalAndHeight.r+CameraViewNormalAndHeight.g+CameraViewNormalAndHeight.b < 0.01))
			//if(!(CameraViewNormalAndHeight.r < 0.01 && CameraViewNormalAndHeight.g < 0.01 && CameraViewNormalAndHeight.b < 0.01))
			{
                vec3 SceneCameraSpacePos = SceneCameraSpacePosAndDepth.xyz;
                CameraViewNormalAndHeight.xyz = 2.0*CameraViewNormalAndHeight.xyz-vec3(1.0);
                float diff = abs(0.5-attr_TexCoord0.s);
                diff = max(diff, abs(0.5-attr_TexCoord0.t));
                diff *= 2.0;
                float df = pow(1.0-diff, DisplacementViewDirectionValue1);
                df *= min(1.0, max(0.0, DisplacementViewDirectionValue2+dot(NegViewDirection,CameraViewNormalAndHeight.xyz)));
                ivec2 iTexcoord2 = iTexcoord1+ivec2(15, 15);
                ivec2 iTexcoord3 = iTexcoord1-ivec2(15, 15);
                ivec2 iTexcoord4 = iTexcoord1+ivec2(-15, 15);
                ivec2 iTexcoord5 = iTexcoord1+ivec2(15, -15);
                float SceneCameraSpaceDepth2 = GetPosAndDepth(u_PositionMap, iTexcoord2, 0).w;
                float SceneCameraSpaceDepth3 = GetPosAndDepth(u_PositionMap, iTexcoord3, 0).w;
                float SceneCameraSpaceDepth4 = GetPosAndDepth(u_PositionMap, iTexcoord4, 0).w;
                float SceneCameraSpaceDepth5 = GetPosAndDepth(u_PositionMap, iTexcoord5, 0).w;
                diff = abs(SceneCameraSpacePosAndDepth.w-SceneCameraSpaceDepth2);
                diff = max(diff, abs(SceneCameraSpacePosAndDepth.w-SceneCameraSpaceDepth3));
                diff = max(diff, abs(SceneCameraSpacePosAndDepth.w-SceneCameraSpaceDepth4));
                diff = max(diff, abs(SceneCameraSpacePosAndDepth.w-SceneCameraSpaceDepth5));
                float df2 = max(0.0, 1.0-df*diff);
                //float dist = DistanceDisplacementValue1*sqrt(SceneCameraSpacePosAndDepth.x*SceneCameraSpacePosAndDepth.x + SceneCameraSpacePosAndDepth.y*SceneCameraSpacePosAndDepth.y + SceneCameraSpacePosAndDepth.z*SceneCameraSpacePosAndDepth.z);            
				//float dist = DistanceDisplacementValue1*(SceneCameraSpacePosAndDepth.x*SceneCameraSpacePosAndDepth.x + SceneCameraSpacePosAndDepth.y*SceneCameraSpacePosAndDepth.y + SceneCameraSpacePosAndDepth.z*SceneCameraSpacePosAndDepth.z);
				float dist = dot(SceneCameraSpacePosAndDepth.xyz, SceneCameraSpacePosAndDepth.xyz);
                SceneCameraSpacePosAndDepth.xyz += df*df2*min(1.0,max(0.0, 1.0-DistanceDisplacementValue1/dist))*DisplacementFactor*abs(CameraViewNormalAndHeight.w)*CameraViewNormalAndHeight.xyz;
                Projection = u_ViewProjectionMatrix*SceneCameraSpacePosAndDepth;
            }
			else
			{
				Projection = u_ModelViewProjectionMatrix*vec4(attr_Position, 1.0);
			}
		}
		else
		{
			Projection = u_ModelViewProjectionMatrix*vec4(attr_Position, 1.0);
		}
	}
	else
	{
		Projection = u_ModelViewProjectionMatrix*vec4(attr_Position, 1.0);
	}

    gl_Position = Projection.xyza;
    var_TexCoords = attr_TexCoord0.st; 
#else //!defined(SSDM)
	gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	var_TexCoords = attr_TexCoord0.st;
#endif //defined(SSDM)
	var_Dimensions = u_Dimensions.st;
	var_ViewInfo = u_ViewInfo;
	var_Local0 = u_Local0;
}

uniform sampler2D				u_DiffuseMap;
uniform sampler2D				u_ScreenDepthMap;

uniform mat4					u_ModelViewProjectionMatrix;
uniform vec4					u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec4					u_LightOrigin;

varying vec2					var_ScreenTex;
varying vec2					var_Dimensions;
varying vec2					projAB;
varying vec3					viewRay;
varying vec3					light_p;

float linearize(float depth)
{
	//return -u_ViewInfo.y * u_ViewInfo.x / (depth * (u_ViewInfo.y - u_ViewInfo.x) - u_ViewInfo.y);
	//return depth / 4.0;
	return 1.0 / mix(u_ViewInfo.z, 1.0, depth);
}

vec3 CalcPosition(void){
    float depth = texture2D(u_ScreenDepthMap, var_ScreenTex).r;
    //float linearDepth = projAB.y / (depth - projAB.x);
	float linearDepth = linearize(depth);
    vec3 ray = normalize(viewRay);
    ray = ray / ray.z;
    return linearDepth * ray;
}

void main(void){
	float out_SH = 1.0;
    vec2 texel_size = vec2(1.0 / var_Dimensions);
    vec3 origin = CalcPosition();

	//gl_FragColor = vec4( origin, 1.0);
	//return;

	/*
	if(origin.z < -99) 
	{
		//Don't check points at infinity
		gl_FragColor = texture2D(u_DiffuseMap, var_ScreenTex);
		//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}
	*/

    vec2 pixOrigin = var_ScreenTex;

    vec3 dir = normalize(light_p - origin);

	//gl_FragColor = vec4( dir, 1.0);
	//return;

    vec4 tempDir = u_ModelViewProjectionMatrix * vec4(dir, 0.0);
    vec2 pixDir = -tempDir.xy / tempDir.w;
    float dirLength = length(pixDir);
    pixDir = pixDir / dirLength;

    vec2 nextT, deltaT;

    if(pixDir.x < 0){
        deltaT.x = -texel_size.x / pixDir.x;
        nextT.x = (floor(pixOrigin.x * var_Dimensions.x) * texel_size.x - pixOrigin.x) / pixDir.x;
    }
    else {
        deltaT.x = texel_size.x / pixDir.x;
        nextT.x = ((floor(pixOrigin.x * var_Dimensions.x) + 1.0) * texel_size.x - pixOrigin.x) / pixDir.x;
    }
    if(pixDir.y < 0){
        deltaT.y = -texel_size.y / pixDir.y;
        nextT.y = (floor(pixOrigin.y * var_Dimensions.y) * texel_size.y - pixOrigin.y) / pixDir.y;
    }
    else {
        deltaT.y = texel_size.y / pixDir.y;
        nextT.y = ((floor(pixOrigin.y * var_Dimensions.y) + 1.0) * texel_size.y - pixOrigin.y) / pixDir.y;
    }


    float t = 0.0;
    ivec2 pixIndex = ivec2(pixOrigin / texel_size);

    while(true){
        //if(t > 0){
            float rayDepth = (origin + t * dir).z;
            vec2 texCoord = pixOrigin + 0.5 * pixDir * t * dirLength;
            float depth = texture2D(u_ScreenDepthMap, texCoord).r;
            //float linearDepth = projAB.y / (depth - projAB.x);
			float linearDepth = linearize(depth);
            if(linearDepth > rayDepth + 0.1){
                out_SH = 0.2;
				gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
				return;
                break;
            }
        //}
        if(nextT.x < nextT.y){
            t = nextT.x;
            nextT.x += deltaT.x;
            if(pixDir.x < 0) pixIndex.x -= 1;
            else pixIndex.x += 1;
        }
        else {
            t = nextT.y;
            nextT.y += deltaT.y;
            if(pixDir.y < 0) pixIndex.y -= 1;
            else pixIndex.y += 1;
        }
        if(pixIndex.x < 0 || pixIndex.x > var_Dimensions.x || pixIndex.y < 0 || pixIndex.y > var_Dimensions.y) 
		{
			break;
			//gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			//return;
		}
    }

	gl_FragColor = texture2D(u_DiffuseMap, var_ScreenTex);
	gl_FragColor.rgb *= out_SH;
	gl_FragColor.a = 1.0;
}

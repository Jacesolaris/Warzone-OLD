uniform sampler2D u_DiffuseMap;

varying vec2		var_Tex1;
varying vec3		var_Normal;
varying vec3		var_Position;
varying vec4		var_Color;
varying vec4		var_LightColor;
varying vec4		var_LightOrigin;
varying float		var_LightRadius;
varying vec3		var_LightDir;


vec3 normal = var_Normal;
vec3 lightDir = var_LightDir;
vec3 eyeVec = var_Position;

void main (void)
{
	float shininess = 0.5;
	vec4 tex_color = texture2D(u_DiffuseMap, var_Tex1);
	vec4 tex_ambient = tex_color * 0.33333;
	vec4 tex_diffuse = tex_ambient;
	vec4 tex_specular = tex_ambient;
	vec4 lightColor = var_LightColor; //*0.33333;

	vec4 final_color = tex_ambient;// + (lightColor * tex_ambient);
							
	vec3 N = normalize(normal);
	vec3 L = normalize(lightDir);
	
	float lambertTerm = dot(N,L);
	
	if(lambertTerm > 0.0)
	{
		final_color += lightColor * tex_diffuse * lambertTerm;	
		
		vec3 E = normalize(eyeVec);
		vec3 R = reflect(-L, N);
		float specular = pow( max(dot(R, E), 0.0), shininess );
		final_color += lightColor * tex_specular * specular;	
	}

	gl_FragColor = final_color;			
}

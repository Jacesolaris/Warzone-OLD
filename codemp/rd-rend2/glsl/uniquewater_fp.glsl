//#ifdef GL_ES
precision highp float;
//#endif

uniform sampler2D u_DiffuseMap;

varying vec2	var_TexCoords;
varying vec2	var_Dimensions;
varying float	time;
varying vec4	var_Local0; // (1=water, 2=lava), 0, 0, 0


vec2 iResolution = var_Dimensions;

out vec4 out_Glow;

float iGlobalTime = time * 1.3;


const float PI = 3.1415926535897932;

//const float speed = 0.1;
const float speed = 0.01;
const float speed_x = 0.3;
const float speed_y = 0.3;

//const float emboss = 0.50;
const float emboss = 1.00;
const float intensity = 2.4;
const int steps = 8;
const float frequency = 6.0;
const int angle = 7;

const float delta = 60.;
const float intence = 700.;

const float reflectionCutOff = 0.022;
const float reflectionIntence = 2000.;

float col(vec2 coord)
{
    float delta_theta = 2.0 * PI / float(angle);
    float col = 0.0;
    float theta = 0.0;

    for (int i = 0; i < steps; i++)
    {
		vec2 adjc = coord;
		theta = delta_theta*float(i);
		adjc.x += cos(theta)*iGlobalTime*speed + iGlobalTime * speed_x;
		adjc.y -= sin(theta)*iGlobalTime*speed - iGlobalTime * speed_y;
		col = col + cos( (adjc.x*cos(theta) - adjc.y*sin(theta))*frequency)*intensity;
    }

	return cos(col);
}

void main(void)
{
	if (var_Local0.r > 1.0)
	{// Lava...
		vec2 p = (var_TexCoords.xy), c1 = p, c2 = p;
		float cc1 = col(c1);

		c2.x += (iResolution.x/delta) * (1.0 / iResolution.x);
		float dx = emboss*(cc1-col(c2))/delta;

		c2.x = p.x;
		c2.y += (iResolution.y/delta) * (1.0 / iResolution.y);
		float dy = emboss*(cc1-col(c2))/delta;

		c1.x += dx*2.;
		c1.y = -(c1.y+dy*2.);

		float alpha = 1.+dot(dx,dy)*intence;
	
		float ddx = dx - reflectionCutOff;
		float ddy = dy - reflectionCutOff;
		if (ddx > 0. && ddy > 0.)
			alpha = pow(alpha, ddx*ddy*reflectionIntence);

		alpha = clamp(alpha, 0.0, 1.0);
	
		vec4 col = texture2D(u_DiffuseMap,c1)*(alpha);
		gl_FragColor = col;
		gl_FragColor.a = 1.0;
	}
	else
	{// Water...
		//gl_FragColor = vec4(0.6, 0.6, 1.0, 0.0);
		gl_FragColor = texture2D(u_DiffuseMap, var_TexCoords.xy);
		gl_FragColor.a = -0.1;
	}
	
#if defined(USE_GLOW_BUFFER)
	if (var_Local0.r > 1)
	{// Lava...
		out_Glow = gl_FragColor;
	}
	else
	{// Water...
		out_Glow = vec4(0.0);
	}
#else
	out_Glow = vec4(0.0);
#endif

}


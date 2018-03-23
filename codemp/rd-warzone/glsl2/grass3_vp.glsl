attribute vec3		attr_Position;
attribute vec3		attr_Normal;

flat out	int		isSlope;

#define M_PI				3.14159265358979323846

float normalToSlope(in vec3 normal) {
	float	forward;
	float	pitch;

	if (normal.g == 0.0 && normal.r == 0.0) {
		if (normal.b > 0.0) {
			pitch = 90;
		}
		else {
			pitch = 270;
		}
	}
	else {
		forward = sqrt(normal.r*normal.r + normal.g*normal.g);
		pitch = (atan(normal.b, forward) * 180 / M_PI);
		if (pitch < 0.0) {
			pitch += 360;
		}
	}

	pitch = -pitch;

	if (pitch > 180)
		pitch -= 360;

	if (pitch < -180)
		pitch += 360;

	pitch += 90.0f;

	return pitch;
}

bool SlopeTooGreat(vec3 normal)
{
#if 1
	float pitch = normalToSlope(normal.xyz);

	if (pitch < 0.0) pitch = -pitch;

	if (pitch > 46.0)
	{
		return true; // This slope is too steep for grass...
	}
#else
	if (normal.z <= 0.73 && normal.z >= -0.73)
	{
		return true;
	}
#endif

	return false;
}

void main()
{
	isSlope = 0;

	if (SlopeTooGreat(attr_Normal.xyz * 2.0 - 1.0))
		isSlope = 1;

	gl_Position = vec4(attr_Position.xyz, 1.0);
}

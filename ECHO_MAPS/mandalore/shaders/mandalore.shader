// skyparms work like this:

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// color will be normalized, so it doesn't matter what range you use

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

textures/skies/normallight
{
	qer_editorimage	textures/skies/sky
	q3map_surfacelight	75
	q3map_lightsubdivide	512
	sun 0.75 0.79 1 250 0 65
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/mandalore/dant 1024 -
}

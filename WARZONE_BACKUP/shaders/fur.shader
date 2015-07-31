// ------------------------------------------------------------------------------
// q3map 2 fur demo shader
// this is expensive, so use it sparingly :)
// note, this can be used with terrain shaders to have a layer with grass...
//
// 
// ------------------------------------------------------------------------------


// ------------------------------------------------------------------------------
// sky shader for demo
// ------------------------------------------------------------------------------

textures/fur/sky
{
	q3map_surfacelight 100
	q3map_sun 1 .85 0.5 65 -30 60
	
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm sky
	
	{
		map textures/fur/sky.tga
	}
}



// ------------------------------------------------------------------------------
// base texture
// ------------------------------------------------------------------------------

textures/fur/pink_base
{
	q3map_cloneshader textures/fur/pink_fur
	
	{
		map $lightmap
	}
	{
		map textures/fur/pink_base.tga
		blendFunc GL_DST_COLOR GL_ZERO
	}
}



// ------------------------------------------------------------------------------
// fur texture
// ------------------------------------------------------------------------------

textures/fur/pink_fur
{
	q3map_lightimage textures/fur/pink_fur.q3map.tga
	
	q3map_notjunc
	q3map_nonplanar
	q3map_bounce 0.0
	q3map_shadeangle 120
	
	// format: q3map_fur <layers> <offset> <fade>
	q3map_fur 8 1.25 0.1
	
	surfaceparm trans
	surfaceparm pointlight
	surfaceparm alphashadow
	surfaceparm nonsolid
	surfaceparm noimpact
	
	// cull none
	nomipmaps
	
	{
		map textures/fur/pink_fur.tga
		//alphaFunc GE128
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}



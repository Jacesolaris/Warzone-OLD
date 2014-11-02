textures/hoth/terrain_0
{
	q3map_texturesize 512 512
	q3map_nonplanar 
	q3map_shadeangle 265
	q3map_tcGen ivector ( 512 0 0 ) ( 0 512 0 )
	{
		map textures/hoth/h_outground.tga
	        rgbGen vertex
	}
	
}

textures/hoth/terrain_1
{
	q3map_texturesize 512 512
	q3map_nonplanar 
	q3map_shadeangle 265
	q3map_tcGen ivector ( 512 0 0 ) ( 0 512 0 )
	{
		map textures/hoth/h_outground_2.tga
		rgbGen vertex
	}

}

textures/hoth/terrain_0to1
{
	q3map_texturesize 512 512
	q3map_nonplanar 
	q3map_shadeangle 265
	q3map_tcGen ivector ( 512 0 0 ) ( 0 512 0 )
	
	{
		map textures/hoth/h_outground.tga
	        rgbGen vertex
                alphaGen vertex
	}
	{
		map textures/hoth/h_outground_2.tga
	        rgbGen vertex
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}

}

textures/hoth/terrain.vertex
{
	{
		map textures/hoth/h_outground.tga
		rgbGen vertex
	}
}
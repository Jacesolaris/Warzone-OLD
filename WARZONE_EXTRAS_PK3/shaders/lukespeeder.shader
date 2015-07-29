models/players/lukespeeder/hull
{	  
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/lukespeeder/hull.JPEG
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/clone
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/lukespeeder/parts
{	  
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/lukespeeder/parts.JPEG
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/clone
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/lukespeeder/window
{
	{
		map models/players/lukespeeder/window.JPEG
		blendfunc add
		depthWrite
		rgbGen lightingdiffuse
	}

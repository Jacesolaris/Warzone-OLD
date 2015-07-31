models/players/naboocrusier_hat/LHu
{	  
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/naboocrusier_hat/LHu.JPEG
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

models/players/juggernaut_hat/windows
{	  
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/naboocrusier_hat/windows.JPEG
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


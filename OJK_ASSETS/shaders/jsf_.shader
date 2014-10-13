models/players/jsfighter/lid
{
	surfaceparm	nonsolid
	surfaceparm	trans
	cull	twosided
    {
        map models/players/jsfighter/lid
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/tests/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

models/players/jsfighter/hull
{	  
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/jsfighter/hull
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/players/jsfighter/glare
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

gfx/effects/jfs_blob
{
	cull	twosided
    {
        map gfx/effects/jsf_blob
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
}


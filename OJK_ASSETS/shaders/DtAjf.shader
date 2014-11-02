
models/players/ajsf/ajsf-lid
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/ajsf/ajsf-lid
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/players/ajsf/ajsf-glare
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}
models/players/ajsf/ajsf-hull
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/ajsf/ajsf-hull
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/players/ajsf/ajsf-glare
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}
models/players/ajsf/ajsf-eng
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/ajsf/ajsf-eng
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/players/ajsf/ajsf-glare
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}
gfx/effects/ajfs_blob
{
	cull	twosided
    {
        map gfx/effects/ajsf_blob
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		glow
        alphaGen vertex
    }
}
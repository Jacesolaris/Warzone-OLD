models/players/droideka/shield
{
	cull	twosided
    {
        map models/players/droideka/shield
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

models/players/droideka/droideka
{
    {
        map models/players/droideka/droideka
        rgbGen lightingDiffuse
    }
    {
        map models/players/droideka/droideka_bump
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
 
}
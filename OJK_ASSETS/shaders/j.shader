textures/water/waterTest
{
	qer_editorimage	textures/water/animwater0
	q3map_tesssize	16
	surfaceparm	nonopaque
	q3map_novertexshadows
	cull	twosided
	deformvertexes	wave	60 sin 0 2 0 0.8
    {
        animMap 8 textures/water/animwater0 textures/water/animwater1 textures/water/animwater2 textures/water/animwater3 textures/water/animwater4 textures/water/animwater5 textures/water/animwater6 textures/water/animwater7 
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen wave inversesawtooth 0 0.2 0 8
        tcMod scroll 0.1 0.1
    }
    {
        animMap 8 textures/water/animwater1 textures/water/animwater2 textures/water/animwater3 textures/water/animwater4 textures/water/animwater5 textures/water/animwater6 textures/water/animwater7 textures/water/animwater0 
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen wave sawtooth 0 0.2 0 8
        tcMod scroll 0.1 0.1
    }
    {
        animMap 7 textures/water/animspec0 textures/water/animspec1 textures/water/animspec2 textures/water/animspec3 textures/water/animspec4 textures/water/animspec5 textures/water/animspec6 textures/water/animspec7 
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen wave inversesawtooth 0 0.3 0 7
        alphaGen lightingSpecular
        tcMod scroll 0.09 0
    }
    {
        animMap 7 textures/water/animspec1 textures/water/animspec2 textures/water/animspec3 textures/water/animspec4 textures/water/animspec5 textures/water/animspec6 textures/water/animspec7 textures/water/animspec0 
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen wave sawtooth 0 0.3 0 7
        alphaGen lightingSpecular
        tcMod scroll 0.09 0
    }
}


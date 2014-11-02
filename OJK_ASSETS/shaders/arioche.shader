textures/quicktrip/desert1_1
{
// q3map_material	Sand

	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/quicktrip/dirt_sand
        rgbGen vertex
        tcMod scale 0.3 0.3
    }
    {
        map textures/quicktrip/shrubery
            surfaceSprites vertical 48 32 128 600
            ssFademax 5000
            ssVariance 1 2
            ssWind 0.5
        alphaFunc GE192
        blendFunc GL_ONE GL_ZERO
        depthWrite
        rgbGen vertex
    }
}

textures/quicktrip/desert1_2
{
// q3map_material	Sand

	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/desert/t_rockwall1
        rgbGen vertex
        tcMod scale 0.3 0.3
    }
}

textures/quicktrip/desert_flat
{
// q3map_material	Sand

	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/quicktrip/sand
        rgbGen vertex
        tcMod scale 0.25 0.25
    }
}

// T2_TRIP ARIOCHE SHADERS

textures/quicktrip/desert1_0
{
// q3map_material	Sand

	qer_editorimage	textures/quicktrip/sand
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/quicktrip/sand
        rgbGen vertex
        tcMod scale 0.25 0.25
    }
    {
        clampmap textures/quicktrip/shrubbery2
            surfaceSprites flattened 32 64 128 1100
            ssFademax 300
            ssVariance 0.5 1.5
            ssWind 0.75
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
    {
        clampmap textures/quicktrip/shrubbery1
            surfaceSprites flattened 48 64 300 1200
            ssFademax 200
            ssVariance 0.5 1
            ssWind 0.25
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
    {
        clampmap textures/quicktrip/shrubbery3
            surfaceSprites flattened 48 86 200 1500
            ssFademax 100
            ssVariance 0.5 0.9
            ssWind 0.5
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
}


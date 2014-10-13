models/weapons2/Thermal/sphereofdoom
{
	qer_trans	0.6
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map models/weapons2/Thermal/sphereofdoom.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map gfx/effects/thermalsphere
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
        tcGen environment
        tcMod scroll 0.3 0.2
        tcMod turb 0.6 0.3 0 0.2
	tcMod scale 3 3
    }
}

gfx/misc/thermal_line
{
	cull	twosided
    {
        map gfx/misc/thermal_line
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}
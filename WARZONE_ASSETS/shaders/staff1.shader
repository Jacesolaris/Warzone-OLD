gfx/effects/staffeffect1
{
	surfaceparm	noimpact
	surfaceparm	trans
	nopicmip
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/mp_b_forcefield
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 2
    }
    {
        map gfx/effects/mp_b_forcefield
        blendFunc GL_ONE GL_ONE
        tcMod scroll 2 1
    }
}

gfx/effects/staffeffect2
{
	qer_editorimage	gfx/effects/mp_b_forcefield_d
	surfaceparm	noimpact
	surfaceparm	trans
	nopicmip
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/mp_b_forcefield_d
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 2
    }
    {
        map gfx/effects/mp_b_forcefield_d1
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 2 1
    }
}

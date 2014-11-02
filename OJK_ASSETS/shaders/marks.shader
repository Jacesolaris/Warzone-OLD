gfx/damage/rivetmark
{
	surfaceparm	nomarks
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/damage/rivetmark
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

gfx/damage/burnmark4
{
	polygonOffset
    {
        clampmap gfx/damage/burnmark3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/damage/burnmark1
{
	polygonOffset
    {
        clampmap gfx/damage/burnmark1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/damage/saberburnmark
{
	polygonOffset
    {
        clampmap gfx/damage/saberburnmark
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

gfx/damage/saberglowmark
{
	polygonOffset
    {
        clampmap gfx/damage/saberburnmark2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        clampmap gfx/damage/saberglowmark
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 1 0.5 0 2
    }
}

gfx/damage/saberglowmark2
{
	polygonOffset
    {
        map textures/common/caps
    }
    {
        map textures/common/caps_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 1 0.5 0 2
    }
}


textures/video/mon_mothma
{
	qer_editorimage	textures/system/_videoTall
	q3map_nolightmap
    {
        videoMap video/mon_mothma
        blendFunc GL_ONE GL_ONE
    }
}

textures/video/raven
{
	qer_editorimage	textures/system/_video512
	q3map_nolightmap
    {
        videoMap video/openinglogos
    }
}

textures/video/mon_mothma_idle
{
	qer_editorimage	textures/system/_videoTall
	q3map_nolightmap
    {
        videoMap video/mon_mothma_idle
        blendFunc GL_ONE GL_ONE
    }
}

textures/video/lightspeed_start
{
	qer_editorimage	textures/system/_video
	q3map_nolightmap
    {
        videoMap video/lightspeed_start
    }
}

textures/video/lightspeed_loop
{
	qer_editorimage	textures/system/_video
	q3map_nolightmap
    {
        videoMap video/lightspeed_loop
    }
}

textures/video/static
{
	qer_editorimage	textures/video/staticline
	qer_trans	0.25
	surfaceparm	nodlight
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/video/staticline
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 1 5
        tcMod scroll 1 0.3
    }
    {
        map gfx/hud/static4
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 1 7
        tcMod scroll 0 0.5
        tcMod turb 0 1 0 0.5
    }
    {
        map $whiteimage
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen const ( 0.325490 0.325490 1.000000 )
    }
}

textures/video/mon_mothma_back
{
	qer_editorimage	textures/system/_videoTall
	q3map_nolightmap
    {
        videoMap video/mon_mothma_back
        blendFunc GL_ONE GL_ONE
    }
}

textures/video/kyle_rummage
{
	qer_editorimage	textures/system/_videoTall
	q3map_nolightmap
    {
        videoMap video/kyle_rummage
        blendFunc GL_ONE GL_ONE
    }
}

textures/video/kyle_rummage_stop
{
	qer_editorimage	textures/system/_videoTall
	q3map_nolightmap
    {
        videoMap video/kyle_rummage_stop
        blendFunc GL_ONE GL_ONE
    }
}

textures/video/morgan_talking
{
	qer_editorimage	textures/system/_videoTall
	q3map_nolightmap
    {
        videoMap video/morgan_talking
        blendFunc GL_ONE GL_ONE
    }
}

textures/video/morgan_idle
{
	qer_editorimage	textures/system/_videoTall
	q3map_nolightmap
    {
        videoMap video/morgan_idle
        blendFunc GL_ONE GL_ONE
    }
}

textures/video/morgan_side_idle
{
	qer_editorimage	textures/system/_videoTall
	q3map_nolightmap
    {
        videoMap video/morgan_side_idle
        blendFunc GL_ONE GL_ONE
    }
}

textures/video/morgan_side_talking
{
	qer_editorimage	textures/system/_videoTall
	q3map_nolightmap
    {
        videoMap video/morgan_side_talking
        blendFunc GL_ONE GL_ONE
    }
}

textures/doomgiver/kyle
{

	q3map_nolightmap
    {
        map textures/doomgiver/kyle
        blendFunc GL_ONE GL_ZERO
    }
    {
        map gfx/hud/static
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 12
    }
    {
        map gfx/hud/static
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 8
    }
}


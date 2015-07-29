models/map_objects/bespin/twinpodccglow
{
	qer_editorimage	models/players/twin_pod/model
	//q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/bespin/twinpodcc
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/bespin/twinpodccglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.25 0 25
    }
    {
        map models/map_objects/bespin/twinpodccglow
        blendFunc GL_ONE GL_ONE
        glow
    }
}
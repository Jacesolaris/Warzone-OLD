models/players/KC_anakin/hair
{
	cull	disable
    {
        map models/players/KC_anakin/hair
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}
models/players/KC_anakin/Torso
{
	cull	disable
    {
        map models/players/KC_anakin/Torso
        rgbGen lightingDiffuse
        //alphaFunc GE128
    }
    {
        map models/players/KC_anakin/torsoleather
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen lightingSpecular
	detail
    }
}
models/players/KC_anakin/arms
{
	cull	disable
    {
        map models/players/KC_anakin/arms
        rgbGen lightingDiffuse
        alphaFunc GE128
    }
}

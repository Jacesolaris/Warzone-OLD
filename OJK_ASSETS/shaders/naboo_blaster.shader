models/weapons2/naboo_blaster/naboo_blaster
{
    {
        map models/weapons2/naboo_blaster/naboo_blaster
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
        map models/weapons2/naboo_blaster/naboo_blaster_s
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}
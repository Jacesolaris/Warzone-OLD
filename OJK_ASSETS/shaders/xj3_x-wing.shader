models/players/xj3_x-wing/xwings
{
    {
        map models/players/xj3_x-wing/xwings
        rgbGen lightingDiffuse
    }
    {
        map models/players/xj3_x-wing/xwings_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/xj3_x-wing/xwglass_shd
{
    {
        map textures/colors/black
        rgbGen lightingDiffuse
    }
    {
        map textures/common/env_chrome
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        alphaGen const 0.9
        tcGen environment
    }
    {
        map models/players/xj3_x-wing/xwglass_shd_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen lightingSpecular
    }
    {
        map models/players/xj3_x-wing/xwglass_shd
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/players/xj3_x-wing/xwbody
{
    {
        map models/players/xj3_x-wing/xwbody
        rgbGen lightingDiffuse
    }
    {
        map models/players/xj3_x-wing/xwbody_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}
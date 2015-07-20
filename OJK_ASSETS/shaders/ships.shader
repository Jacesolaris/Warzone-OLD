models/map_objects/ships/cart
{
	/*q3map_nolightmap
    {
        map models/map_objects/ships/cart
        rgbGen lightingDiffuse
    }
    */
    
    {
        map models/map_objects/ships/cart
        //rgbGen lightingDiffuse
        rgbGen vertex
    }
    {
        map models/players/xj3_x-wing/xwbody_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/imp_mine/xwings
{
  {
        //map models/players/xj3_x-wing/xwings
        map models/map_objects/imp_mine/xwings
        //rgbGen lightingDiffuse
        rgbGen vertex
    }
    {
        map models/players/xj3_x-wing/xwings_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/imp_mine/xwglass_shd
{
    {
        map textures/colors/black
        //rgbGen lightingDiffuse
        rgbGen vertex
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

models/map_objects/imp_mine/xwbody
{
    {
        //map models/players/xj3_x-wing/xwbody
        map models/map_objects/imp_mine/xwbody
        //rgbGen lightingDiffuse
        rgbGen vertex
    }
    {
        map models/players/xj3_x-wing/xwbody_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}


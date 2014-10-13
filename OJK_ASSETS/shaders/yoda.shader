models/players/yoda/h_alpha
{
	q3map_nolightmap
    {
        map models/players/yoda/h_alpha.tga
        alphaFunc GE192
        depthWrite
        rgbGen lightingDiffuse
    }
    {
        map models/players/yoda/h_alpha.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}


models/players/yoda/yoda_head
{      
        {
		map models/players/yoda/yoda_head.tga
                blendFunc GL_ONE GL_ZERO
		rgbGen lightingDiffuse

	} 
        {
                map models/players/yoda/eye_env.jpg
                blendFunc GL_ONE GL_ONE
                tcGen environment
                rgbGen lightingDiffuse
	}  
        {
		map models/players/yoda/yoda_head.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingDiffuse
	}
}


models/players/yoda/yoda_head_blue
{      
        {
		map models/players/yoda/yoda_head_blue.tga
                blendFunc GL_ONE GL_ZERO
		rgbGen lightingDiffuse

	} 
        {
                map models/players/yoda/eye_env.jpg
                blendFunc GL_ONE GL_ONE
                tcGen environment
                rgbGen lightingDiffuse
	}  
        {
		map models/players/yoda/yoda_head_blue.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingDiffuse
	}
}


models/players/yoda/yoda_head_red
{      
        {
		map models/players/yoda/yoda_head_red.tga
                blendFunc GL_ONE GL_ZERO
		rgbGen lightingDiffuse

	} 
        {
                map models/players/yoda/eye_env.jpg
                blendFunc GL_ONE GL_ONE
                tcGen environment
                rgbGen lightingDiffuse
	}  
        {
		map models/players/yoda/yoda_head_red.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingDiffuse
	}
}


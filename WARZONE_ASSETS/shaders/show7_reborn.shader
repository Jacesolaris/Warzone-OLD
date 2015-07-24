models/players/reborn/arm_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/torso_show7_evil
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
    }
    
    
}
models/players/reborn/shoulder_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/torso_show7_evil
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
    }
    
    
}

models/players/reborn/hand_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/hand_show7_evil
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
    }
    
    
}

models/players/reborn/torso_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/torso_show7_evil
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
    }
    {
        map models/players/reborn/lightblue
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0 1 5 50
        tcGen environment
        tcMod scroll -0.2 0.2
        tcMod rotate 50
        tcMod entityTranslate
        tcMod stretch square 0 1 0 1
    }
	
    

}

models/players/reborn/legs_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/legs_show7_evil
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
    }
    {
        map models/players/reborn/lightblue
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0 1 5 50
        tcGen environment
        tcMod scroll -0.2 0.2
        tcMod rotate 50
        tcMod entityTranslate
        tcMod stretch square 0 1 0 1
    }
	

}

models/players/reborn/hood_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/hood_show7_evil
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
    }
    {
        map models/players/reborn/lightblue
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0 1 5 50
        tcGen environment
        tcMod scroll -0.2 0.2
        tcMod rotate 50
        tcMod entityTranslate
        tcMod stretch square 0 1 0 1
    }
	

}
models/players/reborn/boots_hips_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/boots_hips_show7_evil
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
        
    }
    {
        map models/players/reborn/lightblue
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0 1 5 50
        tcGen environment
        tcMod scroll -0.2 0.2
        tcMod rotate 50
        tcMod entityTranslate
        tcMod stretch square 0 1 0 1
    }
	

}

models/players/reborn/flap_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/flap_show7_evil
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
    }
    {
        map models/players/reborn/lightblue
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0 1 5 50
        tcGen environment
        tcMod scroll -0.2 0.2
        tcMod rotate 50
        tcMod entityTranslate
        tcMod stretch square 0 1 0 1
    }
	

}

models/players/reborn/mouth_eyes_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/mouth_eyes_show7_gold
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
    }
	

}
models/players/reborn/face_show7_evil
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/face_show7_gold
        blendfunc add
		rgbGen lightingdiffuse
		depthWrite
    }
	

}





































models/players/reborn/arm_show7_silver
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/torso_show7_silver
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    
    {
		map models/players/show7/tob
		blendFunc GL_DST_COLOR GL_ONE
		blendFunc add
		rgbGen wave noise 0.2 0.7 0 1
		tcMod scroll 0.35 0.15 
		tcMod turb 0.55 0.25 0 0.15
    }    
    
}
models/players/reborn/shoulder_show7_silver
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/torso_show7_silver
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    
    {
		map models/players/show7/tob
		blendFunc GL_DST_COLOR GL_ONE
		blendFunc add
		rgbGen wave noise 0.2 0.7 0 1		
		tcMod scroll 0.35 0.15 
		tcMod turb 0.55 0.25 0 0.15
    }    
    
}

models/players/reborn/hand_show7_silver
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/hand_show7_silver
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    {
		map models/players/show7/tob
		blendFunc GL_DST_COLOR GL_ONE
		blendFunc add
		rgbGen wave noise 0.2 0.7 0 1
		tcMod scroll 0.35 0.15 
		tcMod turb 0.55 0.25 0 0.15
    }    
    
}

models/players/reborn/torso_show7_silver
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/torso_show7_silver
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    	
    	
    
    

}

models/players/reborn/legs_show7_silver
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/legs_show7_silver
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    	

}

models/players/reborn/hood_show7_silver
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/hood_show7_silver
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    	

}
models/players/reborn/boots_hips_show7_silver
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/boots_hips_show7_silver
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
        
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    	

}

models/players/reborn/flap_show7_silver
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/flap_show7_silver
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    	

}




































models/players/reborn/arm_show7_fire
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/torso_show7_fire
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    
    {
		map models/players/show7/tob_fire
		blendFunc GL_DST_COLOR GL_ONE
		blendFunc add
		rgbGen wave noise 0.2 0.7 0 1
		tcMod scroll 0.35 0.15 
		tcMod turb 0.55 0.25 0 0.15
    }    
    
}
models/players/reborn/shoulder_show7_fire
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/torso_show7_fire
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    
    {
		map models/players/show7/tob_fire
		blendFunc GL_DST_COLOR GL_ONE
		blendFunc add	
		rgbGen wave noise 0.2 0.7 0 1	
		tcMod scroll 0.35 0.15 
		tcMod turb 0.55 0.25 0 0.15
    }    
    
}

models/players/reborn/hand_show7_fire
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/hand_show7_fire
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    {
		map models/players/show7/tob_fire
		blendFunc GL_DST_COLOR GL_ONE
		blendFunc add
		rgbGen wave noise 0.2 0.7 0 1
		tcMod scroll 0.35 0.15 
		tcMod turb 0.55 0.25 0 0.15
    }    
    
}

models/players/reborn/torso_show7_fire
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/torso_show7_fire
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    	
    	

}

models/players/reborn/legs_show7_fire
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/legs_show7_fire
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}

}

models/players/reborn/hood_show7_fire
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/hood_show7_fire
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}
    	{
		map models/players/show7/tob_fire
		blendFunc GL_DST_COLOR GL_ONE
		blendFunc add
		rgbGen wave noise 0.2 0.7 0 1
		tcMod scroll 0.35 0.15 
		tcMod turb 0.55 0.25 0 0.15
    }

}
models/players/reborn/boots_hips_show7_fire
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/boots_hips_show7_fire
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}

}

models/players/reborn/flap_show7_fire
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/players/reborn/flap_show7_fire
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
	{
    
        map gfx/effects/chr_white_add_mild.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        tcGen environment
    	}

}

























models/players/reborn/hood_show7_gold
{
	q3map_nolightmap
	q3map_onlyvertexlighting	
	qer_editorimage	models/players/reborn/hood_show7_gold
    {
        map models/players/reborn/hood_show7_gold
        blendFunc GL_ONE GL_ZERO
        tcMod scale 3 1
    }
    {
        map models/players/reborn/hood_show7_gold
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.15 0 8
        tcMod scale 3 1
   }
    {
        map models/players/reborn/shock_ripple
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.25 0 8
        tcMod scroll 0 0.1
        tcMod scale 10 10
    }
    {
        map models/players/reborn/hood_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	rgbGen lightingDiffuse
    }
{
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
        map models/players/reborn/hood_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {

map gfx/effects/flamejet
	blendFunc GL_ONE GL_ONE
	tcGen environment
                    tcMod scroll -0.2 0.2
       }
}

models/players/reborn/wrist_show7_gold
{
	q3map_nolightmap
	q3map_onlyvertexlighting	
    
    
    
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
     {
		map models/players/show7/tob_fire
		blendFunc GL_DST_COLOR GL_ONE
		blendFunc add		
		tcMod scroll 0.35 0.15 
		tcMod turb 0.55 0.25 0 0.15
    }    
}
models/players/reborn/arm_show7_gold
{
	q3map_nolightmap
	q3map_onlyvertexlighting	
	qer_editorimage	models/players/reborn/torso_show7_gold
    {
        map models/players/reborn/torso_show7_gold
        blendFunc GL_ONE GL_ZERO
        tcMod scale 3 1
    }
    {
        map models/players/reborn/torso_show7_gold
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.15 0 8
        tcMod scale 3 1
  
     
    }
    {
        map models/players/reborn/torso_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	rgbGen lightingDiffuse
    }
{
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
        map models/players/reborn/torso_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
     {

map gfx/effects/flamejet
	blendFunc GL_ONE GL_ONE
	tcGen environment
                    tcMod scroll -0.2 0.2
       }
}

models/players/reborn/torso_show7_gold
{
	q3map_nolightmap
	q3map_onlyvertexlighting	
	qer_editorimage	models/players/reborn/torso_show7_gold
    {
        map models/players/reborn/torso_show7_gold
        blendFunc GL_ONE GL_ZERO
        tcMod scale 3 1
    }
    {
        map models/players/reborn/torso_show7_gold
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.15 0 8
        tcMod scale 3 1
  
     
    }
    {
        map models/players/reborn/torso_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	rgbGen lightingDiffuse
    }
{
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
        map models/players/reborn/torso_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    
}




models/players/reborn/legs_show7_gold
{
	q3map_nolightmap
	q3map_onlyvertexlighting	
	qer_editorimage	models/players/reborn/legs_show7_gold
    {
        map models/players/reborn/legs_show7_gold
        blendFunc GL_ONE GL_ZERO
        tcMod scale 3 1
    }
    {
        map models/players/reborn/legs_show7_gold
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.15 0 8
        tcMod scale 3 1
 
    }
    {
        map models/players/reborn/legs_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	rgbGen lightingDiffuse
    }
{
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
        map models/players/reborn/legs_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}
models/players/reborn/boots_hips_show7_gold
{
	q3map_nolightmap
	q3map_onlyvertexlighting	
	qer_editorimage	models/players/reborn/boots_hips_show7_gold
    {
        map models/players/reborn/boots_hips_show7_gold
        blendFunc GL_ONE GL_ZERO
        tcMod scale 3 1
    }
    {
        map models/players/reborn/boots_hips_show7_gold
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.15 0 8
        tcMod scale 3 1
    }
    {
        map models/players/reborn/shock_ripple
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.25 0 8
        tcMod scroll 0 0.1
        tcMod scale 10 10
    }
    {
        map models/players/reborn/boots_hips_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	rgbGen lightingDiffuse
    }
{
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
        map models/players/reborn/boots_hips_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {

map gfx/effects/flamejet
	blendFunc GL_ONE GL_ONE
	tcGen environment
                    tcMod scroll -0.2 0.2
       }
}
models/players/reborn/hand_show7_gold
{
	q3map_nolightmap
	q3map_onlyvertexlighting	
	qer_editorimage	models/players/reborn/hand_show7_gold
    {
        map models/players/reborn/hand_show7_gold
        blendFunc GL_ONE GL_ZERO
        tcMod scale 3 1
    }
    {
        map models/players/reborn/hand_show7_gold
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.15 0 8
        tcMod scale 3 1
    }
    {
        map models/players/reborn/shock_ripple
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.25 0 8
        tcMod scroll 0 0.1
        tcMod scale 10 10
    }
    {
        map models/players/reborn/hand_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	rgbGen lightingDiffuse
    }
{
        map gfx/effects/chr_inv
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
        map models/players/reborn/hand_show7_gold
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {

map gfx/effects/flamejet
	blendFunc GL_ONE GL_ONE
	tcGen environment
                    tcMod scroll -0.2 0.2
       }
}

models/players/reborn/mouth_eyes_show7_gold
{
	q3map_nolightmap
	q3map_onlyvertexlighting	
	qer_editorimage	models/players/reborn/mouth_eyes_show7_gold
    {
        map models/players/reborn/mouth_eyes_show7_gold
        blendFunc GL_ONE GL_ZERO
        tcMod scale 3 1
    }
    

    
}

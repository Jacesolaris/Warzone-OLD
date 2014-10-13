models/players/stap_mc/stap_mc
{
	surfaceparm metalsteps
	q3map_nolightmap
{
	map models/players/stap_mc/stap_mc
	////// blendfunc GL_DST_COLOR GL_ONE
	rgbGen lightingDiffuse
}
{
	map $lightmap 
	blendfunc filter
	tcGen lightmap 
}
{
	map models/players/stap_mc/env_chrome
	blendfunc add
	tcGen environment 
}
{
      map models/players/stap_mc/stap_mc_glow
      blendFunc GL_ONE GL_ONE
      glow
	rgbGen identity
}
}

models/players/stap_mc/stap_blue
{
	surfaceparm metalsteps
	q3map_nolightmap
{
	map models/players/stap_mc/stap_blue
	////// blendfunc GL_DST_COLOR GL_ONE
	rgbGen lightingDiffuse
}
{
	map $lightmap 
	blendfunc filter
	tcGen lightmap 
}
{
	map models/players/stap_mc/env_chrome
	blendfunc add
	tcGen environment 
}
{
      map models/players/stap_mc/stap_mc_glow
      blendFunc GL_ONE GL_ONE
      glow
	rgbGen identity
}
}

models/players/stap_mc/stap_red
{
	surfaceparm metalsteps
	q3map_nolightmap
{
	map models/players/stap_mc/stap_red
	////// blendfunc GL_DST_COLOR GL_ONE
	rgbGen lightingDiffuse
}
{
	map $lightmap 
	blendfunc filter
	tcGen lightmap 
}
{
	map models/players/stap_mc/env_chrome
	blendfunc add
	tcGen environment 
}
{
      map models/players/stap_mc/stap_mc_glow
      blendFunc GL_ONE GL_ONE
      glow
	rgbGen identity
}
}

models/players/stap_mc/stap_green
{
	surfaceparm metalsteps
	q3map_nolightmap
{
	map models/players/stap_mc/stap_green
	////// blendfunc GL_DST_COLOR GL_ONE
	rgbGen lightingDiffuse
}
{
	map $lightmap 
	blendfunc filter
	tcGen lightmap 
}
{
	map models/players/stap_mc/env_chrome
	blendfunc add
	tcGen environment 
}
{
      map models/players/stap_mc/stap_mc_glow
      blendFunc GL_ONE GL_ONE
      glow
	rgbGen identity
}
}

models/players/stap_mc/stap_black
{
	surfaceparm metalsteps
	q3map_nolightmap
{
	map models/players/stap_mc/stap_black
	////// blendfunc GL_DST_COLOR GL_ONE
	rgbGen lightingDiffuse
}
{
	map $lightmap 
	blendfunc filter
	tcGen lightmap 
}
{
	map models/players/stap_mc/env_chrome
	blendfunc add
	tcGen environment 
}
{
      map models/players/stap_mc/stap_mc_glow
      blendFunc GL_ONE GL_ONE
      glow
	rgbGen identity
}
}

models/players/stap_mc/stap_inv
{
	qer_editorimage	textures/common/etest4
	surfaceparm	forcefield
	q3map_nolightmap
	q3map_onlyvertexlighting
{
      map textures/colors/black
      rgbGen vertex
}
{
      map textures/common/environ9a
      blendFunc GL_ONE GL_ONE
      rgbGen const ( 0.439216 0.439216 0.439216 )
      tcGen environment
      tcMod scale 0.25 0.25
}	
{
      map models/players/stap_mc/rglow
      blendFunc GL_ONE GL_ONE
      depthWrite
      glow
	rgbGen identity
      tcGen environment
}
{
      map models/players/stap_mc/stap_mc_glow
      blendFunc GL_ONE GL_ONE
      glow
	rgbGen identity
}
}







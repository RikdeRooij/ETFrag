// etfrag.shader
// ETFrag
// Gunrstar* 20:15 24-9-2010


etfrag/limbosky
{
	nopicmip
	{
		map textures/skies_sd/wurzburg_clouds.tga
		rgbGen identityLighting
		tcMod scale 1.5 .75
		tcMod scroll 0.01 -0.005
	}
	{
		map textures/skies_sd/wurzburg_clouds.tga
		blendFunc GL_DST_COLOR GL_ONE
		rgbGen identityLighting
		tcMod scale 1.25 .85
		tcMod transform 0 1 1 0 0 0
		tcMod scroll 0.03 -0.015
	}
	{
		map textures/skies_sd/ydnar_lightning.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen wave triangle -8 9 0.33 0.137
		alphaGen wave noise -3 4 0 2.37
		tcMod scale 2 1
		tcMod scroll 0.002 -0.0025
	}
}

gfx/limbo/nadesoutofstock
{
	nomipmaps
	nopicmip
	{
		map gfx/limbo/nadesoutofstock.tga
		blendfunc blend
		rgbgen vertex
	}
}

gfx/limbo/rframe
{
	nomipmaps
	nopicmip
	{
		clampmap gfx/limbo/rframe.tga
	}
}

gfx/colors/awhite
{
	{
		map gfx/colors/awhite.tga
		blendFunc GL_ONE GL_ZERO
		rgbGen lightingDiffuse
	}
}

// models/players/temperate/allied/leg01.tga
models/players/temperate/allied/engineer/body.tga
{
	//cull none
	//implicitMap -
	{
		map models/players/temperate/allied/engineer/body.tga
		blendfunc GL_ONE GL_ZERO
		rgbGen lightingDiffuse
	}
	{
		map textures/sfx/construction
		blendfunc GL_ONE GL_ONE
		rgbgen const ( 0.35 0.10 0.10 )
		tcGen environment
			tcmod rotate 30
			tcmod scroll 1 .1
	}
}

gfx/2d/compass3
{
	nopicmip
	nocompress
	nomipmaps
	{
		clampmap gfx/2d/compass3.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		depthFunc equal
		rgbGen identity
	}
}

levelshots/automap_mask_square
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/2d/square_mask.tga
		alphafunc GE128	
	}
}

// ------------------------------
// etpub.shader

gfx/2d/friendlycross
{
        nocompress
        nopicmip
        {
                map gfx/2d/friendlycross.tga
                blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        }
}

// Shoutcast landmine shader
textures/sfx/shoutcast_landmine
{
	cull none
	deformVertexes wave 1 sin -0.5 0 0 1
	noPicmip
	surfaceparm trans
	{
		map textures/sfx/construction.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen entity
		tcGen environment
		tcMod scroll 0.025 -0.07625
	}
}

// forty - transparent gun models

textures/sfx/transRed
{ 
     noPicmip 
     surfaceparm trans 
     { 
          map textures/sfx/construction.tga
          blendFunc GL_SRC_ALPHA GL_ONE
          rgbgen const ( 1.0 0.0 0.0 )
          tcMod scroll 0.025 0.025 
     } 
}

textures/sfx/transGreen
{ 
     noPicmip 
     surfaceparm trans 
     { 
          map textures/sfx/construction.tga
          blendFunc GL_SRC_ALPHA GL_ONE
          rgbgen const ( 0.0 1.0 0.0 )
          tcMod scroll 0.025 0.025 
     } 
}

textures/sfx/transBlue
{ 
     noPicmip 
     surfaceparm trans 
     { 
          map textures/sfx/construction.tga
          blendFunc GL_SRC_ALPHA GL_ONE
          rgbgen const ( 0.0 0.0 1.0 )
          tcMod scroll 0.025 0.025 
     } 
}

textures/sfx/transWhite
{ 
     noPicmip 
     surfaceparm trans 
     { 
          map textures/sfx/construction.tga
          blendFunc GL_SRC_ALPHA GL_ONE
          rgbgen const ( 1.0 1.0 1.0 )
          tcMod scroll 0.025 0.025 
     } 
}


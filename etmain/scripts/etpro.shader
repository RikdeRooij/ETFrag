// generated on 2005-09-30 10:36:42 UTC from:
// $Id: makeshaderscript 2383 2005-08-03 19:45:20Z zinx $

images/blackmask
{
	nopicmip 
	nocompress
	nomipmaps
	{
		map gfx/colors/ablack.tga
		depthwrite
		blendfunc blend
		rgbGen identity
		alphaGen vertex
	}
}

sprites/voicemedic
{
	nopicmip
	nocompress
	nomipmaps
	{
		map sprites/voicemedic.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

sprites/objective
{
	nopicmip
	nocompress
	nomipmaps
	{
		map sprites/objective.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ranks/rank1
{
	{
		map models/players/temperate/common/rank1.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank2
{
	{
		map models/players/temperate/common/rank2.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank3
{
	{
		map models/players/temperate/common/rank3.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank4
{
	{
		map models/players/temperate/common/rank4.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank5
{
	{
		map models/players/temperate/common/rank5.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank6
{
	{
		map models/players/temperate/common/rank6.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank7
{
	{
		map models/players/temperate/common/rank7.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank8
{
	{
		map models/players/temperate/common/rank8.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank9
{
	{
		map models/players/temperate/common/rank9.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank10
{
	{
		map models/players/temperate/common/rank10.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

ranks/rank11
{
	{
		map models/players/temperate/common/rank11.tga
		alphaFunc GE128
		rgbGen lightingDiffuse
	}
}

gfx/2d/net
{
	nocompress
	nopicmip
	{
		map gfx/2d/net.tga
		blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

models/gibs/gibs
{
	{
		map models/gibs/gibs.jpg
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingDiffuse 
	}
	{
		map models/gibs/gibs.jpg
		blendFunc GL_ZERO GL_ONE
		rgbGen lightingDiffuse 
	}
}

models/weapons2/shells/m_shell
{
	{
		map models/weapons2/shells/M_shell.jpg
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingDiffuse 
	}
	{
		map models/weapons2/shells/M_shell.jpg
		blendFunc GL_ZERO GL_ONE
		rgbGen lightingDiffuse 
	}
}

// this isn't the shader you're looking for
textures/egypt_floor_sd/marblefloor_c04
{
	{
		map textures/goldrush-env.tga
		rgbGen identity
		tcGen environment
		tcMod scale 0.9 0.9
	}
	{
		map textures/egypt_floor_sd/marblefloor_c04.tga
		blendfunc blend
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
}

icons/iconw_landmine_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_landmine_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_m1_garand_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_m1_garand_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_m1_garand_gren_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_m1_garand_gren_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_kar98_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_kar98_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_kar98_gren_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_kar98_gren_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_fg42_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_fg42_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_mg42_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_mg42_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_mortar_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_mortar_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_syringe_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_syringe_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_syringe2_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_syringe2_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_binoculars_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_binoculars_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_satchel_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_satchel_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_radio_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_radio_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

icons/iconw_silencer_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_silencer_1_select.tga
		blendFunc blend
		rgbGen vertex
	}
}

gfx/limbo/cm_alliedgren
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_alliedgren.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_axisgren
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_axisgren.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_bankdoor
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_bankdoor.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_bo_allied
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_bo_allied.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_bo_axis
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_bo_axis.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_churchill
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_churchill.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_constallied
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_constallied.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_constaxis
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_constaxis.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_dynamite
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_dynamite.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_flagallied
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_flagallied.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_flagaxis
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_flagaxis.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_fuel
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_fuel.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_goldbars
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_goldbars.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_guncontrols
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_guncontrols.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_healthammo
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_healthammo.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_jagdpanther
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_jagdpanther.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_oasiswall
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_oasiswall.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_oasis_pakgun
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_oasis_pakgun.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_radarbox
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_radarbox.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_radar_maindoor
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_radar_maindoor.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_radar_sidedoor
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_radar_sidedoor.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_satchel
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_satchel.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_truck
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_truck.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/cm_tug
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/cm_tug.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/mort_hit
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/mort_hit.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/limbo/mort_target
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/limbo/mort_target.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/loading/pin_allied
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/loading/pin_allied.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/loading/pin_axis
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/loading/pin_axis.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/loading/pin_neutral
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/loading/pin_neutral.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/loading/pin_shot
{
	nopicmip
	nocompress
	nomipmaps
	{
		map gfx/loading/pin_shot.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

sprites/landmine_allied
{
	nopicmip
	nocompress
	nomipmaps
	{
		map sprites/landmine_allied.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

sprites/landmine_axis
{
	nopicmip
	nocompress
	nomipmaps
	{
		map sprites/landmine_axis.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_ammo_blue
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_ammo_blue.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_ammo_red
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_ammo_red.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_arrow_blue
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_arrow_blue.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_arrow_red
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_arrow_red.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_gun_blue
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_gun_blue.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_gun_red
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_gun_red.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_health_blue
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_health_blue.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_health_red
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_health_red.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_player_highlight
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_player_highlight.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_spy_blue
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_spy_blue.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_spy_red
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_spy_red.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_wrench_blue
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_wrench_blue.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/mp_wrench_red
{
	nopicmip
	nocompress
	nomipmaps
	{
		map ui/assets/mp_wrench_red.tga
		depthFunc equal
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

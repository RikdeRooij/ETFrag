Wolfenstein Enemy Territory
	Enemy Territory Frag Modification
		**ETFrag 0.1.0 beta**

	Kim 'Shownie' Reinhold 		Lead Artist
	Rik 'codePRK' de Rooij 		Lead Programmer
	Julius 'Remburanto' Guldbog 	Audio Director
	Thomas 'BackSnip3' Prioul 	Models
	Bente 'SwatElite' Peters 	Programmer
---

ETFrag Features:
===============
ETPro inspired hitboxes
Deployable Turrets
XP killmarker on HUD
Pick up any weapon
Bullet penetration
Killingspree announcer (audio & text)
Multikills announcer (audio & text)
Number of gibs are counted in gamestats
Dynamites can be re-armed after being defused
Covert Ops can steal new uniforms when already use one
Ingame admin panel (modified shrubbot system)
Character customizing in limbo
Clan-Chat
Removed Hudeditor
"frag" instead of "kill" messages

Client Cvars:
changed: cg_drawCompass
	2: draw a square compass
added: cg_hud
added: cg_hudColor
added: cg_hudAlpha
added: cg_numPopups
added: cg_popupDelay
added: cg_shadowedPopups
added: cg_drawPickupMsg
added: cg_drawObjectiveFP (g_weapon 512)
added: cg_drawHardcore
added: cg_hitSounds
added: cg_fireteamAlpha
added: cg_muzzleFlash
added: cg_visWeapSelectTime

Server Cvars:
added: g_realism (bitflag)
	1: jumping needs stamina
	2: slower health regeneration
	4: don't loose uniform while alive
	8: walk slower at low health (etpub: g_healthSpeed)
	16: damage based on where target was hit (etpub: g_dmg 1)
	32: damage over distance reduced faster and earlier (etpub: g_dmg 2)
added: g_hardcore
added: g_adminsysDir
added: g_noFallDamage
added: g_weapons (bitflag)
	256: pick up any weapon
	512: no weapon when holding objective

Commands:
added: "dropweapon"
added: "/pm" & "/m"

ETPub Features:
==============
HUD Editor
Banners
Killingsprees
Multikills
Shrubbot

Server Cvars:
changed: g_voting
changed: g_slashKill
changed: g_medics

keept: g_shuffle_rating
keept: g_maxTeamLandmines
keept: team_maxPanzers
keept: team_maxMortars
keept: team_maxFlamers
keept: team_maxMG42s
keept: team_maxGrenLaunchers
keept: g_mapConfigs
keept: g_packDistance
keept: g_censor
keept: g_censorNames
keept: g_censorPenalty
keept: g_censorMuteTime
keept: g_censorXP
keept: g_intermissionTime
keept: g_intermissionReadyPercent
keept: g_skills
keept: g_skipCorrection
keept: g_maxWarp
keept: g_teamDamageRestriction
keept: g_moverScale
keept: g_ammoCabinetTime
keept: g_healthCabinetTime
keept: g_truePing
keept: g_mapScriptDirectory
keept: g_campaignFile
keept: g_fear
keept: vote_allow_surrender
keept: vote_allow_nextcampaign
keept: vote_allow_restartcampaign
keept: vote_allow_poll
keept: vote_allow_maprestart
keept: vote_allow_shufflenorestart
keept: vote_allow_putspec

Vanilla Fixes:
============
fixed: TeamChatHeight in intermission chat
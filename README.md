Wolfenstein Enemy Territory Modification  
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

*Client Cvars:*  
  changed: cg_drawCompass  
  &emsp; *	2: draw a square compass  
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
    
*Server Cvars:*  
  added: g_realism (bitflag)  
  &emsp; *	1: jumping needs stamina  
  &emsp; *	2: slower health regeneration  
  &emsp; *	4: don't loose uniform while alive  
  &emsp; *	8: walk slower at low health (etpub: g_healthSpeed)  
  &emsp; *	16: damage based on where target was hit (etpub: g_dmg 1)  
  &emsp; *	32: damage over distance reduced faster and earlier (etpub: g_dmg 2)  
  added: g_hardcore  
  added: g_adminsysDir  
  added: g_noFallDamage  
  added: g_weapons (bitflag)  
  &emsp; *	256: pick up any weapon  
  &emsp; *	512: no weapon when holding objective  
  
*Commands:*  
  added: "dropweapon"  
  added: "/pm" & "/m"  
  
ETPub Features:
==============
  HUD Editor  
  Banners  
  Killingsprees  
  Multikills  
  Shrubbot  
  
*Server Cvars:*  
  changed: g_voting  
  changed: g_slashKill  
  changed: g_medics  
  
  kept: g_shuffle_rating  
  kept: g_maxTeamLandmines  
  kept: team_maxPanzers  
  kept: team_maxMortars  
  kept: team_maxFlamers  
  kept: team_maxMG42s  
  kept: team_maxGrenLaunchers  
  kept: g_mapConfigs  
  kept: g_packDistance  
  kept: g_censor  
  kept: g_censorNames  
  kept: g_censorPenalty  
  kept: g_censorMuteTime  
  kept: g_censorXP  
  kept: g_intermissionTime  
  kept: g_intermissionReadyPercent  
  kept: g_skills  
  kept: g_skipCorrection  
  kept: g_maxWarp  
  kept: g_teamDamageRestriction  
  kept: g_moverScale  
  kept: g_ammoCabinetTime  
  kept: g_healthCabinetTime  
  kept: g_truePing  
  kept: g_mapScriptDirectory  
  kept: g_campaignFile  
  kept: g_fear  
  kept: vote_allow_surrender  
  kept: vote_allow_nextcampaign  
  kept: vote_allow_restartcampaign  
  kept: vote_allow_poll  
  kept: vote_allow_maprestart  
  kept: vote_allow_shufflenorestart  
  kept: vote_allow_putspec  
  
Vanilla Fixes:
============
  fixed: TeamChatHeight in intermission chat  
  
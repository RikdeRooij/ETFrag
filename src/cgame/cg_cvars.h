#ifndef __CG_CVARS_H__
#define __CG_CVARS_H__

void GS_SetHudElement( char *elementName, int values, int newVals[6] ) {
	int i;

	if ( !Q_stricmp( elementName, "healthbar" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.healthbar[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "staminabar" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.staminabar[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "chargebar" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.chargebar[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "weaponicon" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.weaponicon[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "ammocount" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.ammo[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "compass" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.compass[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "healthtext" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.hp[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "xptext" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.xp[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "xpaddtext" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.xpadd[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "upperright" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.draws[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "lagometer" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.lagometer[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "skilltexts" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.skilltexts[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "skillboxes" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.skillboxes[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "fireteam" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.fireteam[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "head" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.head[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "flagcov" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.flagcov[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "popuptext" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.popuptext[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "chattext" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.chattext[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "votefttext" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.votefttext[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "livesleft" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.livesleft[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "centerprint" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.centerprint[i] = newVals[i];
		}
	}
	if ( !Q_stricmp( elementName, "killspree" ) ) {
		for( i=0; i < values; i++ ) {
			cg.hud.killspree[i] = newVals[i];
		}
	}
}


typedef struct {
	char	*elementName;
	int		values;
	int		*currentvalues;
} gs_hudTable_t;

gs_hudTable_t	gs_hudTable[] = {
	{ "healthbar", 4, cg.hud.healthbar },
	{ "staminabar", 4, cg.hud.staminabar },
	{ "chargebar", 4, cg.hud.chargebar },
	{ "weaponicon", 4, cg.hud.weaponicon },
	{ "ammocount", 3, cg.hud.ammo }, // ammo
	{ "compass", 3, cg.hud.compass },
	{ "healthtext", 3, cg.hud.hp }, // hp
	{ "xptext", 3, cg.hud.xp }, // xp
	{ "xpaddtext", 3, cg.hud.xpadd }, // xpadd
	{ "upperright", 1, cg.hud.draws }, // draws
	{ "lagometer", 2, cg.hud.lagometer },
	{ "skilltexts", 4, cg.hud.skilltexts },
	{ "skillboxes", 4, cg.hud.skillboxes },
	{ "fireteam", 3, cg.hud.fireteam },
	{ "head", 4, cg.hud.head },
	{ "flagcov", 2, cg.hud.flagcov },
	{ "popuptext", 2, cg.hud.popuptext },
	{ "chattext", 3, cg.hud.chattext },
	{ "votefttext", 2, cg.hud.votefttext },
	{ "livesleft", 3, cg.hud.livesleft },
	{ "centerprint", 1, cg.hud.centerprint },
	{ "killspree", 3, cg.hud.killspree },
	//{ "backgrounds", MAX_HUD_BGS, 4};
};

int	gs_hudTableSize = sizeof( gs_hudTable ) / sizeof( gs_hudTable[0] );

// ------------------------------------------------------

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	int			modificationCount;
} gs_cvarTable_t;

gs_cvarTable_t		gs_cvarTable[] = {
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_cursorHints, "cg_cursorHints", "1", CVAR_ARCHIVE },
	{ &cg_zoomDefaultSniper, "cg_zoomDefaultSniper", "20", CVAR_ARCHIVE }, // JPW NERVE changed per atvi req
	{ &cg_zoomStepSniper, "cg_zoomStepSniper", "2", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
	{ &cg_letterbox, "cg_letterbox", "0", CVAR_TEMP },	//----(SA)	added

	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE },
	{ &cg_drawSpreadScale, "cg_drawSpreadScale", "1", CVAR_ARCHIVE },

	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "1", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawCrosshairPickups, "cg_drawCrosshairPickups", "1", CVAR_ARCHIVE },

	{ &cg_crosshairSize, "cg_crosshairSize", "48", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "0", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },

	{ &cg_lagometer, "cg_lagometer", "0", CVAR_ARCHIVE },

	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },

	{ &cg_teamChatTime, "cg_teamChatTime", "8000", CVAR_ARCHIVE  },
	{ &cg_teamChatHeight, "cg_teamChatHeight", "8", CVAR_ARCHIVE  },

	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},						// NERVE - SMF
	{ &cg_voiceSpriteTime, "cg_voiceSpriteTime", "6000", CVAR_ARCHIVE},		// DHM - Nerve

	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE },				// NERVE - SMF
	{ &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },				// NERVE - SMF

	{ &cg_customVoiceChats, "cg_customVoiceChats", "1", CVAR_ARCHIVE }, // Elf
	// the following variables are created in other parts of the system,
	// but we also reference them here

//	{ &cg_animState, "cg_animState", "0", CVAR_CHEAT},			// UNUSED
	{ &cg_drawCompass, "cg_drawCompass", "1", CVAR_ARCHIVE },

	{ &cg_quickMessageAlt, "cg_quickMessageAlt", "0", CVAR_ARCHIVE },

	{ &cg_descriptiveText, "cg_descriptiveText", "1", CVAR_ARCHIVE },

	{ &cf_wstats, "cf_wstats", "1.2", CVAR_ARCHIVE },
	{ &cf_wtopshots, "cf_wtopshots", "1.0", CVAR_ARCHIVE },

	{ &cg_complaintPopUp, "cg_complaintPopUp", "1", CVAR_ARCHIVE },
	{ &cg_crosshairAlpha, "cg_crosshairAlpha", "1.0", CVAR_ARCHIVE },
	{ &cg_crosshairAlphaAlt, "cg_crosshairAlphaAlt", "1.0", CVAR_ARCHIVE },
	{ &cg_crosshairColor, "cg_crosshairColor", "White", CVAR_ARCHIVE },
	{ &cg_crosshairColorAlt, "cg_crosshairColorAlt", "White", CVAR_ARCHIVE },
	{ &cg_crosshairPulse, "cg_crosshairPulse", "1", CVAR_ARCHIVE },
	{ &cg_drawReinforcementTime, "cg_drawReinforcementTime", "1", CVAR_ARCHIVE },
	{ &cg_drawWeaponIconFlash, "cg_drawWeaponIconFlash", "2", CVAR_ARCHIVE },

	{ &cg_printObjectiveInfo, "cg_printObjectiveInfo", "1", CVAR_ARCHIVE },
	{ &cg_specHelp, "cg_specHelp", "1", CVAR_ARCHIVE },

	{ &demo_drawTimeScale, "demo_drawTimeScale", "1", CVAR_ARCHIVE },
	{ &demo_infoWindow, "demo_infoWindow", "1", CVAR_ARCHIVE },


	{ &cg_drawRoundTimer, "cg_drawRoundTimer", "1", CVAR_ARCHIVE },

	// tjw: not used
	//{ NULL,					"cg_etVersion",		"",		CVAR_USERINFO | CVAR_ROM },
	{ &cg_drawFireteamOverlay, "cg_drawFireteamOverlay", "1", CVAR_ARCHIVE },
	{ &cg_drawSmallPopupIcons, "cg_drawSmallPopupIcons", "0", CVAR_ARCHIVE },

	{ &cg_recording_statusline, "cg_recording_statusline", "9", CVAR_ARCHIVE },

	//matt
	{ &cg_drawTime, "cg_drawTime", "0", CVAR_ARCHIVE },
	{ &cg_drawTimeSeconds, "cg_drawTimeSeconds", "0", CVAR_ARCHIVE },
	//{ &cg_hud, "cg_hud", "", CVAR_ARCHIVE },

	{ &cg_mapzoom, "cg_mapzoom", "5.159", CVAR_ARCHIVE },
	{ &cg_obituary, "cg_obituary", "0", CVAR_ARCHIVE },

	// forty - speedometer
	{ &cg_drawspeed, "cg_drawspeed", "0", CVAR_ARCHIVE },
	{ &cg_speedinterval, "cg_speedinterval", "100", CVAR_ARCHIVE },
	{ &cg_speedunit, "cg_speedunit", "0", CVAR_ARCHIVE },

	// forty - opacity
	{ &cg_fireteamAlpha, "cg_fireteamAlpha", "0.6", CVAR_ARCHIVE},
	{ &cg_chatAlpha, "cg_chatAlpha", "0.33", CVAR_ARCHIVE},
	{ &cg_lagometerAlpha, "cg_lagometerAlpha", "1.0", CVAR_ARCHIVE},
	{ &cg_specAlpha, "cg_specAlpha", "1.0", CVAR_ARCHIVE},

	// tjw
	{ &cg_fontLimboScale, "cg_fontLimboScale", "0.2", CVAR_ARCHIVE },
	{ &cg_fontCPScale, "cg_fontCPScale", "0.2", CVAR_ARCHIVE },
	{ &cg_fontOIDScale, "cg_fontOIDScale", "0.2", CVAR_ARCHIVE },
	{ &cg_fontQAScale, "cg_fontQAScale", "0.2", CVAR_ARCHIVE },
	{ &cg_fontBPScale, "cg_fontBPScale", "0.25", CVAR_ARCHIVE },


	// tjw: ratings scoreboard
	{ &cg_scoreboard, "cg_scoreboard", "0", CVAR_ARCHIVE },

	// HOLLEMOD - END
	{ &cg_watermarkAlpha, "cg_watermarkAlpha", "1.0", CVAR_ARCHIVE},

	// forty - Phoenix - scale weapon for custom FOV settings 
	{ &cg_gun_fovscale, "cg_gun_fovscale", "1", 0	},

	{ &cg_drawRanks, "cg_drawRanks", "1", CVAR_ARCHIVE },
	{ &cg_dynoCounter, "cg_dynoCounter", "1", CVAR_ARCHIVE },
	{ &cg_drawClassIcons, "cg_drawClassIcons", "7", CVAR_ARCHIVE },
	{ &cg_countryFlags, "cg_countryFlags", "3", CVAR_ARCHIVE }, //mcwf GeoIP
	{ &cg_drawCrosshairHP, "cg_drawCrosshairHP", "0", CVAR_ARCHIVE },

	{ &cg_locations, "cg_locations", "1", CVAR_ARCHIVE }, // flms

// codePRK CVARs client
	{ &cg_drawObjectiveFP, "cg_drawObjectiveFP", "1", CVAR_ARCHIVE },
	{ &cg_drawFireteamIcons, "cg_drawFireteamIcons", "1", CVAR_ARCHIVE },
	{ &cg_drawPickupMsg, "cg_drawPickupMsg", "1", CVAR_ARCHIVE },
	{ &cg_popupDelay, "cg_popupDelay", "0", CVAR_ARCHIVE },
	{ &cg_shadowedPopups, "cg_shadowedPopups", "0", CVAR_ARCHIVE },
	{ &cg_numPopups, "cg_numPopups", "5", CVAR_ARCHIVE },
	{ &cg_hudColor, "cg_hudColor", ".16 .2 .17", CVAR_ARCHIVE },
	{ &cg_hudAlpha, "cg_hudAlpha", ".8", CVAR_ARCHIVE },

	{ &cg_visWeapSelectTime, "cg_visWeapSelectTime", "2000", CVAR_ARCHIVE },


};

int	gs_cvarTableSize = sizeof( gs_cvarTable ) / sizeof( gs_cvarTable[0] );

#endif
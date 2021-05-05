#include "cg_local.h"
#include "cg_cvars.h"

qboolean CG_LoadHud(char *filename)
{
	int handle;
	char hudFilename[MAX_TOKEN_CHARS];
	char profile[MAX_CVAR_VALUE_STRING];
	pc_token_t token;


	memset(&cg.hud, 0, sizeof(cg.hud));

	if(!Q_stricmp(filename, "blank")) {
		CG_BlankHud();
		return qtrue;
	}
	Q_strncpyz(hudFilename, va("hud/%s.hud", filename), MAX_TOKEN_CHARS);

	// forty - trap_PC_LoadSource is not what we want to be using...
	handle = trap_PC_LoadSource( hudFilename );
	if (!handle) {
		//forty - try from the profile directory
		trap_Cvar_VariableStringBuffer("cl_profile", profile, sizeof(profile));
		Q_strncpyz(hudFilename, va("profiles/%s/hud/%s.hud", profile, filename), MAX_TOKEN_CHARS);
		handle = trap_PC_LoadSource( hudFilename );

		if(!handle) {
			//CG_LoadDefaultHud();
			return qfalse;
		}
	}

	CG_Printf("Loading hud from '%s'\n", hudFilename);

//	CG_LoadDefaultHud();
	CG_BlankHud(); // clear


	while ( 1 ) {
		// hud elements
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( !Q_stricmp( token.string, "elements") ) {
			// we need a bracket
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				return qfalse;
			}

			if ( !Q_stricmp( token.string, "{" ) ) {
				while ( 1 ) {
					gs_hudTable_t	*hv;
					int i;
					if ( !trap_PC_ReadToken( handle, &token ) ) {
						break;
					}

					// we've reached the end
					if ( !Q_stricmp( token.string, "}" ) ) {
						break;
					}
					//if ( !Q_stricmp ( token.string, "compass" ) ) {
					// x y size
					//	if ( !PC_Int_Parse( handle, &cg.hud.compass[0] ) )
					//		continue;
					//	if ( !PC_Int_Parse( handle, &cg.hud.compass[1] ) )
					//		continue;
					//	if ( !PC_Int_Parse( handle, &cg.hud.compass[2] ) )
					//		continue;
					//}

					for ( i = 0, hv = gs_hudTable ; i < gs_hudTableSize ; i++, hv++ ) {

						if ( !Q_stricmp( token.string, hv->elementName ) ) {
							int		count;
							char	*nextVal;
							int		newVals[6];

							// next should be the new element values
							if ( !trap_PC_ReadToken( handle, &token ) ) {
								break;
							}

							nextVal = strtok( token.string, " ");

							for (count = 0; count < hv->values && nextVal; count++) 
							{
								newVals[count] = atof(nextVal);
								nextVal		   = strtok(NULL, " ,");
							}

							GS_SetHudElement( hv->elementName, hv->values, newVals );

							//CG_Printf( va("^3%s ^5%s\n", hv->elementName, tempbuf ) );
							//trap_Cvar_Set( va("%s", cv->cvarName), va("%s", tempbuf) );

						}
					}
				}
			}
		}
		// -------------------------
		else if ( !Q_stricmp( token.string, "backgrounds") ) {
			int i;

			if ( !trap_PC_ReadToken( handle, &token ) ) {
				return qfalse;
			}

			// we need a bracket
			if ( !Q_stricmp( token.string, "{" ) ) {
				while ( 1 ) {
					if ( !trap_PC_ReadToken( handle, &token ) ) {
						break;
					}

					// we've reached the end
					if ( !Q_stricmp( token.string, "}" ) ) {
						break;
					}

					for( i=0; i<MAX_HUD_BGS; i++ ) {
						char *bg = (va("background%d", i+1));
						if ( !Q_stricmp ( token.string, bg ) ) {
							int		count;
							char	*nextVal;
							int		newVals[4];
							int j;

							// next should be the new values
							if ( !trap_PC_ReadToken( handle, &token ) ) {
								break;
							}

							nextVal = strtok( token.string, " ");

							for (count = 0; count < 4 && nextVal; count++) 
							{
								newVals[count] = atof(nextVal);
								nextVal		   = strtok(NULL, " ,");
							}

							for( j=0; j < 4; j++ ) {
								cg.hud.backgrounds[i][j] = newVals[j];
							}
						}
					}
				}
			}
		}
		// -------------------------
		
		else if ( !Q_stricmp( token.string, "settings") ) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				return qfalse;
			}

			// we need a bracket
			if ( !Q_stricmp( token.string, "{" ) ) {
				while ( 1 ) {
					gs_cvarTable_t	*cv;
					int k;

					if ( !trap_PC_ReadToken( handle, &token ) ) {
						break;
					}

					// we've reached the end
					if ( !Q_stricmp( token.string, "}" ) ) {
						break;
					}

					for ( k = 0, cv = gs_cvarTable ; k < gs_cvarTableSize ; k++, cv++ ) {

						if ( !Q_stricmp( token.string, cv->cvarName ) ) { // it's a cvar
							//const char *tempbuf;
							//if ( !PC_String_Parse( handle, &tempbuf ) )
							//	continue;
							char tempbuf[64];
							if( !PC_String_ParseNoAlloc( handle, tempbuf, sizeof(tempbuf) ) )
								continue;

							CG_Printf( va("^3%s ^5%s\n", cv->cvarName, tempbuf ) );
							trap_Cvar_Set( va("%s", cv->cvarName), va("%s", tempbuf) );

						}
					}
				}
			}
		}		
	// -------------------------
	} // while 1

	trap_PC_FreeSource(handle);
	memset(&cg.hudRect, 0, sizeof(hudRect_t));
	return qtrue;
}


// Create the etmain HUD
qboolean CG_LoadDefaultHud()
{
	cg.hud.weaponicon[0] = 558;
	cg.hud.weaponicon[1] = 430; //424;
	cg.hud.weaponicon[2] = 60;
	cg.hud.weaponicon[3] = 32;
	cg.hud.ammo[0] = 618;
	cg.hud.ammo[1] = 476; //470;
	cg.hud.ammo[2] = 25;
	cg.hud.chattext[0] = 164; // 160
	cg.hud.chattext[1] = 478;
	cg.hud.chattext[2] = (640-160-100); // 20;
	cg.hud.compass[0] = 504;
	cg.hud.compass[1] = 4;
	cg.hud.compass[2] = 132;
	cg.hud.popuptext[0] = 4;
	cg.hud.popuptext[1] = 378; // 360
	cg.hud.draws[0] = 140;
	cg.hud.fireteam[0] = 10;
	cg.hud.fireteam[1] = 10;
	cg.hud.fireteam[2] = 100;
	cg.hud.flagcov[0] = 600;
	cg.hud.flagcov[1] = 340;
	cg.hud.head[0] = 44;
	cg.hud.head[1] = 388;
	cg.hud.head[2] = 62;
	cg.hud.head[3] = 80;
	cg.hud.hp[0] = 52; // 84
	cg.hud.hp[1] = 476;
	cg.hud.hp[2] = 25;
	cg.hud.healthbar[0] = 24;
	cg.hud.healthbar[1] = 388;
	cg.hud.healthbar[2] = 12;
	cg.hud.healthbar[3] = 72;
	cg.hud.staminabar[0] = 4;
	cg.hud.staminabar[1] = 388;
	cg.hud.staminabar[2] = 12;
	cg.hud.staminabar[3] = 72;
	cg.hud.chargebar[0] = 624;
	cg.hud.chargebar[1] = 388;
	cg.hud.chargebar[2] = 12;
	cg.hud.chargebar[3] = 72;
	cg.hud.lagometer[0] = 590;
	cg.hud.lagometer[1] = 280;
	cg.hud.livesleft[0] = 4;
	cg.hud.livesleft[1] = 360;
	cg.hud.livesleft[2] = 14;
	cg.hud.skillboxes[0] = 112;
	cg.hud.skillboxes[1] = 386;
	cg.hud.skillboxes[2] = 16*3;
	cg.hud.skillboxes[3] = 15*4;
	cg.hud.skilltexts[0] = -1;
	cg.hud.skilltexts[1] = -1;
	cg.hud.skilltexts[2] = -1;
	cg.hud.skilltexts[3] = -1;
	cg.hud.votefttext[0] = 8;
	cg.hud.votefttext[1] = 200;
	cg.hud.xp[0] = 100;
	cg.hud.xp[1] = 476;
	cg.hud.xp[2] = 25;
	cg.hud.xpadd[0] = -1;
	cg.hud.xpadd[1] = -1;
	cg.hud.xpadd[2] = -1;
	cg.hud.killspree[0] = 222;
	cg.hud.killspree[1] = 100;
	cg.hud.killspree[2] = 25;
	cg.hud.centerprint[0] = (480*0.75); // SCREEN_HEIGHT * 0.75 = 360

	trap_Cvar_Set("cg_hudColor", "def" );
	trap_Cvar_Set("cg_teamChatHeight", "8" );
	return qtrue;
}

qboolean CG_AltHud1()
{
	cg.hud.chattext[0] = 148;
	cg.hud.chattext[1] = 478;
	cg.hud.chattext[2] = 308; //(640-332);
	cg.hud.draws[0] = 120;
	cg.hud.fireteam[0] = 6;
	cg.hud.fireteam[1] = 6;
	cg.hud.fireteam[2] = 100;
	cg.hud.head[0] = -1; // no head
	cg.hud.weaponicon[0] = 542;
	cg.hud.weaponicon[1] = 436;
	cg.hud.weaponicon[2] = 60;
	cg.hud.weaponicon[3] = 32;
	cg.hud.ammo[0] = 633;
	cg.hud.ammo[1] = 473;
	cg.hud.ammo[2] = 20;
	cg.hud.healthbar[0] = 6;
	cg.hud.healthbar[1] = 430;
	cg.hud.healthbar[2] = 133;
	cg.hud.healthbar[3] = 13;
	cg.hud.staminabar[0] = 6;
	cg.hud.staminabar[1] = 445;
	cg.hud.staminabar[2] = 133;
	cg.hud.staminabar[3] = 13;
	cg.hud.chargebar[0] = 6;
	cg.hud.chargebar[1] = 460;
	cg.hud.chargebar[2] = 133;
	cg.hud.chargebar[3] = 13;
	cg.hud.lagometer[0] = 590;
	cg.hud.lagometer[1] = 280;
	cg.hud.popuptext[0] = 4;
	cg.hud.popuptext[1] = 422;
	cg.hud.centerprint[0] = 360; //(480*0.75);
	cg.hud.skillboxes[0] = 478;
	cg.hud.skillboxes[1] = 430; //386;
	cg.hud.skillboxes[2] = 14*4;
	cg.hud.skillboxes[3] = 15*3;
	cg.hud.skilltexts[0] = -1;
	cg.hud.xp[0] = 590; //620;
	cg.hud.xp[1] = 440;
	cg.hud.xp[2] = 18; //15;
	cg.hud.xpadd[0] = 599; // 594
	cg.hud.xpadd[1] = 450;
	cg.hud.xpadd[2] = 14; // 16
	cg.hud.hp[0] = 62;
	cg.hud.hp[1] = 440;
	cg.hud.hp[2] = 20;
	cg.hud.flagcov[0] = 596; //600;
	cg.hud.flagcov[1] = 382; //340;
	cg.hud.compass[0] = 528;
	cg.hud.compass[1] = 8;
	cg.hud.compass[2] = 104;
	cg.hud.votefttext[0] = 8;
	cg.hud.votefttext[1] = 210;
	cg.hud.livesleft[0] = 4;
	cg.hud.livesleft[1] = 402; //360;
	cg.hud.livesleft[2] = 12;
	cg.hud.killspree[0] = 8;
	cg.hud.killspree[1] = 165;
	cg.hud.killspree[2] = 25;

	// left bg
	cg.hud.backgrounds[0][0] = 4;
	cg.hud.backgrounds[0][1] = (480 - 53);
	cg.hud.backgrounds[0][2] = 140;
	cg.hud.backgrounds[0][3] = 50;

	// right bg
	cg.hud.backgrounds[1][0] = (640 - 180);
	cg.hud.backgrounds[1][1] = (480 - 53);
	cg.hud.backgrounds[1][2] = (180 - 4);
	cg.hud.backgrounds[1][3] = 49;

	trap_Cvar_Set("cg_hudColor", "0.1 0.1 0.1" );
	trap_Cvar_Set("cg_teamChatHeight", "6" );

	return qtrue;
}

qboolean CG_AltHud2()
{
	cg.hud.chattext[0] = 148;
	cg.hud.chattext[1] = 478;
	cg.hud.chattext[2] = 392;
	cg.hud.draws[0] = 140;
	cg.hud.fireteam[0] = 430;
	cg.hud.fireteam[1] = 6;
	cg.hud.fireteam[2] = 100;
	cg.hud.healthbar[0] = 20;
	cg.hud.healthbar[1] = 376;
	cg.hud.healthbar[2] = 12;
	cg.hud.healthbar[3] = 72;
	cg.hud.staminabar[0] = 4;
	cg.hud.staminabar[1] = 376;
	cg.hud.staminabar[2] = 12;
	cg.hud.staminabar[3] = 72;
	cg.hud.chargebar[0] = 624;
	cg.hud.chargebar[1] = 376;
	cg.hud.chargebar[2] = 12;
	cg.hud.chargebar[3] = 72;
	cg.hud.lagometer[0] = 590;
	cg.hud.lagometer[1] = 280;
	cg.hud.popuptext[0] = 4;
	cg.hud.popuptext[1] = 372;
	cg.hud.centerprint[0] = 360; //(480*0.75)
	cg.hud.skillboxes[0] = -1;
	cg.hud.skillboxes[1] = -1;
	cg.hud.skillboxes[2] = -1;
	cg.hud.skillboxes[3] = -1;
	cg.hud.skilltexts[0] = 36;
	cg.hud.skilltexts[1] = 376;
	cg.hud.skilltexts[2] = 30;
	cg.hud.skilltexts[3] = 40;
	cg.hud.xp[0] = 94;
	cg.hud.xp[1] = 476;
	cg.hud.xp[2] = 20;
	cg.hud.xpadd[0] = -1;
	cg.hud.xpadd[1] = -1;
	cg.hud.xpadd[2] = -1;
	cg.hud.hp[0] = 2;
	cg.hud.hp[1] = 477;
	cg.hud.hp[2] = 20;
	cg.hud.flagcov[0] = 584; //600;
	cg.hud.flagcov[1] = 394; //340;
	cg.hud.compass[0] = 50;
	cg.hud.compass[1] = 370;
	cg.hud.compass[2] = 94;
	cg.hud.votefttext[0] = 8;
	cg.hud.votefttext[1] = 200;
	cg.hud.livesleft[0] = 4;
	cg.hud.livesleft[1] = 350;
	cg.hud.livesleft[2] = 12;
	cg.hud.weaponicon[0] = 561;
	cg.hud.weaponicon[1] = 434;
	cg.hud.weaponicon[2] = 60;
	cg.hud.weaponicon[3] = 32;
	cg.hud.ammo[0] = 636;
	cg.hud.ammo[1] = 478;
	cg.hud.ammo[2] = 20;

	cg.hud.killspree[0] = 8;
	cg.hud.killspree[1] = 8;
	cg.hud.killspree[2] = 25;

	trap_Cvar_Set("cg_hudColor", "def" );
	trap_Cvar_Set("cg_teamChatHeight", "8" );

	return qtrue;
}


qboolean CG_BlankHud()
{
	int i, j;

	for( i=0; i<4; i++ ) {
		cg.hud.weaponicon[i] = -1;
		cg.hud.head[i] = -1;
		cg.hud.healthbar[i] = -1;
		cg.hud.staminabar[i] = -1;
		cg.hud.chargebar[i] = -1;
		cg.hud.skilltexts[i] = -1;
		cg.hud.skillboxes[i] = -1;
	}

	for( i=0; i<3; i++ ) {
		cg.hud.ammo[i] = -1;
		cg.hud.chattext[i] = -1;
		cg.hud.compass[i] = -1;
		cg.hud.fireteam[i] = -1;
		cg.hud.hp[i] = -1;
		cg.hud.livesleft[i] = -1;
		cg.hud.xp[i] = -1;
		cg.hud.xpadd[i] = -1;
		cg.hud.killspree[i] = -1;
	}

	for( i=0; i<2; i++ ) {
		cg.hud.flagcov[i] = -1;
		cg.hud.lagometer[i] = -1;
		cg.hud.votefttext[i] = -1;
		cg.hud.popuptext[i] = -1;
	}


	cg.hud.draws[0] = -1;
	cg.hud.centerprint[0] = -1;


	for( i=0; i<MAX_HUD_BGS; i++ ) {
		for( j=0; j<4; j++ ) {
			cg.hud.backgrounds[i][j] = -1;
		}
	}

	trap_Cvar_Set("cg_hudColor", "def" );
	trap_Cvar_Set("cg_teamChatHeight", "8" );

	return qtrue;
}


void CG_SaveHud(char *filename) {
	char hudPath[256];
	
	char buffer[1024];
	fileHandle_t handle;
	char profile[MAX_CVAR_VALUE_STRING];
	int i;
	int writebg = 0;

	trap_Cvar_VariableStringBuffer("cl_profile", profile, sizeof(profile));

	Com_sprintf(hudPath, sizeof(hudPath), "hud/%s.hud", filename);

	CG_Printf("Saving hud to '%s'\n", hudPath);

	if(trap_FS_FOpenFile(hudPath, &handle, FS_WRITE) < 0) {
		trap_FS_Write(buffer, strlen(buffer), handle);
		CG_Printf("Unable to open: %s for writing\n", hudPath);
		return;
	} 

	// forty - update the cvar so this hud will be reloaded on map changes...
	//trap_Cvar_Set("cg_hud", filename);
	// moved this down

	Com_sprintf(buffer, sizeof(buffer), "elements {\n");
	trap_FS_Write(buffer, strlen(buffer), handle);
	
	if(1) {
		gs_hudTable_t	*hv;
		int j, k;

		for ( j = 0, hv = gs_hudTable ; j < gs_hudTableSize ; j++, hv++ ) {

			// __<element>_"
			Com_sprintf(buffer, sizeof(buffer), "  %s \"", 
				hv->elementName);
			trap_FS_Write(buffer, strlen(buffer), handle);

			// [i] [i] ...

			for( k=0; k < hv->values; k++ ) {
				
				if( k == 0 ) {
					Com_sprintf(buffer, sizeof(buffer), "%d", 
						hv->currentvalues[k] );
				} else {
					Com_sprintf(buffer, sizeof(buffer), " %d", 
						hv->currentvalues[k] );
				}

				trap_FS_Write(buffer, strlen(buffer), handle);
			}

			Com_sprintf( buffer, sizeof(buffer), "\"\n" );

			trap_FS_Write(buffer, strlen(buffer), handle);

		}

	}

	Com_sprintf(buffer, sizeof(buffer), "}\n");
	trap_FS_Write(buffer, strlen(buffer), handle);

	// newline
	Com_sprintf(buffer, sizeof(buffer), "\n");
	trap_FS_Write(buffer, strlen(buffer), handle);

	// ---------------
	
	for( i=0; i<MAX_HUD_BGS; i++ ) {
		// check if there are any
		if( cg.hud.backgrounds[i][0] > 0 ) {
			writebg = 1;
		}
	}

	if( writebg ) { // there are so write them
		Com_sprintf(buffer, sizeof(buffer), "backgrounds {\n");
		trap_FS_Write(buffer, strlen(buffer), handle);

		for( i=0; i<MAX_HUD_BGS; i++ ) {

			if( cg.hud.backgrounds[i][0] > 0 ) {
				Com_sprintf(buffer, sizeof(buffer), "  background%d \"%d %d %d %d\"\n",
				(i + 1), cg.hud.backgrounds[i][0], cg.hud.backgrounds[i][1],
				   cg.hud.backgrounds[i][2], cg.hud.backgrounds[i][3] );
				trap_FS_Write(buffer, strlen(buffer), handle);
			}
		}

		Com_sprintf(buffer, sizeof(buffer), "}\n");
		trap_FS_Write(buffer, strlen(buffer), handle);

	// newline
	Com_sprintf(buffer, sizeof(buffer), "\n");
	trap_FS_Write(buffer, strlen(buffer), handle);

	}

	// ---------------

	Com_sprintf(buffer, sizeof(buffer), "settings {\n");
	trap_FS_Write(buffer, strlen(buffer), handle);

		Com_sprintf(buffer, sizeof(buffer), "  cg_hudColor \"%f %f %f\"\n",
		//(int)(HUD_Color[0] * 100), (int)(HUD_Color[1] * 100), (int)(HUD_Color[2] * 100) );
		(float)HUD_Color[0], (float)HUD_Color[1], (float)HUD_Color[2] );
		trap_FS_Write(buffer, strlen(buffer), handle);

		Com_sprintf(buffer, sizeof(buffer), "  cg_teamChatHeight \"%d\"\n",
		cg_teamChatHeight.integer );
		trap_FS_Write(buffer, strlen(buffer), handle);

	Com_sprintf(buffer, sizeof(buffer), "}\n");
	trap_FS_Write(buffer, strlen(buffer), handle);

	// newline
	Com_sprintf(buffer, sizeof(buffer), "\n");
	trap_FS_Write(buffer, strlen(buffer), handle);

	// ---------------

	trap_FS_FCloseFile(handle);

	// forty - update the cvar so this hud will be reloaded on map changes...
	if( cg_hud.string == filename ) { // it's already this name
		CG_UpdateHud(); // changing cvar will eventually call this
	} else {
		trap_Cvar_Set("cg_hud", filename);
	}

	//return;
}

void CG_SaveHud_f(void) {
	char filename[256];

	if ( trap_Argc() < 2 ) {
		CG_Printf("^1usage: /savehud [hudname]\n");
		return;
	}

	trap_Argv(1, filename, sizeof(filename));

	CG_SaveHud(filename);
}

void CG_CancelHudMenu_f(void) {
	trap_Cvar_Set("savehudmenu", "");
	trap_Cvar_Set("loadhudmenu", "");
	// re-enter the hudeditor
	CG_HudEditor_f();
}

void CG_SaveHudMenu_f(void) {
	char filename[MAX_CVAR_VALUE_STRING];

	trap_Cvar_VariableStringBuffer("savehudmenu", filename, sizeof(filename));
	CG_SaveHud(filename);
	// re-enter the hudeditor
	CG_HudEditor_f();
}

void CG_LoadHudMenu_f(void) {
	char filename[MAX_CVAR_VALUE_STRING];

	trap_Cvar_VariableStringBuffer("loadhudmenu", filename, sizeof(filename));
	CG_LoadHud(filename);
	// re-enter the hudeditor
	CG_HudEditor_f();
}
 
// forty - toggle visual hud editor
void CG_HudEditor_f(void) {
	int milli;
	if( cg.hudEditor.showHudEditor ) {
		CG_EventHandling( CGAME_EVENT_NONE, qfalse );

		milli = trap_Milliseconds();

		if(milli < cg.fadeTime) {
			cg.fadeTime = 2 * milli + STATS_FADE_TIME - cg.fadeTime;
			cg.hudEditor.fadeTime = 2 * milli + STATS_FADE_TIME - cg.hudEditor.fadeTime;
		} else {
			cg.fadeTime = milli + STATS_FADE_TIME;
			cg.hudEditor.fadeTime = milli + STATS_FADE_TIME;
		}

		cg.hudEditor.showHudEditorHelp = SHOW_SHUTDOWN;

		//make it go away...
		cgs.voteTime = 0;

		if(!cg.hudEditor.saving) {
			// forty - automagically save the hud so unsaved changes stick on level changes...
			CG_SaveHud("autosave");
		}
	} else if(cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR && !cg.cameraMode) {
		CG_EventHandling( CGAME_EVENT_HUDEDITOR, qfalse );

		memset(&cg.hudRect, 0, sizeof(hudRect_t));

		milli = trap_Milliseconds();

		if(milli < cg.fadeTime) {
			cg.fadeTime = 2 * milli + STATS_FADE_TIME - cg.fadeTime;
			cg.hudEditor.fadeTime = 2 * milli + STATS_FADE_TIME - cg.hudEditor.fadeTime;
		} else {
			cg.fadeTime = milli + STATS_FADE_TIME;
			cg.hudEditor.fadeTime = milli + STATS_FADE_TIME;
		}
			
		cg.hudEditor.showHudEditorHelp = SHOW_ON;
		cg.hudEditor.hudScaleMode = HUD_SCALE_XY;
		cg.hudEditor.chatTime = 0;
		cg.hudEditor.popupTime = 0;
		cg.hudEditor.ksTime = 0;
		cg.hudEditor.showCoor = qtrue;

	} else {
		CG_PriorityCenterPrint("^3Can't load hud editor as spectator.", CP_HIGHERHEIGHT, BIGCHAR_WIDTH * 0.6, 1  );
	}
}

// forty - modified from BG_RectContainsPoint
static qboolean Hudrect_ContainsPoint(HudrectDef_t *rect, float x, float y) {
	if (rect) {
		if (x > rect->x && x < rect->x + rect->w && y > rect->y && y < rect->y + rect->h) {
			return qtrue;
		}
	}
	return qfalse;
}

// forty - handle the mouse over/hovering and dragging.
void CG_HudEditor_MouseHandling( int x, int y ) {
	int i;
	HudrectDef_t *testrect;

	if (!cg.hudEditor.showHudEditor)
		return;

	testrect = &cg.hudRect.healthbar;
	for(i=0; i<sizeof(hudRect_t)/sizeof(HudrectDef_t); i++) {

		// forty - mouse over effect
		if(Hudrect_ContainsPoint(testrect, cgDC.cursorx, cgDC.cursory)) {
			testrect->hover = qtrue;
		} else if(!testrect->grouped) {
				testrect->hover = qfalse;
		}

		// forty - if we're dragging setup the boxes with new coordinates
		//         the actual hud elements will be updated in the draw function.
		if(
			(testrect->grouped || testrect->dragged) 
		) {
			// forty - don't move until we are done grouping or are not grouping.
			if(!cg.hudEditor.grouping) {
				testrect->x += cgDC.cursorx - testrect->draggedx;
				testrect->y += cgDC.cursory - testrect->draggedy;
			}
			// forty - keep these in line no matter what. 
			testrect->draggedx = cgDC.cursorx;
			testrect->draggedy = cgDC.cursory;

			// don't let them escape....
			if(testrect->x < 0)
				testrect->x = 0;
			if(testrect->x > 640)
				testrect->x = 640;
			if((testrect->x + testrect->w) > 640 )
				testrect->x -= (testrect->x + testrect->w) - 640;
			if(testrect->y < 0)
				testrect->y = 0;
			if(testrect->y > 480)
				testrect->y = 480;
			if((testrect->y + testrect->h) > 480 ) {
				testrect->y -= (testrect->y + testrect->h) - 480;
			}
		}

		// forty - scale the boxes 
		//		   the actual hud elements will be updated in the draw function.
		switch(testrect->scaled) {
			case HUD_SCALE_NONE:
				break;

			case HUD_SCALE_XY:
				testrect->w += cgDC.cursorx - testrect->scaledx;
				testrect->h += cgDC.cursory - testrect->scaledy;
				testrect->scaledx = cgDC.cursorx;
				testrect->scaledy = cgDC.cursory;
				break;

			case HUD_SCALE_X:
				testrect->w += cgDC.cursorx - testrect->scaledx;
				testrect->scaledx = cgDC.cursorx;
				testrect->scaledy = cgDC.cursory;
				break;

			case HUD_SCALE_Y:
				testrect->h += cgDC.cursory - testrect->scaledy;
				testrect->scaledx = cgDC.cursorx;
				testrect->scaledy = cgDC.cursory;
				break;

			// forty - Mouse wheel uniform scaling is handled in the keyhandler.
			
			default: 
				break;
		}

		if(testrect->w <= 2)
			testrect->w = 2;
		if(testrect->h <= 2)
			testrect->h = 2;
	
		testrect++;
	}
}

// forty - invert the sign bit please.
#define FLIPSIGN(x) x = -x

// forty - visual hud editor key handler and drag drop
void CG_HudEditor_KeyHandling( int _key, qboolean down ) {
	int i, milli, key;
	HudrectDef_t *testrect;

	key = _key;

	switch(key) {
		// forty - element drag and drop and group
		case K_MOUSE1:
			testrect = &cg.hudRect.healthbar;
			for(i=0; i<sizeof(hudRect_t)/sizeof(HudrectDef_t); i++) {
				if(down) {
					if(Hudrect_ContainsPoint(testrect, cgDC.cursorx, cgDC.cursory)) {
                        if(!testrect->dragged) {
							testrect->dragged = qtrue;
							testrect->draggedx = cgDC.cursorx;
							testrect->draggedy = cgDC.cursory;
						}
						if(testrect->grouped)
							testrect->grouped = qfalse;
						if(cg.hudEditor.grouping)
							testrect->grouped = qtrue;
					} else if(!cg.hudEditor.grouping && testrect->grouped) {
							testrect->grouped = qfalse;
					}
					// forty - movement is handled in the mouse handler...
				} else { 
					if(testrect->dragged && !cg.hudEditor.grouping && !testrect->grouped) {
						testrect->dragged = qfalse;
						testrect->draggedx = -1;
						testrect->draggedy = -1; 
					}
				}
				testrect++;
			}
			break;

		// forty - element scale
		case K_MOUSE2:
			testrect = &cg.hudRect.healthbar;
			for(i=0; i<sizeof(hudRect_t)/sizeof(HudrectDef_t); i++) {
				if(down) {
					if(Hudrect_ContainsPoint(testrect, cgDC.cursorx, cgDC.cursory)) {
						testrect->scaled = cg.hudEditor.hudScaleMode;
						testrect->scaledx = cgDC.cursorx;
						testrect->scaledy = cgDC.cursory;
					} 
					// forty - mouse scaling is handled in the mouse handler...
				} else { 
					if(testrect->scaled) {
						testrect->scaled = HUD_SCALE_NONE;
						testrect->scaledx = -1;
						testrect->scaledy = -1; 
					}
				}
				testrect++;
			}
			break;

		//forty - element mouse wheel uniform scale
		case K_MWHEELUP:
			testrect = &cg.hudRect.healthbar;
			for(i=0; i<sizeof(hudRect_t)/sizeof(HudrectDef_t); i++) {
				if(down) {
					if(Hudrect_ContainsPoint(testrect, cgDC.cursorx, cgDC.cursory)) {
						testrect->scaled = HUD_SCALE_UNIFORM_UP;
						testrect->scaledx = -1;
						testrect->scaledy = -1;
						testrect->w += 2; 
						testrect->h += 2;
					} 
				} 
				testrect++;
			}
			break;
		case K_MWHEELDOWN:
			testrect = &cg.hudRect.healthbar;
			for(i=0; i<sizeof(hudRect_t)/sizeof(HudrectDef_t); i++) {
				if(down) {
					if(Hudrect_ContainsPoint(testrect, cgDC.cursorx, cgDC.cursory)) {
						testrect->scaled = HUD_SCALE_UNIFORM_DOWN;
						testrect->scaledx = -1;
						testrect->scaledy = -1;
						testrect->w -= 2; 
						testrect->h -= 2;
					} 
				} 
				testrect++;
			}
			break;

		// forty - toggle the help message
		case K_BACKSPACE:
			if(!down)
				break;
			if(cg.hudEditor.showHudEditorHelp == SHOW_OFF) {
				milli = trap_Milliseconds();

				if(milli < cg.fadeTime) {
					cg.fadeTime = 2 * milli + STATS_FADE_TIME - cg.fadeTime;
				} else {
					cg.fadeTime = milli + STATS_FADE_TIME;
				}

				cg.hudEditor.showHudEditorHelp = SHOW_ON;

			} else if (cg.hudEditor.showHudEditorHelp == SHOW_ON) {
				milli = trap_Milliseconds();

				if(milli < cg.fadeTime) {
					cg.fadeTime = 2 * milli + STATS_FADE_TIME - cg.fadeTime;
				} else {
					cg.fadeTime = milli + STATS_FADE_TIME;
				}

				cg.hudEditor.showHudEditorHelp = SHOW_SHUTDOWN;
			} 
			break;

		// forty - toggle individual elements
		case K_TAB:
			if(!down)
				break;
			FLIPSIGN(cg.hud.healthbar[0]);
			FLIPSIGN(cg.hudRect.healthbar.x);
			if(cg.hud.healthbar[2] <= 0) {
				cg.hud.healthbar[0] = 1;
				cg.hud.healthbar[1] = 1;
				cg.hud.healthbar[2] = 12;
				cg.hud.healthbar[3] = 72;
				cg.hudRect.healthbar.initiazlied = qfalse;
			}
			break;
		case 'q':
		case 'Q':
			if(!down)
				break;
			FLIPSIGN(cg.hud.staminabar[0]);
			FLIPSIGN(cg.hudRect.staminabar.x);
			if(cg.hud.staminabar[2] <= 0) {
				cg.hud.staminabar[0] = 1;
				cg.hud.staminabar[1] = 1;
				cg.hud.staminabar[2] = 12;
				cg.hud.staminabar[3] = 72;
				cg.hudRect.staminabar.initiazlied = qfalse;
			}
			break;
		case 'w':
		case 'W':
			if(!down)
				break;
			FLIPSIGN(cg.hud.chargebar[0]);
			FLIPSIGN(cg.hudRect.chargebar.x);
			if(cg.hud.chargebar[2] <= 0) {
				cg.hud.chargebar[0] = 1;
				cg.hud.chargebar[1] = 1;
				cg.hud.chargebar[2] = 12;
				cg.hud.chargebar[3] = 72;
				cg.hudRect.chargebar.initiazlied = qfalse;
			}
			break;
		case 'e':
		case 'E':
			if(!down)
				break;
			FLIPSIGN(cg.hud.compass[0]);
			FLIPSIGN(cg.hudRect.compass.x);
			if(cg.hud.compass[2] <= 0) {
				cg.hud.compass[0] = 1;
				cg.hud.compass[1] = 1;
				cg.hud.compass[2] = 132;
				cg.hudRect.compass.initiazlied = qfalse;
			}
			break;
		case 'r':
		case 'R':		
			if(!down)
				break;
			FLIPSIGN(cg.hud.hp[0]);
			FLIPSIGN(cg.hudRect.hp.x);
			if(cg.hud.hp[2] <= 0) {
				cg.hud.hp[0] = 60;
				cg.hud.hp[1] = 20;
				cg.hud.hp[2] = 25;
				cg.hudRect.hp.initiazlied = qfalse;
			}
			break;
		case 't':
		case 'T':
			if(!down)
				break;
			FLIPSIGN(cg.hud.xp[0]);
			FLIPSIGN(cg.hudRect.xp.x);
			if(cg.hud.xp[2] == 0) {
				cg.hud.xp[0] = 60;
				cg.hud.xp[1] = 50;
				cg.hud.xp[2] = 25;
				cg.hudRect.xp.initiazlied = qfalse;
			}
			break;
		case 'y':
		case 'Y':
			if(!down)
				break;
			FLIPSIGN(cg.hud.xpadd[0]);
			FLIPSIGN(cg.hudRect.xpadd.x);
			if(cg.hud.xpadd[2] == 0) {
				cg.hud.xpadd[0] = 60;
				cg.hud.xpadd[1] = 50;
				cg.hud.xpadd[2] = 25;
				cg.hudRect.xpadd.initiazlied = qfalse;
			}
			break;
		case 'U':
		case 'u':
			if(!down)
				break;
			FLIPSIGN(cg.hud.draws[0]);
			FLIPSIGN(cg.hudRect.draws.x);
			cg.hudRect.draws.initiazlied = qfalse;
			break;
		case 'I':
		case 'i':
			if(!down)
				break;
			FLIPSIGN(cg.hud.skilltexts[0]);
			FLIPSIGN(cg.hudRect.skilltexts.x);
			if(cg.hud.skilltexts[1] <= 0) {
				cg.hud.skilltexts[0] = 1;
                cg.hud.skilltexts[1] = 17;
				cg.hud.skilltexts[2] = 0;
				cg.hud.skilltexts[3] = 0;
				cg.hudRect.skilltexts.initiazlied = qfalse;
			}
		case 'O':
		case 'o':
			if(!down)
				break;
			FLIPSIGN(cg.hud.skillboxes[0]);
			FLIPSIGN(cg.hudRect.skillboxes.x);
			if( (cg.hud.skillboxes[2] <= 0) || (cg.hud.skillboxes[3] <= 0) ) {
				cg.hud.skillboxes[0] = 1;
				cg.hud.skillboxes[1] = 17;
				cg.hud.skillboxes[2] = 16;
				cg.hud.skillboxes[3] = 16;
				cg.hudRect.skillboxes.initiazlied = qfalse;
			}
			break;
		case 'P':
		case 'p':
			if(!down)
				break;
			FLIPSIGN(cg.hud.weaponicon[0]);
			FLIPSIGN(cg.hudRect.weaponicon.x);
			if(cg.hud.weaponicon[2] <= 0) {
				cg.hud.weaponicon[0] = 1;
				cg.hud.weaponicon[1] = 1;
				cg.hud.weaponicon[2] = 60;
				cg.hud.weaponicon[3] = 32;
				cg.hudRect.weaponicon.initiazlied = qfalse;
			}
			break;
		case K_CAPSLOCK:
			// free
			break;
		case 'A':
		case 'a':
			if(!down)
				break;
			FLIPSIGN(cg.hud.fireteam[0]);
			FLIPSIGN(cg.hudRect.fireteam.x);
			if(cg.hud.fireteam[2] <= 0) {
				cg.hud.fireteam[0] = 1;
				cg.hud.fireteam[1] = 1;
				cg.hud.fireteam[2] = 100; 
				cg.hudRect.fireteam.initiazlied = qfalse;
			}
			break;
		case 'S':
		case 's':
			if(!down)
				break;
			FLIPSIGN(cg.hud.ammo[0]);
			FLIPSIGN(cg.hudRect.ammo.x);
			if(cg.hud.ammo[2] <= 0) {
				cg.hud.ammo[0] = 90;
				cg.hud.ammo[1] = 80;
				cg.hud.ammo[2] = 25; 
				cg.hudRect.ammo.initiazlied = qfalse;
			}
			break;
		case 'D':
		case 'd':
			if(!down)
				break;
			FLIPSIGN(cg.hud.head[0]);
			FLIPSIGN(cg.hudRect.head.x);
			if(cg.hud.head[2] <= 0) {
				cg.hud.head[0] = 1;
				cg.hud.head[1] = 1;
				cg.hud.head[2] = 62;
				cg.hud.head[3] = 80;
				cg.hudRect.head.initiazlied = qfalse;
			}
			break;
		case 'F':
		case 'f':
			if(!down)
				break;
			FLIPSIGN(cg.hud.lagometer[0]);
			FLIPSIGN(cg.hudRect.lagometer.x);
			cg.hudRect.lagometer.initiazlied = qfalse;
			break;
		case 'G':
		case 'g':
			if(!down)
				break;
			FLIPSIGN(cg.hud.flagcov[0]);
			FLIPSIGN(cg.hudRect.flagcov.x);
			cg.hudRect.flagcov.initiazlied = qfalse;
			break;
		case 'H':
		case 'h':
			if(!down)
				break;
			FLIPSIGN(cg.hud.popuptext[0]);
			FLIPSIGN(cg.hudRect.popuptext.x);
			cg.hudRect.popuptext.initiazlied = qfalse;
			break;
		case 'J':
		case 'j':
			if(!down)
				break;
			FLIPSIGN(cg.hud.chattext[0]);
			FLIPSIGN(cg.hudRect.chattext.x);
			if(cg.hud.chattext[2] <= 0) {
				cg.hud.chattext[0] = 160;
				cg.hud.chattext[1] = 478;
				cg.hud.chattext[2] = 20;
				cg.hudRect.chattext.initiazlied = qfalse;
			}
			break;
		case 'V':
		case 'v':
			if(!down)
				break;
			FLIPSIGN(cg.hud.votefttext[0]);
			FLIPSIGN(cg.hudRect.votefttext.x);
			if(cg.hud.votefttext[1] <= 0) {
				cg.hud.votefttext[0] = 8;
				cg.hud.votefttext[1] = 200;
				cg.hudRect.votefttext.initiazlied = qfalse;
			}
			break;
		case 'L':
		case 'l':
			if(!down)
				break;
			FLIPSIGN(cg.hud.livesleft[0]);
			FLIPSIGN(cg.hudRect.livesleft.x);
			if(cg.hud.livesleft[2] <= 0) {
				cg.hud.livesleft[0] = 4;
				cg.hud.livesleft[1] = 100;
				cg.hud.livesleft[2] = 14;
				cg.hudRect.livesleft.initiazlied = qfalse;
			}
			break;

		case 'K':
		case 'k':
			if(!down)
				break;
			FLIPSIGN(cg.hud.killspree[0]);
			FLIPSIGN(cg.hudRect.killspree.x);
			if(cg.hud.livesleft[2] <= 0) {
				cg.hud.killspree[0] = 8;
				cg.hud.killspree[1] = 180;
				cg.hud.killspree[2] = 25;
				cg.hudRect.killspree.initiazlied = qfalse;
			}
			break;


		// forty - load default hud.
		case 'Z':
		case 'z':
			if(!down)
				break;
			memset(&cg.hudRect, 0, sizeof(hudRect_t));
			CG_LoadDefaultHud();
			break;

		// forty - quitting time
		case K_ESCAPE: // We never actually catc
		case 'X':
		case 'x':
			if(!down)
				break;
			// forty - lets get out of here
			CG_HudEditor_f();
			break;

		// forty - set the scale mode
		case 'M':
		case 'm':
			if(down) {
				testrect = &cg.hudRect.healthbar;
				cg.hudEditor.hudScaleMode++;
				if(cg.hudEditor.hudScaleMode > HUD_SCALE_Y) {
					cg.hudEditor.hudScaleMode = HUD_SCALE_XY;
				}
				for(i=0; i<sizeof(hudRect_t)/sizeof(HudrectDef_t); i++) {
					if(down) {
						if(
							testrect->scaled && 
							testrect->scaled != HUD_SCALE_UNIFORM_DOWN &&
							testrect->scaled != HUD_SCALE_UNIFORM_UP
						) {
							testrect->scaled = cg.hudEditor.hudScaleMode;
						} 
					} 
					testrect++;
				}
			}
			break;

		// forty - toggle coordinates
		case 'C':
		case 'c':
			if(down) {
				cg.hudEditor.showCoor ^= qtrue; 
			}
			break;

		// forty - grouping
		case K_SHIFT:
			if(down) {
				cg.hudEditor.grouping = qtrue;
			} else {
				cg.hudEditor.grouping = qfalse;
			}
			break;

		case 'B':
		case 'b':
			if(down) {
				//CG_HudEditor_f();
				//trap_UI_Popup( UIMENU_HUDEDITOR_SAVE );
				// forty - set the flag and eat the key...
				cg.hudEditor.saving = qtrue;
			}
			break;
		case 'N':
		case 'n':
			if(down) {
				cg.hudEditor.loading = qtrue;
			}
			break;

		// forty - ignore everything else
		default:
			break;
	}
}

#define DH_X	-140	// spacing from right
#define DH_Y	-120	// spacing from bottom
#define DH_W	155

#define COLOR_BG			{ 0.0f, 0.0f, 0.0f, 0.6f }
#define COLOR_BORDER		{ 0.5f, 0.5f, 0.5f, 0.5f }
#define COLOR_BG_TITLE		{ 0.16, 0.2f, 0.17f, 0.8f }
#define COLOR_BG_VIEW		{ 0.16, 0.2f, 0.17f, 0.8f }
#define COLOR_BORDER_TITLE	{ 0.1f, 0.1f, 0.1f,  0.2f }
#define COLOR_BORDER_VIEW	{ 0.2f, 0.2f, 0.2f,  0.4f }
#define COLOR_HDR			{ 0.6f, 0.6f, 0.6f,  1.0f }
#define COLOR_HDR2			{ 0.6f, 0.6f, 0.4f,  1.0f }
#define COLOR_TEXT			{ 0.625f, 0.625f, 0.6f,  1.0f }

#define FONT_HEADER			&cgs.media.limboFont1
#define FONT_SUBHEADER		&cgs.media.limboFont1_lo
#define FONT_TEXT			&cgs.media.font3

#define HUDEDITOR_HOVERWIDTH 4
#define HUDEDITOR_BORDERWIDTH 2

// forty - visual hud editor - help message
static void CG_HudEditor_HelpDraw() {
	if(cg.hudEditor.showHudEditorHelp == SHOW_OFF) {
		// forty - don't draw
		return;
	} else {
		const char *help[] = {
			"^nMOUSE1  ^mDRAG AN ELEMENT",
			"^nMOUSE2  ^mSCALE AN ELEMENT",
			"^nMWHLUP  ^mUNIFORM SCALE UP",
			"^nMWHLDN  ^mUNIFORM SCALE DOWN",
			NULL,
			"^nTAB     ^mTOGGLE HEALTHBAR",
			"^nQ       ^mTOGGLE STAMINABAR",
			"^nW       ^mTOGGLE CHARGEBAR",
			"^nE       ^mTOGGLE COMPASS",
			"^nR       ^mTOGGLE HP",
			"^nT       ^mTOGGLE XP",
			"^nY       ^mTOGGLE XP ADD",
			"^nU       ^mTOGGLE DRAWS",
			"^nI       ^mTOOGLE SKILLTEXTS",
			"^nO       ^mTOGGLE SKILLBOXES",
			"^nP       ^mTOGGLE OVERHEAT",
			"^nCAPS    ^mTOGGLE WEAPONCARD",
			"^nA       ^mTOGGLE FIRETEAM",
			"^nS       ^mTOGGLE AMMO",
			"^nD       ^mTOGGLE HEAD",
			"^nF       ^mTOGGLE LAGOMETER",
			"^nG       ^mTOGGLE FLAGCOV",
			"^nH       ^mTOGGLE CPMTEXT",
			"^nJ       ^mTOGGLE CHATTEXT",
			"^nK       ^mTOGGLE SPREETEXT",
			"^nV       ^mTOGGLE VOTEFTTEXT",
			"^nL       ^mTOGGLE LIVESLEFT",
			NULL,
			"^nSHIFT   ^mHOLD TO GROUP",
			"^nZ       ^mDEFAULT HUD",
			"^nX|ESC   ^mEXIT",
			"^nC       ^mTOGGLE COORDINATES",
			"^nB       ^mSAVE HUD",
			"^nN       ^mLOAD HUD",
			"^nM       ^mSET SCALEMODE",

		};

		int i, x, y = 480, w, h;

		vec4_t bgColor			= COLOR_BG;			// window
		vec4_t borderColor		= COLOR_BORDER;		// window

		vec4_t bgColorTitle     = COLOR_BG_TITLE;		// titlebar
		vec4_t borderColorTitle = COLOR_BORDER_TITLE;	// titlebar

		// Main header
		int hStyle			= ITEM_TEXTSTYLE_SHADOWED;
		float hScale		= 0.16f;
		float hScaleY		= 0.21f;
		fontInfo_t *hFont	= FONT_HEADER;
		vec4_t hdrColor2	= COLOR_HDR2;	// text

		// Text settings
		int tStyle			= ITEM_TEXTSTYLE_SHADOWED;
		int tSpacing		= 9;		// Should derive from CG_Text_Height_Ext
		float tScale		= 0.19f;
		fontInfo_t *tFont	= FONT_TEXT;
		vec4_t tColor		= COLOR_TEXT;	// text

		float diff = cg.fadeTime - trap_Milliseconds();

		// FIXME: Should compute this beforehand
		w = DH_W;
		x = 640 + DH_X - w;
		h = 2 + tSpacing + 2 +									// Header
			2 + 1 +
			tSpacing * (2 + sizeof(help)) / sizeof(char *) + 2;

		// Fade-in effects
		if(diff > 0.0f) {
			float scale = (diff / STATS_FADE_TIME);

			if(cg.hudEditor.showHudEditorHelp == SHOW_ON) {
				scale = 1.0f - scale;
			}

			bgColor[3] *= scale;
			bgColorTitle[3] *= scale;
			borderColor[3] *= scale;
			borderColorTitle[3] *= scale;
			hdrColor2[3] *= scale;
			tColor[3] *= scale;

			y += (DH_Y - h) * scale;

		} else if (cg.hudEditor.showHudEditorHelp == SHOW_SHUTDOWN) {
			cg.hudEditor.showHudEditorHelp = SHOW_OFF;
			return;
		} else {
			y += DH_Y - h;
		}		
		
		// forty - draw help
		CG_DrawRect(x, y, w, h, 1, borderColor);
		CG_FillRect(x, y, w, h, bgColor);

		// Header
		CG_FillRect(x, y, w, tSpacing + 4, bgColorTitle);
		CG_DrawRect(x, y, w, tSpacing + 4, 1, borderColorTitle);

		x += 4;
		y += 1;
		y += tSpacing;
		CG_Text_Paint_Ext(x, y, hScale, hScaleY, hdrColor2, "EDITOR CONTROLS", 0.0f, 0, hStyle, hFont);
		y += 3;

		// Control info
		for(i=0; i<sizeof(help)/sizeof(char *); i++) {
			y += tSpacing;
			if(help[i] != NULL) {
				CG_Text_Paint_Ext(x, y, tScale, tScale, tColor, (char*)help[i], 0.0f, 0, tStyle, tFont);
			}
		}

		y += tSpacing * 2;
		CG_Text_Paint_Ext(x, y, tScale, tScale, tColor, "^nBACKSPACE ^mhelp on/off", 0.0f, 0, tStyle, tFont);
		//y += tSpacing * 2;
		//CG_Text_Paint_Ext(x, y, tScale, tScale, tColor, "^1DANGER ^muse at your own risk.", 0.0f, 0, tStyle, tFont);
	}
}

// forty - visual hud editor - drawing
static void CG_HudEditor_CursorDraw() {
	int w, h, x, y;
	float ScaleVal;
	char *str;

	// draw these last.
	trap_R_SetColor( NULL );
	CG_DrawPic( cgDC.cursorx, cgDC.cursory, 32, 32, cgs.media.cursorIcon );

	if(!cg.hudEditor.showCoor)
		return;

	// draw a box under cursor with co-ordinates.
	ScaleVal = 0.19f;
	if(cg.hudEditor.grouping) {
		str = va("%d, %d (Click MOUSE1 to add to group.)", cgDC.cursorx, cgDC.cursory );
	} else {
		switch(cg.hudEditor.hudScaleMode) {
			case HUD_SCALE_XY:
				str = va("%d, %d (XY Scaling)", cgDC.cursorx, cgDC.cursory );
				break;
			case HUD_SCALE_X:
				str = va("%d, %d (X Scaling)", cgDC.cursorx, cgDC.cursory );
				break;
			case HUD_SCALE_Y:
				str = va("%d, %d (Y Scaling)", cgDC.cursorx, cgDC.cursory );
				break;
			default:
				str = va("%d, %d", cgDC.cursorx, cgDC.cursory );
				break;
		}
	}

	w = CG_Text_Width_Ext( str, ScaleVal, 0, &cgs.media.font4 );
	h = CG_Text_Height_Ext( str, ScaleVal, 0, &cgs.media.font4 ) + 4;

	x = cgDC.cursorx;
	y = cgDC.cursory+32-h-4;

	// keep it on screen
	if(x < 0)
		x = 0;
	if(x > 640)
		x = 640;
	if((x + w) > 640)
		x -= (x + w) - 640;
	if(y < 0)
		y = 0;
	if(y > 480)
		y = 480;
	if((y + h) > 480) 
		y -= (y + h) - 480;

	CG_FillRect(x, y, w, h, cg.hudEditor.coorbg);
	CG_DrawRect_FixedBorder(x, y, w, h, 1, cg.hudEditor.coorborder);
	CG_Text_Paint_Ext(x, y+8, ScaleVal, ScaleVal, cg.hudEditor.coortxt, str, 0.0f, 0, 0, FONT_TEXT);

}

static void CG_HudEditor_HealthBarDraw() {
	// forty - if this hud element is being displayed.
	if(cg.hud.healthbar[0] >= 0) {
		if(!cg.hudRect.healthbar.initiazlied) {
			// forty - the drag and drop box hasn't been setup, 
			//         lets give it a default value that matches 
			//         where the hud element is now.
			cg.hudRect.healthbar.x = cg.hud.healthbar[0];
			cg.hudRect.healthbar.y = cg.hud.healthbar[1];
			cg.hudRect.healthbar.w = cg.hud.healthbar[2];
			cg.hudRect.healthbar.h = cg.hud.healthbar[3];//72;
			cg.hudRect.healthbar.initiazlied = qtrue;
		} else if(cg.hudRect.healthbar.dragged) {
			// forty - the drag and drop box has been moved 
			//         update the hud element with the new 
			//         coordinates
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.healthbar[0] = cg.hudRect.healthbar.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.healthbar[1] = cg.hudRect.healthbar.y;

			cg.hudRect.healthbar.x = cg.hud.healthbar[0];
			cg.hudRect.healthbar.y = cg.hud.healthbar[1];
		} else if(cg.hudRect.healthbar.scaled) {
			// forty - the drag and drop box has been scaled
			//         update the hud element with the new 
			//         width and height.
			cg.hud.healthbar[2] = cg.hudRect.healthbar.w;
			cg.hud.healthbar[3] = cg.hudRect.healthbar.h;
			// forty - height is static so keep it locked at 72.
			//cg.hudRect.healthbar.h = 72;
		}

		// forty - draw the drag and drop box.
		CG_DrawRect_FixedBorder(
			cg.hudRect.healthbar.x,
			cg.hudRect.healthbar.y,
			cg.hudRect.healthbar.w,
			cg.hudRect.healthbar.h,
			// forty - set the border width on mouse over.
			(cg.hudRect.healthbar.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			// forty - set the border color on mouse over.
			(cg.hudRect.healthbar.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);
	}
}

static void CG_HudEditor_StaminaBarDraw() {
	if(cg.hud.staminabar[0] >= 0) {
		if(!cg.hudRect.staminabar.initiazlied) {
			cg.hudRect.staminabar.x = cg.hud.staminabar[0];
			cg.hudRect.staminabar.y = cg.hud.staminabar[1];
			cg.hudRect.staminabar.w = cg.hud.staminabar[2];
			cg.hudRect.staminabar.h = cg.hud.staminabar[3];//72;
			cg.hudRect.staminabar.initiazlied = qtrue;
		} else if(cg.hudRect.staminabar.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.staminabar[0] = cg.hudRect.staminabar.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.staminabar[1] = cg.hudRect.staminabar.y;

			cg.hudRect.staminabar.x = cg.hud.staminabar[0];
			cg.hudRect.staminabar.y = cg.hud.staminabar[1];
		} else if(cg.hudRect.staminabar.scaled) {
			cg.hud.staminabar[2] = cg.hudRect.staminabar.w;
			cg.hud.staminabar[3] = cg.hudRect.staminabar.h;
			// forty - height is static so keep it locked at 72.
			//cg.hudRect.staminabar.h = 72;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.staminabar.x,
			cg.hudRect.staminabar.y,
			cg.hudRect.staminabar.w,
			cg.hudRect.staminabar.h,
			(cg.hudRect.staminabar.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.staminabar.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);
	}
}

static void CG_HudEditor_ChargeBarDraw() {
	if(cg.hud.chargebar[0] >= 0) {
		if(!cg.hudRect.chargebar.initiazlied) {
			cg.hudRect.chargebar.x = cg.hud.chargebar[0];
			cg.hudRect.chargebar.y = cg.hud.chargebar[1];
			cg.hudRect.chargebar.w = cg.hud.chargebar[2];
			cg.hudRect.chargebar.h = cg.hud.chargebar[3]; //72;
			cg.hudRect.chargebar.initiazlied = qtrue;
		} else if(cg.hudRect.chargebar.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.chargebar[0] = cg.hudRect.chargebar.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.chargebar[1] = cg.hudRect.chargebar.y;

			cg.hudRect.chargebar.x = cg.hud.chargebar[0];
			cg.hudRect.chargebar.y = cg.hud.chargebar[1];
		} else if(cg.hudRect.chargebar.scaled) {
			cg.hud.chargebar[2] = cg.hudRect.chargebar.w;
			cg.hud.chargebar[3] = cg.hudRect.chargebar.h;
			// forty - height is static so keep it locked at 72.
			//cg.hudRect.chargebar.h = 72;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.chargebar.x,
			cg.hudRect.chargebar.y,
			cg.hudRect.chargebar.w,
			cg.hudRect.chargebar.h,
			(cg.hudRect.chargebar.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.chargebar.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);	
	}
}

static void CG_HudEditor_CompassDraw() {
	if(cg.hud.compass[0] >= 0) {
		if(!cg.hudRect.compass.initiazlied) {
			cg.hudRect.compass.x = cg.hud.compass[0];
			cg.hudRect.compass.y = cg.hud.compass[1];
			cg.hudRect.compass.w = cg.hud.compass[2];
			cg.hudRect.compass.h = cg.hud.compass[2];
			cg.hudRect.compass.initiazlied = qtrue;
		} else if(cg.hudRect.compass.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.compass[0] = cg.hudRect.compass.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.compass[1] = cg.hudRect.compass.y;

			cg.hudRect.compass.x = cg.hud.compass[0];
			cg.hudRect.compass.y = cg.hud.compass[1];
		} else if(cg.hudRect.compass.scaled) {
			switch(cg.hudEditor.hudScaleMode) {
				case HUD_SCALE_Y:
					cg.hud.compass[2] = cg.hudRect.compass.h;
					cg.hudRect.compass.w = cg.hudRect.compass.h;
					break;
				default:			
					cg.hud.compass[2] = cg.hudRect.compass.w;
					cg.hudRect.compass.h = cg.hudRect.compass.w;
					break;
			}
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.compass.x,
			cg.hudRect.compass.y,
			cg.hudRect.compass.w,
			cg.hudRect.compass.h,
			(cg.hudRect.compass.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.compass.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);
	}
}

static void CG_HudEditor_HpDraw() {
	int w, h;
	float ScaleVal;
	if(cg.hud.hp[0] >= 0) {
		ScaleVal = cg.hud.hp[2]/100.f;
		w = CG_Text_Width_Ext( "100", ScaleVal, 0, &cgs.media.limboFont1 ) + 
			CG_Text_Width_Ext( "HP", ScaleVal * 0.8f, 0, &cgs.media.limboFont1 ) + 2;
		h = CG_Text_Height_Ext( "100", ScaleVal, 0, &cgs.media.limboFont1 );
		if(!cg.hudRect.hp.initiazlied) {
			cg.hudRect.hp.x = cg.hud.hp[0] - 2;
			cg.hudRect.hp.y = cg.hud.hp[1] - h - 4;
			cg.hudRect.hp.w = w + 4;
			cg.hudRect.hp.h = h + 8;
			cg.hudRect.hp.initiazlied = qtrue;
		} else if(cg.hudRect.hp.dragged){
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.hp[0] = cg.hudRect.hp.x + 2;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.hp[1] = cg.hudRect.hp.y + h + 4;

			cg.hudRect.hp.x = cg.hud.hp[0] - 2;
			cg.hudRect.hp.y = cg.hud.hp[1] - h - 4;
		} else if(cg.hudRect.hp.scaled) {
			switch(cg.hudEditor.hudScaleMode) {
				case HUD_SCALE_Y:
					cg.hud.hp[2] += (cg.hudRect.hp.h-8) - h;
					break;
				default: 
					cg.hud.hp[2] += (cg.hudRect.hp.w-4) - w;
					break;
			}
			ScaleVal = cg.hud.hp[2]/100.f;
			w = CG_Text_Width_Ext( "100", ScaleVal, 0, &cgs.media.limboFont1 ) + 
				CG_Text_Width_Ext( "HP", ScaleVal * 0.8f, 0, &cgs.media.limboFont1 ) + 2;
			h = CG_Text_Height_Ext( "100", ScaleVal, 0, &cgs.media.limboFont1 );
			cg.hudRect.hp.w = w + 4;
			cg.hudRect.hp.h = h + 8;
			cg.hud.hp[0] = cg.hudRect.hp.x + 2;
			cg.hud.hp[1] = cg.hudRect.hp.y + h + 4;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.hp.x,
			cg.hudRect.hp.y,
			cg.hudRect.hp.w,
			cg.hudRect.hp.h,
			(cg.hudRect.hp.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.hp.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);

	}
}

static void CG_HudEditor_XpDraw() {
	int w, h;
	float ScaleVal;

	if(cg.hud.xp[0] >= 0) {
		ScaleVal = cg.hud.xp[2]/100.f;
		w = CG_Text_Width_Ext( "32768", ScaleVal, 0, &cgs.media.limboFont1 ) + 
			CG_Text_Width_Ext( "XP", ScaleVal * 0.8f, 0, &cgs.media.limboFont1 ) + 2;
		h = CG_Text_Height_Ext( "32768", ScaleVal, 0, &cgs.media.limboFont1);
		if(!cg.hudRect.xp.initiazlied) {
			cg.hudRect.xp.x = cg.hud.xp[0] - 2;
			cg.hudRect.xp.y = cg.hud.xp[1] - h - 4;
			cg.hudRect.xp.w = w + 4;
			cg.hudRect.xp.h = h + 8;
			cg.hudRect.xp.initiazlied = qtrue;
		} else if(cg.hudRect.xp.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.xp[0] = cg.hudRect.xp.x + 2;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.xp[1] = cg.hudRect.xp.y + h + 4;

			cg.hudRect.xp.x = cg.hud.xp[0] - 2;
			cg.hudRect.xp.y = cg.hud.xp[1] - h - 4;
		} else if(cg.hudRect.xp.scaled) {
			switch(cg.hudEditor.hudScaleMode) {
				case HUD_SCALE_Y:
					cg.hud.xp[2] += (cg.hudRect.xp.h-8) - h;
					break;
				default: 
					cg.hud.xp[2] += (cg.hudRect.xp.w-4) - w;
					break;
			}
			ScaleVal = cg.hud.xp[2]/100.f;
			w = CG_Text_Width_Ext( "32768", ScaleVal, 0, &cgs.media.limboFont1 ) + 
				CG_Text_Width_Ext( "XP", ScaleVal * 0.8f, 0, &cgs.media.limboFont1 ) + 2;
			h = CG_Text_Height_Ext( "32768", ScaleVal, 0, &cgs.media.limboFont1);
			cg.hudRect.xp.w = w + 4;
			cg.hudRect.xp.h = h + 8;
			cg.hud.xp[0] = cg.hudRect.xp.x + 2;
			cg.hud.xp[1] = cg.hudRect.xp.y + h + 4;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.xp.x,
			cg.hudRect.xp.y,
			cg.hudRect.xp.w,
			cg.hudRect.xp.h,
			(cg.hudRect.xp.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.xp.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);

		//if(cg.hudRect.xp.dragged) {
		//}
	}
}


static void CG_HudEditor_XpAddDraw() {
	int w, h;
	float ScaleVal;

	if(cg.hud.xpadd[0] >= 0) {
		ScaleVal = cg.hud.xpadd[2]/100.f;
		w = CG_Text_Width_Ext( "32768", ScaleVal, 0, &cgs.media.limboFont1 ) + 
			CG_Text_Width_Ext( "XP", ScaleVal * 0.8f, 0, &cgs.media.limboFont1 ) + 2;
		h = CG_Text_Height_Ext( "32768", ScaleVal, 0, &cgs.media.limboFont1);
		if(!cg.hudRect.xpadd.initiazlied) {
			cg.hudRect.xpadd.x = cg.hud.xpadd[0] - 2;
			cg.hudRect.xpadd.y = cg.hud.xpadd[1] - h - 4;
			cg.hudRect.xpadd.w = w + 4;
			cg.hudRect.xpadd.h = h + 8;
			cg.hudRect.xpadd.initiazlied = qtrue;
		} else if(cg.hudRect.xpadd.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.xpadd[0] = cg.hudRect.xpadd.x + 2;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.xpadd[1] = cg.hudRect.xpadd.y + h + 4;

			cg.hudRect.xpadd.x = cg.hud.xpadd[0] - 2;
			cg.hudRect.xpadd.y = cg.hud.xpadd[1] - h - 4;
		} else if(cg.hudRect.xpadd.scaled) {
			switch(cg.hudEditor.hudScaleMode) {
				case HUD_SCALE_Y:
					cg.hud.xpadd[2] += (cg.hudRect.xpadd.h-8) - h;
					break;
				default: 
					cg.hud.xpadd[2] += (cg.hudRect.xpadd.w-4) - w;
					break;
			}
			ScaleVal = cg.hud.xpadd[2]/100.f;
			w = CG_Text_Width_Ext( "32768", ScaleVal, 0, &cgs.media.limboFont1 ) + 
				CG_Text_Width_Ext( "XP", ScaleVal * 0.8f, 0, &cgs.media.limboFont1 ) + 2;
			h = CG_Text_Height_Ext( "32768", ScaleVal, 0, &cgs.media.limboFont1);
			cg.hudRect.xpadd.w = w + 4;
			cg.hudRect.xpadd.h = h + 8;
			cg.hud.xpadd[0] = cg.hudRect.xpadd.x + 2;
			cg.hud.xpadd[1] = cg.hudRect.xpadd.y + h + 4;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.xpadd.x,
			cg.hudRect.xpadd.y,
			cg.hudRect.xpadd.w,
			cg.hudRect.xpadd.h,
			(cg.hudRect.xpadd.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.xpadd.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);
	}
}

static void CG_HudEditor_DrawsDraw() {
	if(cg.hud.draws[0] >= 0) {
		if(!cg.hudRect.draws.initiazlied) {
			cg.hudRect.draws.x = 504;
			cg.hudRect.draws.y = cg.hud.draws[0];
			cg.hudRect.draws.w = 132;
			cg.hudRect.draws.h = 132;
			cg.hudRect.draws.initiazlied = qtrue;
		} else if(cg.hudRect.draws.dragged) {
			cg.hud.draws[0] = cg.hudRect.draws.y;
			// forty - x is static so keep it locked at 504.
			cg.hudRect.draws.x = 504;
		} else if(cg.hudRect.draws.scaled) {
			// forty - width and height is just for decoration 
			//         here so keep it locked at 132.
			cg.hudRect.draws.w = 132;
			cg.hudRect.draws.h = 132;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.draws.x,
			cg.hudRect.draws.y,
			cg.hudRect.draws.w,
			cg.hudRect.draws.h,
			(cg.hudRect.draws.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.draws.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);
	}
}


static void CG_HudEditor_SkillDraw() {
	int i;
	int off;
//	float ScaleVal;

	for(i=0;i<3;i++) {
		//ScaleVal = cg.hud.skilltexts[i][2]/100.f;
		off = 20;

		if(cg.hud.skilltexts[0] >= 0) {
			if(!cg.hudRect.skilltexts.initiazlied) {
				cg.hudRect.skilltexts.x = cg.hud.skilltexts[0];
				cg.hudRect.skilltexts.y = cg.hud.skilltexts[1];
				cg.hudRect.skilltexts.w = off;
				cg.hudRect.skilltexts.h = off;
				cg.hudRect.skilltexts.initiazlied = qtrue;
			} else if(cg.hudRect.skilltexts.dragged) {
				if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
					cg.hud.skilltexts[0] = cg.hudRect.skilltexts.x;
				if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
					cg.hud.skilltexts[1] = cg.hudRect.skilltexts.y;

				cg.hudRect.skilltexts.x = cg.hud.skilltexts[0];
				cg.hudRect.skilltexts.y = cg.hud.skilltexts[1];
			} else if(cg.hudRect.skilltexts.scaled) {
				//cg.hud.skilltexts[2] = cg.hudRect.skilltexts.w;
				//cg.hudRect.skilltexts.h = cg.hudRect.skilltexts.w;

				if( cg.hudRect.skilltexts.w < off ) {
						cg.hud.skilltexts[2] = 1;
				} else { cg.hud.skilltexts[2] = 0; }

				if( cg.hudRect.skilltexts.w > cg.hudRect.skilltexts.h ) {
						cg.hud.skilltexts[3] = 0;
				} else { cg.hud.skilltexts[3] = 1; }

				if( cg.hudRect.skilltexts.w > off*2 )
					cg.hudRect.skilltexts.w = off*2;
				if( cg.hudRect.skilltexts.h > off*2 )
					cg.hudRect.skilltexts.h = off*2;
			}
			CG_DrawRect_FixedBorder(
				cg.hudRect.skilltexts.x,
				cg.hudRect.skilltexts.y,
				cg.hudRect.skilltexts.w,
				cg.hudRect.skilltexts.h,
				(cg.hudRect.skilltexts.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
				(cg.hudRect.skilltexts.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
			);
		}

		if(cg.hud.skillboxes[0] >=0) {
			if(!cg.hudRect.skillboxes.initiazlied) {
				cg.hudRect.skillboxes.x = cg.hud.skillboxes[0];
				cg.hudRect.skillboxes.y = cg.hud.skillboxes[1];
				cg.hudRect.skillboxes.w = cg.hud.skillboxes[2];
				cg.hudRect.skillboxes.h = cg.hud.skillboxes[3];
				cg.hudRect.skillboxes.initiazlied = qtrue;
			} else if (cg.hudRect.skillboxes.dragged) {
				if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
					cg.hud.skillboxes[0] = cg.hudRect.skillboxes.x;
				if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
					cg.hud.skillboxes[1] = cg.hudRect.skillboxes.y;

				cg.hudRect.skillboxes.x = cg.hud.skillboxes[0];
				cg.hudRect.skillboxes.y = cg.hud.skillboxes[1];
			} else if (cg.hudRect.skillboxes.scaled) {
				cg.hud.skillboxes[2] = cg.hudRect.skillboxes.w;
				cg.hud.skillboxes[3] = cg.hudRect.skillboxes.h;
			}

			CG_DrawRect_FixedBorder(
				cg.hudRect.skillboxes.x,
				cg.hudRect.skillboxes.y,
				cg.hudRect.skillboxes.w,
				cg.hudRect.skillboxes.h,
				(cg.hudRect.skillboxes.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
				(cg.hudRect.skillboxes.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
			);
		}
	}
}

static void CG_HudEditor_WeaponDraw() {
	if(cg.hud.weaponicon[0] >=0) {
		if(!cg.hudRect.weaponicon.initiazlied) {
			cg.hudRect.weaponicon.x = cg.hud.weaponicon[0];
			cg.hudRect.weaponicon.y = cg.hud.weaponicon[1];
			cg.hudRect.weaponicon.w = cg.hud.weaponicon[2];
			cg.hudRect.weaponicon.h = cg.hud.weaponicon[3];
			cg.hudRect.weaponicon.initiazlied = qtrue;
		} else if(cg.hudRect.weaponicon.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.weaponicon[0] = cg.hudRect.weaponicon.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.weaponicon[1] = cg.hudRect.weaponicon.y;

			cg.hudRect.weaponicon.x = cg.hud.weaponicon[0];
			cg.hudRect.weaponicon.y = cg.hud.weaponicon[1];
		} else if(cg.hudRect.weaponicon.scaled) {
			cg.hud.weaponicon[2] = cg.hudRect.weaponicon.w;
			cg.hud.weaponicon[3] = cg.hudRect.weaponicon.h;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.weaponicon.x,
			cg.hudRect.weaponicon.y,
			cg.hudRect.weaponicon.w,
			cg.hudRect.weaponicon.h,
			(cg.hudRect.weaponicon.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.weaponicon.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);
	}
}

static void CG_HudEditor_FireTeamDraw() {
	if(cg.hud.fireteam[0] >= 0) {
		if(!cg.hudRect.fireteam.initiazlied) {
			cg.hudRect.fireteam.x = cg.hud.fireteam[0];
			cg.hudRect.fireteam.y = cg.hud.fireteam[1];
			cg.hudRect.fireteam.w = 204;
			cg.hudRect.fireteam.h = 88;
			cg.hudRect.fireteam.initiazlied = qtrue;
		} else if(cg.hudRect.fireteam.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.fireteam[0] = cg.hudRect.fireteam.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.fireteam[1] = cg.hudRect.fireteam.y;

			cg.hudRect.fireteam.x = cg.hud.fireteam[0];
			cg.hudRect.fireteam.y = cg.hud.fireteam[1];
		} else if(cg.hudRect.fireteam.scaled) {
			// don't scale.
			cg.hudRect.fireteam.w = 204;
			cg.hudRect.fireteam.h = 88;
		}
		CG_DrawRect_FixedBorder(
			cg.hudRect.fireteam.x,
			cg.hudRect.fireteam.y,
			cg.hudRect.fireteam.w,
			cg.hudRect.fireteam.h,
			(cg.hudRect.fireteam.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.fireteam.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);
	}
}

static void CG_HudEditor_AmmoDraw() {
	int w, h;
	float ScaleVal;

	if(cg.hud.ammo[0] >= 0) {
		ScaleVal = cg.hud.ammo[2]/100.f;
		w = CG_Text_Width_Ext( "9999/9999", ScaleVal, 0, &cgs.media.limboFont1 );
		h = CG_Text_Height_Ext( "9999/9999", ScaleVal, 0, &cgs.media.limboFont1 );
		if(!cg.hudRect.ammo.initiazlied) {
			cg.hudRect.ammo.x = cg.hud.ammo[0] - w;
			cg.hudRect.ammo.y = cg.hud.ammo[1] - h - 2;
			cg.hudRect.ammo.w = w;
			cg.hudRect.ammo.h = h + 2;
			cg.hudRect.ammo.initiazlied = qtrue;
		} else if(cg.hudRect.ammo.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.ammo[0] = cg.hudRect.ammo.x + w;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.ammo[1] = cg.hudRect.ammo.y + h + 2;

			cg.hudRect.ammo.x = cg.hud.ammo[0] - w;
			cg.hudRect.ammo.y = cg.hud.ammo[1] - h - 2;
		} else if(cg.hudRect.ammo.scaled) {
			switch(cg.hudEditor.hudScaleMode) {
				case HUD_SCALE_Y:
					cg.hud.ammo[2] += (cg.hudRect.ammo.h-2) - h;
					break;
				default: 
					cg.hud.ammo[2] += (cg.hudRect.ammo.w) - w;
					break;
			}
			ScaleVal = cg.hud.ammo[2]/100.f;
			w = CG_Text_Width_Ext( "30|30/9999", ScaleVal, 0, &cgs.media.limboFont1 );
			h = CG_Text_Height_Ext( "30|30/9999", ScaleVal, 0, &cgs.media.limboFont1 );
			cg.hudRect.ammo.w = w;
			cg.hudRect.ammo.h = h + 2;
			cg.hud.ammo[0] = cg.hudRect.ammo.x + w;
			cg.hud.ammo[1] = cg.hudRect.ammo.y + h + 2;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.ammo.x,
			cg.hudRect.ammo.y,
			cg.hudRect.ammo.w,
			cg.hudRect.ammo.h,
			(cg.hudRect.ammo.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.ammo.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);
	}
}


static void CG_HudEditor_HeadDraw() {
	if(cg.hud.head[0] >= 0) {
		if(!cg.hudRect.head.initiazlied) {
			cg.hudRect.head.x = cg.hud.head[0];
			cg.hudRect.head.y = cg.hud.head[1];
			cg.hudRect.head.w = cg.hud.head[2];
			cg.hudRect.head.h = cg.hud.head[3];
			cg.hudRect.head.initiazlied = qtrue;
		} else if(cg.hudRect.head.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.head[0] = cg.hudRect.head.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.head[1] = cg.hudRect.head.y;

			cg.hudRect.head.x = cg.hud.head[0];
			cg.hudRect.head.y = cg.hud.head[1];
		} else if(cg.hudRect.head.scaled) {
			cg.hud.head[2] = cg.hudRect.head.w;
			cg.hud.head[3] = cg.hudRect.head.h;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.head.x,
			cg.hudRect.head.y,
			cg.hudRect.head.w,
			cg.hudRect.head.h,
			(cg.hudRect.head.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.head.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr

		);
	}
}

static void CG_HudEditor_LagometerDraw() {
	if(cg.hud.lagometer[0] >= 0) {
		if(!cg.hudRect.lagometer.initiazlied) {
			cg.hudRect.lagometer.x = cg.hud.lagometer[0]-4;
			cg.hudRect.lagometer.y = cg.hud.lagometer[1];
			cg.hudRect.lagometer.w = 48;
			cg.hudRect.lagometer.h = 48;
			cg.hudRect.lagometer.initiazlied = qtrue;
		} else if(cg.hudRect.lagometer.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.lagometer[0] = cg.hudRect.lagometer.x+4;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.lagometer[1] = cg.hudRect.lagometer.y;

			cg.hudRect.lagometer.x = cg.hud.lagometer[0]-4;
			cg.hudRect.lagometer.y = cg.hud.lagometer[1];
		} else if(cg.hudRect.lagometer.scaled) {
			//no scale ever
			cg.hudRect.lagometer.w = 48;
			cg.hudRect.lagometer.h = 48;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.lagometer.x,
			cg.hudRect.lagometer.y,
			cg.hudRect.lagometer.w,
			cg.hudRect.lagometer.h,
			(cg.hudRect.lagometer.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.lagometer.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);
	}
}

static void CG_HudEditor_FlagcovDraw() {
	if(cg.hud.flagcov[0] >= 0) {
		if(!cg.hudRect.flagcov.initiazlied) {
			cg.hudRect.flagcov.x = cg.hud.flagcov[0];
			cg.hudRect.flagcov.y = cg.hud.flagcov[1];
			cg.hudRect.flagcov.w = 36;
			cg.hudRect.flagcov.h = 36;
			cg.hudRect.flagcov.initiazlied = qtrue;
		} else if(cg.hudRect.flagcov.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.flagcov[0] = cg.hudRect.flagcov.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.flagcov[1] = cg.hudRect.flagcov.y;

			cg.hudRect.flagcov.x = cg.hud.flagcov[0];
			cg.hudRect.flagcov.y = cg.hud.flagcov[1];
		} else if(cg.hudRect.flagcov.scaled) {
			//no scale ever
			cg.hudRect.flagcov.w = 36;
			cg.hudRect.flagcov.h = 36;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.flagcov.x,
			cg.hudRect.flagcov.y,
			cg.hudRect.flagcov.w,
			cg.hudRect.flagcov.h,
			(cg.hudRect.flagcov.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.flagcov.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);

		CG_DrawPic( cg.hud.flagcov[0], cg.hud.flagcov[1], 36, 36, cgs.media.objectiveShader );
	}
}


static void CG_HudEditor_PopupTextDraw() {
	//int height = ( cg_numPopups.integer * ((cg_drawSmallPopupIcons.integer ? 12 : 20)+2) );
	int size2 = ((cg_drawSmallPopupIcons.integer ? 12 : 20) + 2);
	int height; // = ( CG_CalculateNumPopups( size2 ) * size2 );

	if( cg_numPopups.integer < 10 ) {
		height = ( cg_numPopups.integer * size2 );
	} else {
		height =( 10 * size2 );
	}

	if(cg.hud.popuptext[0] >= 0) {
		if(!cg.hudRect.popuptext.initiazlied) {
			cg.hudRect.popuptext.x = cg.hud.popuptext[0];
			cg.hudRect.popuptext.y = cg.hud.popuptext[1] - height - 1;
			cg.hudRect.popuptext.w = 300;
			cg.hudRect.popuptext.h = height + 2;
			cg.hudRect.popuptext.initiazlied = qtrue;
		} else if(cg.hudRect.popuptext.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.popuptext[0] = cg.hudRect.popuptext.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.popuptext[1] = cg.hudRect.popuptext.y + height + 1;

			cg.hudRect.popuptext.x = cg.hud.popuptext[0];
			cg.hudRect.popuptext.y = cg.hud.popuptext[1] - height - 1;
			// forty - x is static so keep it locked.
			//cg.hudRect.popuptext.x = 4;
		} else if(cg.hudRect.popuptext.scaled) {
			// forty - width and height is just for decoration 
			//         here so keep it locked at 132.
			cg.hudRect.popuptext.w = 300;
			cg.hudRect.popuptext.h = height + 2;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.popuptext.x,
			cg.hudRect.popuptext.y,
			cg.hudRect.popuptext.w,
			cg.hudRect.popuptext.h,
			(cg.hudRect.popuptext.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.popuptext.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);

		// forty - ratelimit
		if(trap_Milliseconds() - cg.hudEditor.popupTime > 1000) {
			vec4_t color = { 1.f, 1.f, 1.f, 1.f };
			CG_AddPMItem( PM_MESSAGE, "Sample", cgs.media.voiceChatShader, color );
			cg.hudEditor.popupTime = trap_Milliseconds();
		}
	}
}

static void CG_HudEditor_ChatTextDraw() {
	int w,h;
	
	if(cg.hud.chattext[0] >= 0) {
		//w = 640 - cg.hud.chattext[0] - 100; 
		w = cg.hud.chattext[2];
		if( cg_teamChatHeight.integer < TEAMCHAT_HEIGHT ) {
			h = 10 * cg_teamChatHeight.integer;
		} else {
			h = 10 * TEAMCHAT_HEIGHT;
		}

		if(!cg.hudRect.chattext.initiazlied) {
			cg.hudRect.chattext.x = cg.hud.chattext[0];
			cg.hudRect.chattext.y = cg.hud.chattext[1] - h;
			cg.hudRect.chattext.w = w;
			cg.hudRect.chattext.h = h;
			cg.hudRect.chattext.initiazlied = qtrue;
		} else if(cg.hudRect.chattext.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.chattext[0] = cg.hudRect.chattext.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.chattext[1] = cg.hudRect.chattext.y + h;

			cg.hudRect.chattext.x = cg.hud.chattext[0];
			cg.hudRect.chattext.y = cg.hud.chattext[1] - h;
		} else if(cg.hudRect.chattext.scaled) {
			// forty - scaling this scales the text but not the backgrounds....
			switch(cg.hudEditor.hudScaleMode) {
				case HUD_SCALE_Y:
					//cg.hud.chattext[2] += (cg.hudRect.chattext.h) - h;
					break;
				default: 
					cg.hud.chattext[2] += (cg.hudRect.chattext.w) - w;
					break;
			}
			// forty - background box doesn't scale only text.
			//cg.hudRect.chattext.w = w;
			cg.hudRect.chattext.h = h;
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.chattext.x,
			cg.hudRect.chattext.y,
			cg.hudRect.chattext.w,
			cg.hudRect.chattext.h,
			(cg.hudRect.chattext.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.chattext.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);

		if(trap_Milliseconds() - cg.hudEditor.chatTime > 1000) {
			CG_AddToTeamChat("Sample", cg.clientNum);
			cg.hudEditor.chatTime = trap_Milliseconds();
		}

	}
}

static void CG_HudEditor_VoteFTTextDraw() {
	int w, h;
	float ScaleVal;

	if(cg.hud.votefttext[0] >= 0) {
		ScaleVal = cg_fontQAScale.value;
		w = CG_Text_Width_Ext( "Cannot vote as Spectator", ScaleVal, 0, &cgs.media.font4 );
		h = CG_Text_Height_Ext( "Cannot vote as Spectator", ScaleVal, 0, &cgs.media.font4 );
		if(!cg.hudRect.votefttext.initiazlied) {
			cg.hudRect.votefttext.x = cg.hud.votefttext[0] - 2;
			cg.hudRect.votefttext.y = cg.hud.votefttext[1] - h - 4;
			cg.hudRect.votefttext.w = (w * 2);
			cg.hudRect.votefttext.h = (h * 6);
			cg.hudRect.votefttext.initiazlied = qtrue;
		} else if (cg.hudRect.votefttext.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.votefttext[0] = cg.hudRect.votefttext.x + 2;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.votefttext[1] = cg.hudRect.votefttext.y + h + 4;

			cg.hudRect.votefttext.x = cg.hud.votefttext[0] - 2;
			cg.hudRect.votefttext.y = cg.hud.votefttext[1] - h - 4;
		} else if (cg.hudRect.votefttext.scaled) {
			// scaling this element is handled elsewhere. 
			cg.hudRect.votefttext.w = (w * 2);
			cg.hudRect.votefttext.h = (h * 6);
		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.votefttext.x,
			cg.hudRect.votefttext.y,
			cg.hudRect.votefttext.w,
			cg.hudRect.votefttext.h,
			(cg.hudRect.votefttext.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.votefttext.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);

		if(trap_Milliseconds() - cg.hudEditor.voteTime > 1000) {
			cgs.voteTime = cg.time;
			Com_sprintf(cgs.voteString, sizeof(cgs.voteString), "Sample Vote Called by Sample Player");
			cg.hudEditor.voteTime = trap_Milliseconds();
		}
	}
}

static void CG_HudEditor_LivesLeftDraw() {
	if(cg.hud.livesleft[0] >= 0) {

		if(!cg.hudRect.livesleft.initiazlied) {
			cg.hudRect.livesleft.x = cg.hud.livesleft[0];
			cg.hudRect.livesleft.y = cg.hud.livesleft[1];
			cg.hudRect.livesleft.w = (int)(cg.hud.livesleft[2] * 3.4f);
			cg.hudRect.livesleft.h = (int)(cg.hud.livesleft[2] * 1.7f);
			cg.hudRect.livesleft.initiazlied = qtrue;
		} else if(cg.hudRect.livesleft.dragged) {
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_Y )
				cg.hud.livesleft[0] = cg.hudRect.livesleft.x;
			if( cg.hudEditor.hudScaleMode != HUD_SCALE_X )
				cg.hud.livesleft[1] = cg.hudRect.livesleft.y;

			cg.hudRect.livesleft.x = cg.hud.livesleft[0];
			cg.hudRect.livesleft.y = cg.hud.livesleft[1];

			//cpm text depends on our position.
			cg.hudRect.popuptext.initiazlied = qfalse;
		} else if(cg.hudRect.livesleft.scaled) {
			switch(cg.hudEditor.hudScaleMode) {
				case HUD_SCALE_Y:
					cg.hud.livesleft[2] = (int)(cg.hudRect.livesleft.h/1.7f);
					break;
				default:
					cg.hud.livesleft[2] = (int)(cg.hudRect.livesleft.w/3.4f);
					break;
			}
		}	

		CG_DrawRect_FixedBorder(
			cg.hudRect.livesleft.x,
			cg.hudRect.livesleft.y,
			cg.hudRect.livesleft.w,
			cg.hudRect.livesleft.h,
			(cg.hudRect.livesleft.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.livesleft.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);

		CG_DrawPic(
			cg.hudRect.livesleft.x,
			cg.hudRect.livesleft.y,
			cg.hudRect.livesleft.w,
			cg.hudRect.livesleft.h,
			cgs.media.hudAxisHelmet
		);
	}
}


// codePRK killingspree
#define BP_TIME 10000
static void CG_HudEditor_KillspreeDraw() {
	int w, h;
	float ScaleVal;
	char *msg = "^1KILLINGSPREE! \n^7SampleName: ^3999 ^7kills in a row!";
//	int yoff = 4;


	if(cg.hud.killspree[0] >= 0) {
		ScaleVal = cg.hud.ammo[2]/100.f;
		w = 200;
		h = CG_Text_Height_Ext( msg, ScaleVal, 0, &cgs.media.limboFont1 );

		if(!cg.hudRect.killspree.initiazlied) {
			cg.hudRect.killspree.x = cg.hud.killspree[0];
			cg.hudRect.killspree.y = cg.hud.killspree[1];
			cg.hudRect.killspree.w = w;
			cg.hudRect.killspree.h = (h * 2)*2;
			cg.hudRect.killspree.initiazlied = qtrue;
		} else if (cg.hudRect.killspree.dragged) {
			cg.hud.killspree[0] = cg.hudRect.killspree.x;
			cg.hud.killspree[1] = cg.hudRect.killspree.y;
		} else if (cg.hudRect.killspree.scaled) {
			//cg.hudRect.killspree.w = w;
			//cg.hudRect.killspree.h = (h * 2)*2;
			switch(cg.hudEditor.hudScaleMode) {
				case HUD_SCALE_Y:
					cg.hud.killspree[2] += (cg.hudRect.killspree.h - ((h * 2)*2) );
					break;
				default: 
					cg.hud.killspree[2] += (cg.hudRect.killspree.w) - w;
					break;
			}
			ScaleVal = cg.hud.killspree[2]/100.f;
			w = 200;
			h = CG_Text_Height_Ext( msg, ScaleVal, 0, &cgs.media.limboFont1 );
			cg.hudRect.killspree.w = w;
			cg.hudRect.killspree.h = (h * 2)*2;
			cg.hud.killspree[0] = cg.hudRect.killspree.x;
			cg.hud.killspree[1] = cg.hudRect.killspree.y;

		}

		CG_DrawRect_FixedBorder(
			cg.hudRect.killspree.x,
			cg.hudRect.killspree.y,
			cg.hudRect.killspree.w,
			cg.hudRect.killspree.h,
			(cg.hudRect.killspree.hover) ? HUDEDITOR_HOVERWIDTH : HUDEDITOR_BORDERWIDTH,
			(cg.hudRect.killspree.hover) ? cg.hudEditor.hoverclr : cg.hudEditor.borderclr
		);

		if(trap_Milliseconds() - cg.hudEditor.ksTime > 1000 ) { 
			CG_AddKillSpreeItemBig( msg );
			cg.killspreePrintTime = cg.time - 5000 + 1200 + 200;
			cg.hudEditor.ksTime = trap_Milliseconds();
		}

	}
}



void CG_HudEditor_Draw() {
	float diff;
	int i;
	HudrectDef_t *testrect;

	Vector4Set(cg.hudEditor.borderclr, 0.0, 0.0, 0.0, 0.85);
	Vector4Set(cg.hudEditor.hoverclr, 0.15, 0.15, 0.15, 0.85);
	Vector4Set(cg.hudEditor.coorborder, 0.5f, 0.5f, 0.5f, 0.5f);
	Vector4Set(cg.hudEditor.coorbg, 0.0f, 0.0f, 0.0f, 0.6f);
	Vector4Set(cg.hudEditor.coortxt, 0.625f, 0.625f, 0.6f,  1.0f);

	diff = cg.hudEditor.fadeTime - trap_Milliseconds();

	// forty - do the cool fade in effect
	if(diff > 0.0f) {
		float scale = (diff / STATS_FADE_TIME);

		if(cg.hudEditor.showHudEditor)
			scale = 1.00f - scale;
		
		cg.hudEditor.borderclr[3] *= scale;
		cg.hudEditor.hoverclr[3] *= scale;
		cg.hudEditor.coorborder[3] *= scale;
		cg.hudEditor.coorbg[3] *= scale;
		cg.hudEditor.coortxt[3] *= scale;
	}

	if(cg.hudEditor.loading) {
		//temporarily leave the hud editor....
		CG_HudEditor_f();
		cg.hudEditor.loading = qfalse;
		trap_Cvar_Set("loadhudmenu", "");
		trap_UI_Popup(UIMENU_HUDEDITOR_LOAD);
		return;
	}
	
	if(cg.hudEditor.saving) {
		//temporarily leave the hud editor....
		CG_HudEditor_f();
		cg.hudEditor.saving = qfalse;
		trap_Cvar_Set("savehudmenu", "");
		trap_UI_Popup(UIMENU_HUDEDITOR_SAVE);
		return;
	}

	if( 1 ) {
		vec4_t	testbg =	{ 0.8f,	0.2f,	0.2f,	0.1f };

		CG_FillRect( 0, 0, 20, 480, testbg );
		CG_FillRect( 640 - 20, 0, 20, 480, testbg );
	}

	// forty - draw all our individual elements.
	CG_HudEditor_HealthBarDraw();
	CG_HudEditor_StaminaBarDraw();
	CG_HudEditor_ChargeBarDraw();
	CG_HudEditor_CompassDraw();
	CG_HudEditor_HpDraw();
	CG_HudEditor_XpDraw();
	CG_HudEditor_XpAddDraw();
	CG_HudEditor_DrawsDraw();
	CG_HudEditor_SkillDraw();
	CG_HudEditor_WeaponDraw();
	CG_HudEditor_FireTeamDraw();
	CG_HudEditor_AmmoDraw();
	CG_HudEditor_HeadDraw();
	CG_HudEditor_LagometerDraw();
	CG_HudEditor_FlagcovDraw();
	CG_HudEditor_PopupTextDraw();
	CG_HudEditor_ChatTextDraw();
	CG_HudEditor_VoteFTTextDraw();
	CG_HudEditor_LivesLeftDraw();

	CG_HudEditor_KillspreeDraw();

	// forty - draw the help
	CG_HudEditor_HelpDraw();

	// forty - draw the cursor
	CG_HudEditor_CursorDraw();
	
	// forty - turn off the uniform scaling at the end of the frame...
	testrect = &cg.hudRect.healthbar;
	for(i=0; i<sizeof(hudRect_t)/sizeof(HudrectDef_t); i++) {
		if(
			testrect->scaled == HUD_SCALE_UNIFORM_UP || 
			testrect->scaled == HUD_SCALE_UNIFORM_DOWN
		) {
			testrect->scaled = HUD_SCALE_NONE;
			testrect->scaledx = -1;
			testrect->scaledy = -1; 
		}
	}
}


void CG_UpdateHud() {
	CG_BlankHud(); // clear

	if(CG_LoadHud(cg_hud.string)) {
		//CG_Printf("^2Loaded HUD: %s\n", cg_hud.string);
		// prints in loadhud func
	} else if( (!Q_stricmp(cg_hud.string, "alt1")) || (!Q_stricmp(cg_hud.string, "althud1")) || cg_hud.integer == 1 ) {
		CG_AltHud1();
		CG_Printf("Alt HUD 1 loaded\n");
	} else if ( (!Q_stricmp(cg_hud.string, "alt2")) || (!Q_stricmp(cg_hud.string, "althud2")) || cg_hud.integer == 2 ) {
		CG_AltHud2();
		CG_Printf("Alt HUD 2 loaded\n");
	} else if ( (!Q_stricmp(cg_hud.string, "default")) || (!Q_stricmp(cg_hud.string, "def")) || cg_hud.integer <= 0 ) {
		CG_LoadDefaultHud();
		CG_Printf("default HUD loaded\n");
	} else {
		//CG_BlankHud();
		//CG_Printf("^1ERROR: HUD not found\n");
		CG_Printf("^1HUD not found\n");
		CG_Printf("^3Type ^7/loadhud ? ^3to show a list of available huds.\n");
		return;
	}


}
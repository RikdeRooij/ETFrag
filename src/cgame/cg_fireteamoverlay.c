/******************************************************************************
***** teh firetams! (sic)
****/

#include "cg_local.h"

/******************************************************************************
***** Defines, constants, etc 
****/

static int sortedFireTeamClients[MAX_CLIENTS];

/******************************************************************************
***** Support Routines
****/

int QDECL CG_SortFireTeam( const void *a, const void *b ) {
	clientInfo_t	*ca, *cb;
	int				cna, cnb;

	cna = *(int*)a;
	cnb = *(int*)b;

	ca = &cgs.clientinfo[cna];
	cb = &cgs.clientinfo[cnb];

	// Not on our team, so shove back
	if ( !CG_IsOnSameFireteam( cnb, cg.clientNum ) ) {
		return -1;
	}
	if ( !CG_IsOnSameFireteam( cna, cg.clientNum ) ) {
		return 1;
	}

	// Leader comes first
	if ( CG_IsFireTeamLeader( cna ) ) {
		return -1;
	}
	if ( CG_IsFireTeamLeader( cnb ) ) {
		return 1;
	}

	// Then higher ranks
	if ( ca->rank > cb->rank ) {
		return -1;
	}
	if ( cb->rank > ca->rank ) {
		return 1;
	}

	// Then score
/*	if ( ca->score > cb->score ) {
		return -1;
	}
	if ( cb->score > ca->score ) {
		return 1;
	}*/ // not atm

	return 0;
}

// Sorts client's fireteam by leader then rank
void CG_SortClientFireteam() {
	int i;

	for(i = 0; i < MAX_CLIENTS; i++) {
		sortedFireTeamClients[i] = i;
	}

	qsort( sortedFireTeamClients, MAX_CLIENTS, sizeof(sortedFireTeamClients[0]), CG_SortFireTeam );

/*	for(i = 0; i < MAX_CLIENTS; i++) {
		CG_Printf( "%i ", sortedFireTeamClients[i] );
	}

	CG_Printf( "\n" );*/
}

// Parses fireteam servercommand
void CG_ParseFireteams() {
	int i, j;
	char* s;
	const char* p;
	int clnts[2];

	//qboolean onFireteam2;
	//qboolean isLeader2;

//	qboolean onFireteam =	CG_IsOnFireteam( cg.clientNum ) ? qtrue : qfalse;
//	qboolean isLeader =		CG_IsFireTeamLeader( cg.clientNum ) ? qtrue : qfalse;

	for(i = 0; i < MAX_CLIENTS; i++) {
		cgs.clientinfo[i].fireteamData = NULL;
	}

	for(i = 0; i < MAX_FIRETEAMS; i++) {
		char hexbuffer[11] = "0x00000000";
		p = CG_ConfigString(CS_FIRETEAMS + i);
		
/*		s = Info_ValueForKey(p, "n");
		if(!s || !*s) {
			cg.fireTeams[i].inuse = qfalse;
			continue;
		} else {
			cg.fireTeams[i].inuse = qtrue;
		}*/

//		Q_strncpyz(cg.fireTeams[i].name, s, 32);
//		CG_Printf("Fireteam: %s\n", cg.fireTeams[i].name);

		j = atoi(Info_ValueForKey(p, "id"));
		if(j == -1) {
			cg.fireTeams[i].inuse = qfalse;
			continue;
		} else {
			cg.fireTeams[i].inuse = qtrue;
			cg.fireTeams[i].ident = j;
		}

		s = Info_ValueForKey(p, "l");
		cg.fireTeams[i].leader = atoi(s);

		s = Info_ValueForKey(p, "c");
		Q_strncpyz(hexbuffer+2, s, 9);
		sscanf(hexbuffer, "%x", &clnts[1]);
		Q_strncpyz(hexbuffer+2, s+8, 9);
		sscanf(hexbuffer, "%x", &clnts[0]);

		for(j = 0; j < MAX_CLIENTS; j++) {
			if(COM_BitCheck(clnts, j)) {
				cg.fireTeams[i].joinOrder[j] = qtrue;
				cgs.clientinfo[j].fireteamData = &cg.fireTeams[i];
//				CG_Printf("%s\n", cgs.clientinfo[j].name);
			} else {
				cg.fireTeams[i].joinOrder[j] = qfalse;
			}
		}
	}

	CG_SortClientFireteam();

//	onFireteam2 =	CG_IsOnFireteam( cg.clientNum ) ? qtrue : qfalse;
//	isLeader2 =		CG_IsFireTeamLeader( cg.clientNum ) ? qtrue : qfalse;
}

// Fireteam that both specified clients are on, if they both are on the same team
fireteamData_t* CG_IsOnSameFireteam( int clientNum, int clientNum2 ) {
	if(CG_IsOnFireteam(clientNum) == CG_IsOnFireteam(clientNum2)) {
		return CG_IsOnFireteam(clientNum);
	}

	return NULL;
}

// Fireteam that specified client is leader of, or NULL if none
fireteamData_t* CG_IsFireTeamLeader( int clientNum ) {
	fireteamData_t* f;

	if(!(f = CG_IsOnFireteam(clientNum))) {
		return NULL;
	}

	if(f->leader != clientNum) {
		return NULL;
	}

	return f ;
}

// Client, not on a fireteam, not sorted, but on your team
clientInfo_t* CG_ClientInfoForPosition(int pos, int max) {
	int i, cnt = 0;

	for(i = 0; i < MAX_CLIENTS && cnt < max; i++) {
		if(cg.clientNum != i && cgs.clientinfo[i].infoValid && !CG_IsOnFireteam( i ) && cgs.clientinfo[cg.clientNum].team == cgs.clientinfo[i].team ) {
			if(cnt == pos) {
				return &cgs.clientinfo[i];
			}
			cnt++;
		}
	}

	return NULL;
}

// Fireteam, that's on your same team
fireteamData_t* CG_FireTeamForPosition( int pos, int max ) {
	int i, cnt = 0;

	for(i = 0; i < MAX_FIRETEAMS && cnt < max; i++) {
		if(cg.fireTeams[i].inuse && cgs.clientinfo[cg.fireTeams[i].leader].team == cgs.clientinfo[cg.clientNum].team) {
			if(cnt == pos) {				
				return &cg.fireTeams[i];
			}
			cnt++;
		}
	}

	return NULL;
}

// Client, not sorted by rank, on CLIENT'S fireteam
clientInfo_t* CG_FireTeamPlayerForPosition(int pos, int max) {
	int i, cnt = 0;
	fireteamData_t* f = CG_IsOnFireteam(cg.clientNum);

	if(!f) {
		return NULL;
	}

	for(i = 0; i < MAX_CLIENTS && cnt < max; i++) {
		if(cgs.clientinfo[i].infoValid && cgs.clientinfo[cg.clientNum].team == cgs.clientinfo[i].team ) {
			if(!(f == CG_IsOnFireteam(i))) {
				continue;
			}

			if(cnt == pos) {
				return &cgs.clientinfo[i];
			}
			cnt++;
		}
	}

	return NULL;
}

// Client, sorted by rank, on CLIENT'S fireteam
clientInfo_t* CG_SortedFireTeamPlayerForPosition(int pos) {
	int i, cnt = 0;
	fireteamData_t* f = CG_IsOnFireteam(cg.clientNum);

	if(!f) {
		return NULL;
	}

	for(i = 0; i < MAX_CLIENTS && cnt < MAX_FIRETEAM_MEMBERS; i++) {
		if(!(f == CG_IsOnFireteam(sortedFireTeamClients[i]))) {
			return NULL;
		}

		if(cnt == pos) {
			return &cgs.clientinfo[sortedFireTeamClients[i]];
		}
		cnt++;
	}

	return NULL;
}

/******************************************************************************
***** Main Functions
****/

#define FT_BAR_YSPACING 2.f
#define FT_BAR_HEIGHT 10.f
#define FT_WIDTH 224
void CG_DrawFakeFireTeamOverlay( rectDef_t* rect ) {
	int x = rect->x;
	int y = rect->y + 1;	// +1, jitter it into place in 1024 :)
	float h;
	char buffer[64];
	int i;
	vec4_t clr1 =	{ .16f,		.2f,	.17f,	.8f };
	vec4_t clr2 =	{ 0.f,		0.f,		0.f,		.2f };
	vec4_t tclr =	{ 0.6f,		0.6f,		0.6f,		1.0f };
	vec4_t bgColor		= { 0.0f, 0.0f, 0.0f, 0.6f };		// window
	vec4_t borderColor	= { 0.5f, 0.5f, 0.5f, 0.5f };	// window

	bgColor[3] = cg_fireteamAlpha.value;

	h = 12 + 2 + 2;
	for(i = 0; i < MAX_FIRETEAM_MEMBERS; i++) {
		h += FT_BAR_HEIGHT + FT_BAR_YSPACING;
	}

	CG_DrawRect( x, y, FT_WIDTH, h, 1, borderColor);
	CG_FillRect( x + 1, y + 1, FT_WIDTH - 2, h - 2, bgColor);

	x += 2;
	y += 2;

	CG_FillRect( x, y, FT_WIDTH - 4, 12, clr1 );

	Com_sprintf( buffer, 64, "Fireteam: %s", "Sample" );
	//sprintf( buffer, "Fireteam: %s", "Sample" );
	Q_strupr( buffer );
	CG_Text_Paint_Ext( x + 3, y + FT_BAR_HEIGHT, .19f, .19f, tclr, buffer, 0, 0, 0, &cgs.media.limboFont1 );

	x += 2;

	for(i = 0; i < MAX_FIRETEAM_MEMBERS; i++) {
		y += FT_BAR_HEIGHT + FT_BAR_YSPACING;
		x = rect->x + 2;

		CG_FillRect( x, y + FT_BAR_YSPACING, FT_WIDTH - 4, FT_BAR_HEIGHT, clr2 );

		x += 4;

		CG_Text_Paint_Ext( x, y + FT_BAR_HEIGHT, .2f, .2f, tclr, "S", 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );	
		x += 10;

		CG_Text_Paint_Ext( x, y + FT_BAR_HEIGHT, .2f, .2f, tclr, "S", 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );	
		x += 22;

		CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT, .2f, .2f, tclr, "Sample", 0, 17, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
		x += 90;

		CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT,  .2f, .2f, tclr, "100", 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );

		{
			char *s;

			s = va( "^3(%s)", "X,Y");

			x = rect->x + ( FT_WIDTH - 4 - CG_Text_Width_Ext( s, .2f, 0, &cgs.media.font3 ) );

			CG_Text_Paint_Ext( x, y + FT_BAR_HEIGHT,  .2f, .2f, tclr, va( "^3(%s)", "X,Y" ), 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
		}
	}
}

void CG_DrawFireTeamOverlay( rectDef_t* rect ) {
	int x = rect->x;
	int y = rect->y + 1;	// +1, jitter it into place in 1024 :)
	int boxWidth = 204;
	int bestWidth = -1;
	char *locStr[MAX_FIRETEAM_MEMBERS];
	vec2_t loc;
	float h;
	clientInfo_t* ci = NULL;
	char buffer[64];
	fireteamData_t* f = NULL;
	int i;
	vec4_t clr1 =	{ .16f,		.2f,	.17f,	.8f };
	vec4_t clr2 =	{ 0.f,		0.f,		0.f,		.2f };
	vec4_t clr3 =	{ 0.25f,		0.f,		0.f,		153/255.f };
	vec4_t tclr =	{ 0.6f,		0.6f,		0.6f,		1.0f };
	vec4_t bgColor		= { 0.0f, 0.0f, 0.0f, 0.6f };		// window
	vec4_t borderColor	= { 0.5f, 0.5f, 0.5f, 0.5f };	// window
	centity_t*	cent;

	bgColor[3] = cg_fireteamAlpha.value;

	if(cg.hudEditor.showHudEditor) {
		//draw a fake fireteam box...
		CG_DrawFakeFireTeamOverlay(rect);
		return;
	} else 	if(!(f = CG_IsOnFireteam( cg.clientNum ))) {
		return;
	}

	h = 12 + 2 + 2;
	for(i = 0; i < MAX_FIRETEAM_MEMBERS; i++) {
		int		locwidth;
		vec3_t	origin;

		ci = CG_SortedFireTeamPlayerForPosition( i );
		if(!ci) {
			break;
		}

		h += FT_BAR_HEIGHT + FT_BAR_YSPACING;

		loc[0] = ci->location[0];
		loc[1] = ci->location[1];

		if(cg_locations.integer > 0) {
			qboolean locValid = qtrue;
			cent = &cg_entities[ci->clientNum];

			// Dens: use lerpOrigin for now
			origin[0] = cent->lerpOrigin[0];
			origin[1] = cent->lerpOrigin[1];
			origin[2] = cent->lerpOrigin[2];
	
			locStr[i] = va( "^3%s", CG_GetLocationMsg(origin));

			if (!Q_stricmp( locStr[i], "^3Unknown")){
				locStr[i] = va( "^3(%s)", BG_GetLocationString( loc ));
				locValid = qfalse;
			}

			if(cg_locations.integer > 1 && locValid)
				Q_strcat( locStr[i], 64, va(" ^3(%s)", BG_GetLocationString( loc )) );

		} else {
			locStr[i] = va( "^3(%s)", BG_GetLocationString( loc ));
		}

		if( !locStr[i][1] || !*locStr[i] )
			locStr[i] = " ";
	
		locwidth = CG_Text_Width_Ext( locStr[i], 0.2f, 0, &cgs.media.font3 );
	
		if(locwidth > bestWidth)
			bestWidth = locwidth;
	}

	boxWidth += bestWidth;

	CG_DrawRect( x, y, boxWidth, h, 1, borderColor);
	CG_FillRect( x + 1, y + 1, boxWidth - 2, h - 2, bgColor);

	x += 2;
	y += 2;

	CG_FillRect( x, y, boxWidth - 4, 12, clr1 );

	Com_sprintf( buffer, 64, "Fireteam: %s", bg_fireteamNames[f->ident] );
	//sprintf( buffer, "Fireteam: %s", bg_fireteamNames[f->ident] );
	Q_strupr( buffer );
	CG_Text_Paint_Ext( x + 3, y + FT_BAR_HEIGHT, .19f, .19f, tclr, buffer, 0, 0, 0, &cgs.media.limboFont1 );

	x += 2;
	//y += 2;

	for(i = 0; i < MAX_FIRETEAM_MEMBERS; i++) {
		y += FT_BAR_HEIGHT + FT_BAR_YSPACING;
		x = rect->x + 2;

		ci = CG_SortedFireTeamPlayerForPosition( i );
		if(!ci) {
			break;;
		}
		
		if( ci->selected ) {
			CG_FillRect( x, y + FT_BAR_YSPACING, boxWidth - 4, FT_BAR_HEIGHT, clr3 );
		} else {
			CG_FillRect( x, y + FT_BAR_YSPACING, boxWidth - 4, FT_BAR_HEIGHT, clr2 );
		}

		x += 4;
				
		//draw class
		if(cg_drawClassIcons.integer & CLASSICON_FIRETEAM){
			trap_R_SetColor( colorWhite );
			CG_DrawPic(x-2, y+2, FT_BAR_HEIGHT, FT_BAR_HEIGHT,
				cgs.media.skillPics[BG_ClassSkillForClass( ci->cls )]);
			trap_R_SetColor( NULL );
		}else{
			CG_Text_Paint_Ext( x, y + FT_BAR_HEIGHT, .2f, .2f, colorWhite, BG_ClassLetterForNumber( ci->cls ), 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );	
		}
		x += 10;
		
		// quad: draw latched class
		// pheno: not with an old server, that doesn't send the latched class
		if( ci->cls != ci->latchClass ) {
			//draw separator
			CG_Text_Paint_Ext( x, y + FT_BAR_HEIGHT, .2f, .2f, colorYellow, ">", 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
			x += 10;
			
			//draw class
			if(cg_drawClassIcons.integer & CLASSICON_FIRETEAM) {
				trap_R_SetColor( colorYellow );
				CG_DrawPic(x - 2, y + 2, FT_BAR_HEIGHT, FT_BAR_HEIGHT,
					cgs.media.skillPics[BG_ClassSkillForClass(ci->latchClass)]);
				trap_R_SetColor(NULL);
			} else {
				CG_Text_Paint_Ext( x, y + FT_BAR_HEIGHT, .2f, .2f, colorYellow, BG_ClassLetterForNumber( ci->latchClass ), 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
			}
			x += 10;
		}
		else
			x += 20;
		
		CG_Text_Paint_Ext( x, y + FT_BAR_HEIGHT, .2f, .2f, tclr, ci->team == TEAM_AXIS ? miniRankNames_Axis[ci->rank] : miniRankNames_Allies[ci->rank], 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );	
		x += 22;
		
		CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT, .2f, .2f, tclr, ci->name, 0, 17, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
		x += 90;

/*		CG_DrawPic(x + 2, y + 2, FT_BAR_HEIGHT - 4, FT_BAR_HEIGHT - 4, cgs.media.movementAutonomyIcons[0]);
		x += FT_BAR_HEIGHT;

		CG_DrawPic(x + 2, y + 2, FT_BAR_HEIGHT - 4, FT_BAR_HEIGHT - 4, cgs.media.weaponAutonomyIcons[0]);
		x += FT_BAR_HEIGHT;
		x += 4;*/

/*		if( isLeader ) {
			CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT, .2f, .2f, tclr, va("%i", i+4), 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
		}*/
		x += 20;
		
		if( ci->health > 80 ) {
			CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT,  .2f, .2f, tclr, va("%i", ci->health < 0 ? 0 : ci->health ), 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
		} else if( ci->health > 0 ) {
			CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT,  .2f, .2f, colorYellow, va("%i", ci->health < 0 ? 0 : ci->health ), 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
		} else {
			CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT,  .2f, .2f, colorRed, va("0%s", ci->health < 0 ? "" : "*" ), 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
		}
		x += 30;

		CG_Text_Paint_Ext( x, y + FT_BAR_HEIGHT,  .2f, .2f, tclr, locStr[i], 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
	}
}

qboolean CG_FireteamGetBoxNeedsButtons() {
	if ( cgs.applicationEndTime > cg.time ) {
		if( cgs.applicationClient < 0 ) {
			return qfalse;
		}
		return qtrue;
	}

	if ( cgs.invitationEndTime > cg.time ) {
		if( cgs.invitationClient < 0 ) {
			return qfalse;
		}
		return qtrue;
	}

	if ( cgs.propositionEndTime > cg.time ) {
		if( cgs.propositionClient < 0 ) {
			return qfalse;
		}
		return qtrue;
	}

	return qfalse;
}

const char* CG_FireteamGetBoxText() {
	if ( cgs.applicationEndTime > cg.time ) {
		if ( cgs.applicationClient == -1 ) {
			return "Sent";
		}

		if ( cgs.applicationClient == -2 ) {
			return "Failed";
		}

		if ( cgs.applicationClient == -3 ) {
			return "Accepted";
		}

		if ( cgs.applicationClient == -4 ) {
			return "Sent";
		}

		if( cgs.applicationClient < 0 ) {
			return NULL;
		}

		return va("Accept application from %s?", cgs.clientinfo[cgs.applicationClient].name);
	}

	if ( cgs.invitationEndTime > cg.time ) {
		if ( cgs.invitationClient == -1 ) {
			return "Sent";
		}

		if ( cgs.invitationClient == -2 ) {
			return "Failed";
		}

		if ( cgs.invitationClient == -3 ) {
			return "Accepted";
		}

		if ( cgs.invitationClient == -4 ) {
			return "Sent";
		}

		if( cgs.invitationClient < 0 ) {
			return NULL;
		}

		return va("Accept invitiation from %s?", cgs.clientinfo[cgs.invitationClient].name);
	}

	if ( cgs.propositionEndTime > cg.time ) {
		if ( cgs.propositionClient == -1 ) {
			return "Sent";
		}

		if ( cgs.propositionClient == -2 ) {
			return "Failed";
		}

		if ( cgs.propositionClient == -3 ) {
			return "Accepted";
		}

		if ( cgs.propositionClient == -4 ) {
			return "Sent";
		}

		if( cgs.propositionClient < 0 ) {
			return NULL;
		}

		return va( "Accept %s's proposition to invite %s to join your fireteam?", cgs.clientinfo[cgs.propositionClient2].name, cgs.clientinfo[cgs.propositionClient].name);
	}

	return NULL;
}

qboolean CG_FireteamHasClass( int classnum, qboolean selectedonly ) {
	fireteamData_t* ft;
	int i;

	if( !(ft = CG_IsOnFireteam( cg.clientNum )) ) {
		return qfalse;
	}

	for( i = 0; i < MAX_CLIENTS; i++ ) {
/*		if( i == cgs.clientinfo ) {
			continue;
		}*/

		if( !cgs.clientinfo[ i ].infoValid ) {
			continue;
		}

		if( ft != CG_IsOnFireteam( i ) ) {
			continue;
		}

		if( cgs.clientinfo[ i ].cls != classnum ) {
			continue;
		}

		if( selectedonly && !cgs.clientinfo[ i ].selected ) {
			continue;
		}

		return qtrue;
	}

	return qfalse;
}

const char* CG_BuildSelectedFirteamString( void ) {
	char buffer[256];
	clientInfo_t* ci;
	int cnt = 0;
	int i;

	*buffer = '\0';
	for( i = 0; i < MAX_FIRETEAM_MEMBERS; i++ ) {
		ci = CG_SortedFireTeamPlayerForPosition( i );
		if( !ci ) {
			break;
		}

		if( !ci->selected ) {
			continue;
		}

		cnt++;

		Q_strcat( buffer, sizeof( buffer ), va( "%i ", ci->clientNum ) );
	}

	if( cnt == 0 ) {
		return "0";
	}

	if( !cgs.clientinfo[cg.clientNum].selected ) {
		Q_strcat( buffer, sizeof( buffer ), va( "%i ", cg.clientNum ) );
		cnt++;
	}
	
	return va( "%i %s", cnt, buffer );
}



int CG_GetFireTeamHeight( clientInfo_t* ci ) {
	int i, h;

	//h = 12 + 2 + 2;
	h = 0;
	for(i = 0; i < MAX_FIRETEAM_MEMBERS; i++) {
		if( !cg.hudEditor.showHudEditor ) { // hudeditor is full fireteam
			ci = CG_SortedFireTeamPlayerForPosition( i );
			if(!ci) {
				break;;
			}
		}

		h += FT_BAR_HEIGHT + FT_BAR_YSPACING;
	}
	// if fireteam 'exists' add 16
	if (h > 0) { h += 16 + 4; } // fireteam without members size
	else { h = 0; } // just in case :)

	return h;
}

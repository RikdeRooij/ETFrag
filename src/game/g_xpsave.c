#include "g_local.h"
#include "g_etbot_interface.h"

g_xpsave_t *g_xpsaves[MAX_XPSAVES];

// josh: this is static because is makes it easier to handle
// reconnects. If you have a better idea, like a 2-way linked list,
// I'm all for that. I was just being lazy and simplistic.
g_disconnect_t g_disconnects[MAX_DISCONNECTS];

/*
 * gabriel: Get g_XPSaveMaxAge with applied multipliers (if any)
 */
int G_getXPSaveMaxAge() {
	int result = g_XPSaveMaxAge.integer;

	if (*g_XPSaveMaxAge.string) {
		switch(g_XPSaveMaxAge.string[strlen(g_XPSaveMaxAge.string) - 1]) {
			case 'O':
			case 'o':
				result *= 4;

			case 'W':
			case 'w':
				result *= 7;

			case 'D':
			case 'd':
				result *= 24;

			case 'H':
			case 'h':
				result *= 60;

			case 'M':
			case 'm':
				result *= 60;

				break;
		}
	}

	return result;
}

/*
 * gabriel: Get g_XPSaveMaxAge_xp with applied multipliers (if any)
 */
int G_getXPSaveMaxAge_xp() {
	int result = g_XPSaveMaxAge_xp.integer;

	if (*g_XPSaveMaxAge_xp.string) {
		switch(g_XPSaveMaxAge_xp.string[strlen(g_XPSaveMaxAge_xp.string) - 1]) {
			case 'O':
			case 'o':
				result *= 4;

			case 'W':
			case 'w':
				result *= 7;

			case 'D':
			case 'd':
				result *= 24;

			case 'H':
			case 'h':
				result *= 60;

			case 'M':
			case 'm':
				result *= 60;

				break;
		}
	}

	return result;
}

void G_xpsave_writeconfig() 
{
	fileHandle_t f;
	int len, i, j;
	time_t t;
	int age = 0;
	int eff_XPSaveMaxAge_xp = G_getXPSaveMaxAge_xp();
	int eff_XPSaveMaxAge = G_getXPSaveMaxAge();

	if(!(g_XPSave.integer & XPSF_ENABLE))
		return;	
	if(!g_XPSaveFile.string[0])
		return;
	time(&t);
	len = trap_FS_FOpenFile(g_XPSaveFile.string, &f, FS_WRITE);
	if(len < 0) {
		G_Printf("G_xpsave_writeconfig: could not open %s\n",
				g_XPSaveFile.string);
 	}

	for(i=0; g_xpsaves[i]; i++) {
		if(!g_xpsaves[i]->time)
			continue;
		age = t - g_xpsaves[i]->time;
		if(age > eff_XPSaveMaxAge) {
			continue;
 		}
 
 		trap_FS_Write("[xpsave]\n", 9, f);
		trap_FS_Write("guid              = ", 20, f);
 		G_shrubbot_writeconfig_string(g_xpsaves[i]->guid, f);
		trap_FS_Write("name              = ", 20, f);
 		G_shrubbot_writeconfig_string(g_xpsaves[i]->name, f);
		trap_FS_Write("time              = ", 20, f);
 		G_shrubbot_writeconfig_int(g_xpsaves[i]->time, f);
 		if(age <= eff_XPSaveMaxAge_xp) {
 			for(j=0; j<SK_NUM_SKILLS; j++) {
 				//if(g_xpsaves[i]->skill[j] == 0.0f)
 				//	continue;
				trap_FS_Write(va("skill[%i]          = ", j),
					20, f);
 				G_shrubbot_writeconfig_float(
 					g_xpsaves[i]->skill[j], f);
 			}
 		}
 		if(g_xpsaves[i]->kill_rating != 0.0f) { 
			trap_FS_Write("kill_rating       = ", 20, f);
 			G_shrubbot_writeconfig_float(g_xpsaves[i]->kill_rating,
 				f);
 		}
 		if(g_xpsaves[i]->mutetime) {
			trap_FS_Write("mutetime          = ", 20, f);
 			G_shrubbot_writeconfig_int(g_xpsaves[i]->mutetime, f);
 		}
		trap_FS_Write("\n", 1, f);
	}
	G_Printf("xpsave: wrote %d xpsaves\n", i);
	trap_FS_FCloseFile(f);
}

void G_xpsave_readconfig() 
{
	g_xpsave_t *x = g_xpsaves[0];
	int xc = 0;
	fileHandle_t f;
	int i;
	int len;
	char *cnf, *cnf2;
	char *t;
	float skill;
	qboolean xpsave_open;

	if(!(g_XPSave.integer & XPSF_ENABLE))
		return;	
	if(!g_XPSaveFile.string[0])
		return;
	len = trap_FS_FOpenFile(g_XPSaveFile.string, &f, FS_READ) ; 
	if(len < 0) {
		G_Printf("readconfig: could not open xpsave file %s\n",
			g_XPSaveFile.string);
		return;
	}
	cnf = malloc(len+1);
	cnf2 = cnf;
	trap_FS_Read(cnf, len, f);
	*(cnf + len) = '\0';
	trap_FS_FCloseFile(f);

	G_xpsave_cleanup();
	
	t = COM_Parse(&cnf);
	xpsave_open = qfalse;
	while(*t) {
		if(!Q_stricmp(t, "[xpsave]")
			) {

			if(xpsave_open)
				g_xpsaves[xc++] = x;
			xpsave_open = qfalse;
		}

		if(xpsave_open) {
			if(!Q_stricmp(t, "guid")) {
				G_shrubbot_readconfig_string(&cnf, 
					x->guid, sizeof(x->guid)); 
			}
			else if(!Q_stricmp(t, "name")) {
				G_shrubbot_readconfig_string(&cnf, 
					x->name, sizeof(x->name)); 
			}
			else if(!Q_stricmp(t, "time")) {
				G_shrubbot_readconfig_int(&cnf, &x->time);
			}
			else if(!Q_stricmpn(t, "skill[", 6)) {
				for(i=0; i<SK_NUM_SKILLS; i++) {
					if(Q_stricmp(t, va("skill[%i]", i))) {
						x->skill[i] = 0.0f;
						continue;
					}
					G_shrubbot_readconfig_float(&cnf,
						&skill);
					x->skill[i] = skill;
					break;
				}
			}
			else if(!Q_stricmp(t, "kill_rating")) {
				G_shrubbot_readconfig_float(&cnf, &x->kill_rating);
			}
			else if(!Q_stricmp(t, "kill_variance")) {
				G_shrubbot_readconfig_float(&cnf, &x->kill_variance);
			}
			else if(!Q_stricmp(t, "mutetime")) {
				G_shrubbot_readconfig_int(&cnf, &x->mutetime);
			}
			else {
				G_Printf("xpsave: [xpsave] parse error near "
					"%s on line %d\n", 
					t, 
					COM_GetCurrentParseLine());
			}
		}

		if(!Q_stricmp(t, "[xpsave]")) {
			if(xc >= MAX_XPSAVES) {
				G_Printf("xpsave: error MAX_XPSAVES exceeded");
				return;
			}
			x = malloc(sizeof(g_xpsave_t));
			x->guid[0] = '\0';
			x->name[0] = '\0';
			x->kill_rating = 0.0f;
			x->kill_variance = SIGMA2_DELTA;
			x->mutetime = 0;
			x->hits = 0;
			x->team_hits = 0;
			x->time = 0;
			xpsave_open = qtrue;
		}
		t = COM_Parse(&cnf);
	}
	if(xpsave_open)
		g_xpsaves[xc++] = x;

	free(cnf2);
	G_Printf("xpsave: loaded %d xpsaves\n", xc);
}
void AddDisconnect(gentity_t *ent
	,g_xpsave_t *xpsave
	,int axis_time
	,int allies_time
	,team_t last_playing_team
	,float killrating
    ,float killvariance) {

	int i;

	for (i = 0; i < MAX_DISCONNECTS; i++) {
		if (g_disconnects[i].xpsave == NULL) {
			g_disconnects[i].xpsave = xpsave;
			g_disconnects[i].axis_time = axis_time;
			g_disconnects[i].allies_time = allies_time;
			g_disconnects[i].last_playing_team = last_playing_team;
			g_disconnects[i].killrating = killrating;
			g_disconnects[i].killvariance = killvariance;
			g_disconnects[i].lives = -999;
			// tjw: usually going to be -999 unless the client
			//      reconnected more than once without joining
			//      a team.
			if(!ent->client->maxlivescalced)  {
				g_disconnects[i].lives =
					ent->client->disconnectLives;
			}
			else if(g_maxlives.integer || 
				g_alliedmaxlives.integer || 
				g_axismaxlives.integer) {

				g_disconnects[i].lives =
				(ent->client->ps.persistant[PERS_RESPAWNS_LEFT]
					- 1);
				if(g_disconnects[i].lives < 0)
					g_disconnects[i].lives = 0;
			}

			G_Printf("Added DisconnectRecord guid %s lives %d\n",
				ent->client->sess.guid,
				g_disconnects[i].lives);
			return;
		}
	}
}

void Reconnect(g_xpsave_t *connect, gentity_t *ent) {
	int i;
	for (i = 0; i < MAX_DISCONNECTS; i++) {
		if (g_disconnects[i].xpsave == connect) {
			g_disconnects[i].xpsave = NULL;
			ent->client->sess.mapAxisTime =
				g_disconnects[i].axis_time;
			ent->client->sess.mapAlliesTime =
			g_disconnects[i].allies_time;
			ent->client->sess.last_playing_team =
				g_disconnects[i].last_playing_team;
			ent->client->sess.overall_killrating =
				g_disconnects[i].killrating;
			ent->client->sess.overall_killvariance =
				g_disconnects[i].killvariance;
			ent->client->disconnectLives =
				g_disconnects[i].lives;
			G_Printf("Found DisconnectRecord guid %s lives %d\n",
				ent->client->sess.guid,
				ent->client->disconnectLives);
			return;
		}
	}
}

void G_reset_disconnects() {
	int i;
	for (i = 0; i < MAX_DISCONNECTS; i++) {
		g_disconnects[i].xpsave = NULL;
	}
}

qboolean G_xpsave_add(gentity_t *ent,qboolean disconnect)
{
	int i = 0;
	int j = 0;
	//int k = 0;
	char guid[33];
	char name[MAX_NAME_LENGTH] = {""};
	//int clientNum;
	qboolean found = qfalse;
	g_xpsave_t *x = g_xpsaves[0];
	time_t t;

	if(!(g_XPSave.integer & XPSF_ENABLE))
		return qfalse;	
	if(!ent || !ent->client)
		return qfalse;
	if(!time(&t))
		return qfalse;
	if(ent->client->pers.connected != CON_CONNECTED)
		return qfalse;
#ifdef GS_OMNIBOT
	if ((g_OmniBotFlags.integer & OBF_DONT_XPSAVE) && 
		(ent->r.svFlags & SVF_BOT))
		return qfalse;
#endif

//	clientNum = ent - g_entities;
	// tjw: some want raw name
	//SanitizeString(ent->client->pers.netname, name, qtrue);
	Q_strncpyz(name, ent->client->pers.netname, sizeof(name));
	Q_strncpyz(guid, ent->client->sess.guid, sizeof(guid));
	
	if(!guid[0] || strlen(guid) != 32)
		return qfalse;

	for(i=0; g_xpsaves[i]; i++) {
		if(!Q_stricmp(g_xpsaves[i]->guid, guid)) {
			x = g_xpsaves[i];
			found = qtrue;
			break;
		}
	}
	if(!found) {
		if(i == MAX_XPSAVES) {
			G_Printf("xpsave: cannot save. MAX_XPSAVES exceeded");
			return qfalse;
		}
		x = malloc(sizeof(g_xpsave_t));
		x->guid[0] = '\0';
		x->name[0] = '\0';
		x->kill_rating = 0.0f;
		x->kill_variance = SIGMA2_DELTA;
		for(j=0; j<SK_NUM_SKILLS; j++) {
			x->skill[j] = 0.0f;
		}
		x->mutetime = 0;
		x->hits = 0;
		x->team_hits = 0;
		x->time = 0;
		g_xpsaves[i] = x;
	}
	Q_strncpyz(x->guid, guid, sizeof(x->guid));
	Q_strncpyz(x->name, name, sizeof(x->name));
	x->time = t;
	for(i=0; i<SK_NUM_SKILLS; i++) {
		x->skill[i] = ent->client->sess.skillpoints[i];
	}
	x->kill_rating = ent->client->sess.overall_killrating;
	x->kill_variance = ent->client->sess.overall_killvariance;

	if(ent->client->sess.auto_unmute_time <= -1){
		x->mutetime = -1;
	}else if(ent->client->sess.auto_unmute_time == 0){
		x->mutetime = 0;
	}else{
		x->mutetime = ((ent->client->sess.auto_unmute_time-level.time)/1000+t);
	}

	x->hits = ent->client->sess.hits;
	x->team_hits = ent->client->sess.team_hits;

	if (disconnect) {
		AddDisconnect(ent
			,x
			,ent->client->sess.mapAxisTime
			,ent->client->sess.mapAlliesTime
			,ent->client->sess.last_playing_team
			,ent->client->sess.overall_killrating
			,ent->client->sess.overall_killvariance
		);
	}
	return qtrue;
}

qboolean G_xpsave_load(gentity_t *ent)
{
	int i;
	qboolean found = qfalse, XPSMuted = qfalse;
	//int clientNum;
	g_xpsave_t *x = g_xpsaves[0];
	time_t t;
	char agestr[MAX_STRING_CHARS];
	//char desc[64];
	// josh: Increased this
	// josh: TODO: tjw? What is this desc thing for?
	char desc[115];
	int age;
	int eff_XPSaveMaxAge_xp = G_getXPSaveMaxAge_xp();
	int eff_XPSaveMaxAge = G_getXPSaveMaxAge();
	float startxptotal = 0.0f;

	if(!ent || !ent->client)
		return qfalse;
	if(!(g_XPSave.integer & XPSF_ENABLE))
		return qfalse;	
	if(!time(&t))
		return qfalse;

	desc[0] = '\0';
//	clientNum = ent - g_entities;
	
	for(i=0; g_xpsaves[i]; i++) {
		if(!Q_stricmp(g_xpsaves[i]->guid, 
			ent->client->sess.guid)) {
			found = qtrue;
			x = g_xpsaves[i];
			break;
		}
	}
	if(!found)
		return qfalse;

	age = t - x->time;
	if(age > eff_XPSaveMaxAge) {
		return qfalse;
	}

	if(age <= eff_XPSaveMaxAge_xp) {
		for(i=0; i<SK_NUM_SKILLS; i++) {
			ent->client->sess.skillpoints[i] = x->skill[i];
			// pheno: fix for session startxptotal value
			startxptotal += x->skill[i];
		}
		ent->client->sess.startxptotal = startxptotal;
		ent->client->ps.stats[STAT_XP] = 
			(int)ent->client->sess.startxptotal;
		Q_strcat(desc, sizeof(desc), "XP/");
		if((g_XPDecay.integer & XPDF_ENABLE) &&
			!(g_XPDecay.integer & XPDF_NO_DISCONNECT_DECAY)) {
			G_XPDecay(ent, age, qtrue);
		}
	}

	ent->client->sess.overall_killrating = x->kill_rating;
	ent->client->sess.overall_killvariance = x->kill_variance;

	Q_strcat(desc, sizeof(desc), "ratings/");
	
	if(x->mutetime != 0) {
		if(x->mutetime < 0){
			ent->client->sess.auto_unmute_time = -1;
			XPSMuted = qtrue;
		}else if(x->mutetime > t){
			ent->client->sess.auto_unmute_time = (level.time + 1000*(x->mutetime - t));
			XPSMuted = qtrue;;
		}

		if(XPSMuted == qtrue){
			CP("print \"^5You've been muted by XPSave\n\"" );
			Q_strcat(desc, sizeof(desc), "mute/");
		}
	}

	ent->client->sess.hits = x->hits;
	ent->client->sess.team_hits = x->team_hits;
	G_CalcRank(ent->client);
	BG_PlayerStateToEntityState(&ent->client->ps,
			&ent->s,
			level.time,
			qtrue);
	// tjw: trim last / from desc
	if(strlen(desc))
		desc[strlen(desc)-1] = '\0';
	G_shrubbot_duration(age, agestr, sizeof(agestr)); 
	CP(va(
		"print \"^3server: loaded stored ^7%s^3 state from %s ago\n\"",
		desc,
		agestr));

	// josh: check and update if disconnect found
	// cs: not for bots since it is pointless for them, plus we never want SetTeam failing for them
	if (!(ent->r.svFlags & SVF_BOT)) {
		Reconnect(x,ent);
	}
	return qtrue;
}

/*
void G_xpsave_clear()
{
	int i;
	for(i=0; g_xpsaves[i]; i++) {
		free(g_xpsaves[i]);
		g_xpsaves[i] = NULL;
	}
	G_xpsave_writeconfig();
}
*/

void G_xpsave_resetxp()
{
	int i,j;
	for(i=0; g_xpsaves[i]; i++) {
		for(j=0; j<SK_NUM_SKILLS; j++) {
			g_xpsaves[i]->skill[j] = 0.0f;
		}
		g_xpsaves[i]->hits = 0;
		g_xpsaves[i]->team_hits = 0;

	}
}

g_disconnect_t *G_xpsave_disconnect(int i) {
	return &g_disconnects[i];
}

void G_xpsave_cleanup()
{
	int i = 0;

	for(i=0; g_xpsaves[i]; i++) {
		free(g_xpsaves[i]);
		g_xpsaves[i] = NULL;
	}
}

// pheno
void G_xpsave_writexp()
{
	int i = 0;

	for( i = 0; i < level.numConnectedClients; i++ ) {
		G_xpsave_add( &g_entities[level.sortedClients[i]], qfalse );
	}

	G_xpsave_writeconfig();
}

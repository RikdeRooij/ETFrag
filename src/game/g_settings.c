#include "g_local.h"

//g_killingSpree_t *g_killingSprees[MAX_KILLSPREES];
//g_ks_end_t *g_ks_ends[MAX_KS_ENDS];
//g_multiKill_t *g_multiKills[MAX_MULTIKILLS];
g_banner_t *g_banners[MAX_BANNERS];
//g_reviveSpree_t *g_reviveSprees[MAX_REVIVESPREES];



void G_banners_readconfig() 
{
	g_banner_t *b = g_banners[0];
	int bc = 0;
	fileHandle_t f;
	int len;
	char *cnf, *cnf2;
	char *t;
	//qboolean banner_open;
	int i = 0;

	for(i=0; g_banners[i]; i++) {
		free(g_banners[i]);
		g_banners[i] = NULL;
	}


	if(!g_bannersFile.string[0]){
		return;
	}

	len = trap_FS_FOpenFile(g_bannersFile.string, &f, FS_READ) ; 
	if(len < 0) {
		G_Printf("G_banners_readconfig: could not open banners file %s\n", g_bannersFile.string);
		return;
	}

	cnf = malloc(len+1);
	cnf2 = cnf;
	trap_FS_Read(cnf, len, f);
	*(cnf + len) = '\0';
	trap_FS_FCloseFile(f);
	
	t = COM_Parse(&cnf);
	//banner_open = qfalse;


	while(*t) {
		if(bc >= MAX_BANNERS) {
			G_Printf("settings: error MAX_BANNERS exceeded");
			return;
		}
		b = malloc(sizeof(g_banner_t));
		b->message[0] = '\0';

		//G_shrubbot_readconfig_line(&cnf, b->message, sizeof(b->message)); 
		while(t[0]) {
			if((b->message[0] == '\0' && strlen(t) <= sizeof(b->message)) ||
				(strlen(t)+strlen(b->message) < sizeof(b->message))) {

				Q_strcat(b->message, sizeof(b->message), t);
				Q_strcat(b->message, sizeof(b->message), " ");
			}
			t = COM_ParseExt(&cnf, qfalse);
		}
		// trim the trailing space
		if(strlen(b->message) > 0 && b->message[strlen(b->message)-1] == ' ')
			b->message[strlen(b->message)-1] = '\0';

		//G_Printf("^6*^8%s\n", b->message);

		g_banners[bc++] = b;

		t = COM_Parse(&cnf);
	}
	free(cnf2);

	G_Printf("readbanners: loaded %d banners\n", bc);
/*
	while(*t) {
		if( !Q_stricmp(t, "[banner]") ) {

			if(banner_open){
				g_banners[bc++] = b;
			}

			banner_open = qfalse;
		}

		
		if(banner_open) {
			if(!Q_stricmp(t, "message")) {
				G_shrubbot_readconfig_string(&cnf, 
					b->message, sizeof(b->message)); 
			} else {
				G_Printf("settings: [banner] parse error near "
					"%s on line %d\n", 
					t, 
					COM_GetCurrentParseLine());
			}
		}


		if(!Q_stricmp(t, "[banner]")) {
			if(bc >= MAX_BANNERS) {
				G_Printf("settings: error MAX_BANNERS exceeded");
				return;
			}
			b = malloc(sizeof(g_banner_t));
			b->message[0] = '\0';
			//b->wait = 0;
			//b->position[0] = '\0';
			banner_open = qtrue;
		}

		t = COM_Parse(&cnf);
	}
	
	if(banner_open){
		g_banners[bc++] = b;
	}

	free(cnf2);

	//G_Printf("settings: loaded %d sprees, %d ends, %d kills, %d banners and %d revive sprees\n"
	//	, sc, ec, kc, bc, rc);
	G_Printf("settings: loaded %d banners\n", bc);*/
}
/*
// Dens: moved from g_combat.c
char *G_KillSpreeSanitize(char *text)
{
	static char n[MAX_SAY_TEXT] = {""};

	if(!*text)
		return n;
	Q_strncpyz(n, text, sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[a]", "(a)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[v]", "(v)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[n]", "(n)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[d]", "(d)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[k]", "(k)"), sizeof(n));
	return n;
}
*/
void G_displayBanner(int loop)
{
	int nextBanner;

	if(level.lastBanner < 0 || level.lastBanner >= (MAX_BANNERS - 1)){
		level.lastBanner = -1;
	}
	// Dens: prevent noob admins causing an overflow
	if(loop >= MAX_BANNERS){
		level.nextBannerTime = (level.time + 60000);
		return;
	}

	if(g_banners[level.lastBanner+1]){
		nextBanner = (level.lastBanner + 1);
	}else if(level.lastBanner != -1 && g_banners[0]){
		nextBanner = 0;
	}else{
		// Dens: no banners found, try again in 10 secs
		level.nextBannerTime = (level.time + 10000);
		return;
	}

	if(g_banners[nextBanner]->message[0]){
		/*if(g_banners[nextBanner]->position[0]){
			if(!Q_stricmp(g_banners[nextBanner]->position,"center") 
				|| !Q_stricmp(g_banners[nextBanner]->position,"cp")){
				AP(va("cp \"%s\"",g_banners[nextBanner]->message));
			}else if(!Q_stricmp(g_banners[nextBanner]->position,"popup") 
				|| !Q_stricmp(g_banners[nextBanner]->position,"cpm")){
				AP(va("cpm \"%s\"",g_banners[nextBanner]->message));
			}else if(!Q_stricmp(g_banners[nextBanner]->position,"banner") 
				|| !Q_stricmp(g_banners[nextBanner]->position,"bp")){
				AP(va("bp \"%s\"",g_banners[nextBanner]->message));
			}else if(!Q_stricmp(g_banners[nextBanner]->position,"console") 
				|| !Q_stricmp(g_banners[nextBanner]->position,"print")){
				AP(va("print \"%s\n\"",g_banners[nextBanner]->message));
			}else{
				AP(va("chat \"%s\"",g_banners[nextBanner]->message));
			}
		}else{
			AP(va("chat \"%s\"",g_banners[nextBanner]->message));
		}*/
		AP(va("bp \"%s\"",g_banners[nextBanner]->message));
	}

	level.lastBanner = nextBanner;
/*
	if(g_banners[nextBanner]->wait > 0){
		level.nextBannerTime = (level.time + 1000*g_banners[nextBanner]->wait);
	}else if(g_banners[nextBanner]->wait == 0){
		G_displayBanner(loop+1);
	}else{
		// Dens: bad config, show next banner in 10 secs
		level.nextBannerTime = (level.time + 10000);
	}*/
	// 20 seconds
	level.nextBannerTime = (level.time + 20000);

	return;
}
/*
void G_check_revive_spree(gentity_t *ent, int number)
{
	int i;
	char name[MAX_NAME_LENGTH] = {"*unknown*"};
	char *output;

	for(i=0; g_reviveSprees[i]; i++) {
		if(!g_reviveSprees[i]->number) {
		 	continue;
		}

		if(g_reviveSprees[i]->number == number) {
			if(g_reviveSprees[i]->message[0]) {
				Q_strncpyz(name,
					G_KillSpreeSanitize(ent->client->pers.netname),
					sizeof(name));
				output = Q_StrReplace(g_reviveSprees[i]->message, "[n]", name);

				if(g_reviveSprees[i]->display[0] && !Q_stricmp(g_reviveSprees[i]->display,"player")){
					if(g_reviveSprees[i]->position[0]){

						if(!Q_stricmp(g_reviveSprees[i]->position,"center")
							|| !Q_stricmp(g_reviveSprees[i]->position,"cp")){
							CP(va("cp \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"popup")
							|| !Q_stricmp(g_reviveSprees[i]->position,"cpm")){
							CP(va("cpm \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"banner")
							|| !Q_stricmp(g_reviveSprees[i]->position,"bp")){
							CP(va("bp \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"console")
							|| !Q_stricmp(g_reviveSprees[i]->position,"print")){
							CP(va("print \"%s\n\"", output));
						}else{
							CP(va("chat \"%s\"",output));
						}
					}else{
						CP(va("chat \"%s\"",output));
					}
				}else{
					if(g_reviveSprees[i]->position[0]){

						if(!Q_stricmp(g_reviveSprees[i]->position,"center")
							|| !Q_stricmp(g_reviveSprees[i]->position,"cp")){
							AP(va("cp \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"popup")
							|| !Q_stricmp(g_reviveSprees[i]->position,"cpm")){
							AP(va("cpm \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"banner")
							|| !Q_stricmp(g_reviveSprees[i]->position,"bp")){
							AP(va("bp \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"console")
							|| !Q_stricmp(g_reviveSprees[i]->position,"print")){
							AP(va("print \"%s\n\"", output));
						}else{
							AP(va("chat \"%s\"",output));
						}
					}else{
						AP(va("chat \"%s\"",output));
					}
				}
			}
			if(g_reviveSprees[i]->sound[0]){
				if(g_reviveSprees[i]->play[0] && !Q_stricmp(g_reviveSprees[i]->play,"envi")){
					G_AddEvent(ent, EV_GENERAL_SOUND,G_SoundIndex(va("%s",g_reviveSprees[i]->sound)));
				}else if(g_reviveSprees[i]->play[0] && !Q_stricmp(g_reviveSprees[i]->play,"player")){
					G_ClientSound( ent,
						G_SoundIndex( g_reviveSprees[i]->sound ) );
				}else{
					G_globalSound(g_reviveSprees[i]->sound);
				}
			}
		}
	}
	return;
}
*/
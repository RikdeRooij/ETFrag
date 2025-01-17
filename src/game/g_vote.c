// g_vote.c: All callvote handling
// -------------------------------
//
#include "g_local.h"
#include "../ui/menudef.h"	// For vote options



#define T_FFA	0x01
#define T_1V1	0x02
#define T_SP	0x04
#define T_TDM	0x08
#define T_CTF	0x10
#define T_WLF	0x20
#define T_WSW	0x40
#define T_WCP	0x80
#define T_WCH	0x100


static const char *ACTIVATED = "ACTIVATED";
static const char *DEACTIVATED = "DEACTIVATED";
static const char *ENABLED = "ENABLED";
static const char *DISABLED = "DISABLED";

static const char *gameNames[] = {
	"Single Player",
	"Cooperative",
	"Objective",
	"Stopwatch",
	"Campaign",
	"Last Man Standing",
	"Map Voting"
};


//
// Update info:
//	1. Add line to aVoteInfo w/appropriate info
//	2. Add implementation for attempt and success (see an existing command for an example)
//
typedef struct {
	unsigned int dwGameTypes;
	const char *pszVoteName;
	int (*pVoteCommand)(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd);
	const char *pszVoteMessage;
	const char *pszVoteHelp;
} vote_reference_t;

// VC optimizes for dup strings :)
static const vote_reference_t aVoteInfo[] = {
	{ 0x1ff, "comp",		 G_Comp_v,			"Load Competition Settings", "^7\n  Loads standard competition settings for the current mode" },
	{ 0x1ff, "gametype",	 G_Gametype_v,		"Set Gametype to",	" <value>^7\n  Changes the current gametype" },
	{ 0x1ff, "kick",		 G_Kick_v,			"KICK",				" <player_id>^7\n  Attempts to kick player from server" },
	{ 0x1ff, "mute",		 G_Mute_v,			"MUTE",				" <player_id>^7\n  Removes the chat capabilities of a player" },
	{ 0x1ff, "unmute",		 G_UnMute_v,		"UN-MUTE",			" <player_id>^7\n  Restores the chat capabilities of a player" },
	{ 0x1ff, "map",			 G_Map_v,			"Change map to",	" <mapname>^7\n  Votes for a new map to be loaded" },
	{ 0x1ff, "campaign",	 G_Campaign_v,		"Change campaign to",	" <campaign>^7\n  Votes for a new map to be loaded" },
	{ 0x1ff, "maprestart",	 G_MapRestart_v,	"Map Restart",		"^7\n  Restarts the current map in progress" },
	{ 0x1ff, "matchreset",   G_MatchReset_v,	"Match Reset",		"^7\n  Resets the entire match" },
	{ 0x1ff, "mutespecs",	 G_Mutespecs_v,		"Mute Spectators",	" <0|1>^7\n  Mutes in-game spectator chat" },
	{ 0x1ff, "nextmap",		 G_Nextmap_v,		"Load Next Map",	"^7\n  Loads the next map or campaign in the map queue" },
	{ 0x1ff, "pub",			 G_Pub_v,			"Load Public Settings", "^7\n  Loads standard public settings for the current mode" },
	{ 0x1ff, "referee",		 G_Referee_v,		"Referee",			" <player_id>^7\n  Elects a player to have admin abilities" },
	{ 0x1ff, "shuffleteamsxp", G_ShuffleTeams_v,
		"Shuffle Teams and Restart",	
		" ^7\n  Place players on each team according to ability, "
		"then restart" },
	{ 0x1ff, "startmatch",	 G_StartMatch_v,	"Start Match",		" ^7\n  Sets all players to \"ready\" status to start the match" },
	{ 0x1ff, "swapteams",	 G_SwapTeams_v,		"Swap Teams",		" ^7\n  Switch the players on each team" },
	{ 0x1ff, "friendlyfire", G_FriendlyFire_v,	"Friendly Fire",	" <0|1>^7\n  Toggles ability to hurt teammates" },
	{ 0x1ff, "timelimit",	 G_Timelimit_v,		"Timelimit",		" <value>^7\n  Changes the current timelimit" },
	{ 0x1ff, "unreferee",	 G_Unreferee_v,		"UNReferee",		" <player_id>^7\n  Elects a player to have admin abilities removed" },
	{ 0x1ff, "warmupdamage", G_Warmupfire_v,	"Warmup Damage",	" <0|1|2>^7\n  Specifies if players can inflict damage during warmup" },
	{ 0x1ff, "antilag",		 G_AntiLag_v,		"Anti-Lag",			" <0|1>^7\n  Toggles Anit-Lag on the server" },
	{ 0x1ff, "balancedteams",G_BalancedTeams_v,	"Balanced Teams",	" <0|1>^7\n  Toggles team balance forcing" },
	{ 0x1ff, "surrender",G_Surrender_v, "Surrender",
		"  ^7\n  Ends the match." },
	{ 0x1ff, "restartcampaign", G_RestartCampaign_v, "Restart Campaign",
		" ^7\n  Restarts the current Camapaign" },
	{ 0x1ff, "nextcampaign", G_NextCampaign_v, "Next Campaign",
		" ^7\n  Ends the current campaign and starts the next one." },
	{ 0x1ff, "poll", G_Poll_v, "[poll]",
		" <text>^7\n  Poll majority opinion." },
	{ 0x1ff, "shufflenorestart", G_ShuffleNoRestart_v,
		"Shuffle Teams (No Restart)",
		" ^7\n  Place players on each team according to ability." },
	{ 0x1ff, "putspec", G_PutSpec_v, "PutSpec",
		" <player_id>^7\n  Puts the player in the spectator team" },
	// pheno: cointoss
	{ 0x1ff, "cointoss", G_CoinToss_v, "Coin Toss", " ^7\n  Heads or Tails." },
	{ 0, 0, NULL, 0 }
};


// Checks for valid custom callvote requests from the client.
int G_voteCmdCheck(gentity_t *ent, char *arg, char *arg2, qboolean fRefereeCmd)
{
	unsigned int i, cVoteCommands = sizeof(aVoteInfo)/sizeof(aVoteInfo[0]);

	for(i=0; i<cVoteCommands; i++) {
		if(!Q_stricmp(arg, aVoteInfo[i].pszVoteName)) {
			int hResult = aVoteInfo[i].pVoteCommand(ent, i, arg, arg2, fRefereeCmd);

			if(hResult == G_OK) {
				Com_sprintf(arg, VOTE_MAXSTRING, aVoteInfo[i].pszVoteMessage);
				level.voteInfo.vote_fn = aVoteInfo[i].pVoteCommand;
			} else {
				level.voteInfo.vote_fn = NULL;
			}

			return(hResult);
		}
	}

	return(G_NOTFOUND);
}


// Voting help summary.
void G_voteHelp(gentity_t *ent, qboolean fShowVote)
{
	int i, rows = 0, num_cmds = sizeof(aVoteInfo)/sizeof(aVoteInfo[0]) - 1;	// Remove terminator;
	int vi[100];			// Just make it large static.

	
	if(fShowVote) CP("print \"\nValid ^3callvote^7 commands are:\n^3----------------------------\n\"");

	for(i=0; i<num_cmds; i++) {
		if(aVoteInfo[i].dwGameTypes & (1 << g_gametype.integer)) vi[rows++] = i;
	}

	num_cmds = rows;
	rows = num_cmds / HELP_COLUMNS;

	if(num_cmds % HELP_COLUMNS) rows++;
	if(rows < 0) return;

	for(i=0; i<rows; i++) {
		if(i+rows*3+1 <= num_cmds) {
			G_refPrintf(ent, "^5%-17s%-17s%-17s%-17s", aVoteInfo[vi[i]].pszVoteName,
													   aVoteInfo[vi[i+rows]].pszVoteName,
													   aVoteInfo[vi[i+rows*2]].pszVoteName,
													   aVoteInfo[vi[i+rows*3]].pszVoteName);
		} else if(i+rows*2+1 <= num_cmds) {
			G_refPrintf(ent, "^5%-17s%-17s%-17s", aVoteInfo[vi[i]].pszVoteName,
												  aVoteInfo[vi[i+rows]].pszVoteName,
												  aVoteInfo[vi[i+rows*2]].pszVoteName);
		} else if(i+rows+1 <= num_cmds) {
			G_refPrintf(ent, "^5%-17s%-17s", aVoteInfo[vi[i]].pszVoteName,
											 aVoteInfo[vi[i+rows]].pszVoteName);
		} else {
			G_refPrintf(ent, "^5%-17s", aVoteInfo[vi[i]].pszVoteName);
		}
	}

	if(fShowVote) {
		CP("print \"\nUsage: ^3\\callvote <command> <params>\n^7For current settings/help, use: ^3\\callvote <command> ?\n\n\"");
	}

	return;
}

// Set disabled votes for client UI
void G_voteFlags(void)
{
	int i, flags = 0;
	
	for(i=0; i<numVotesAvailable; i++) {
		if(trap_Cvar_VariableIntegerValue(voteToggles[i].pszCvar) == 0) flags |= voteToggles[i].flag;
	}

	if(flags != voteFlags.integer) trap_Cvar_Set("voteFlags", va("%d", flags));
}

// Prints specific callvote command help description.
qboolean G_voteDescription(gentity_t *ent, qboolean fRefereeCmd, int cmd)
{
	char arg[MAX_TOKEN_CHARS];
	char *ref_cmd = (fRefereeCmd) ? "\\ref" : "\\callvote";

	// yada - give console some help!
	//if(!ent) return(qfalse);

	trap_Argv(2, arg, sizeof(arg));
	if(!Q_stricmp(arg, "?") || trap_Argc() == 2) {
		trap_Argv(1, arg, sizeof(arg));
		G_refPrintf(ent, "\nUsage: ^3%s %s%s\n", ref_cmd, arg, aVoteInfo[cmd].pszVoteHelp);
		return(qtrue);
	}

	return(qfalse);
}


// Localize disable message info.
void G_voteDisableMessage(gentity_t *ent, const char *cmd)
{
	G_refPrintf(ent, "Sorry, [lof]^3%s^7 [lon]voting has been disabled", cmd);
}


// Player ID message stub.
void G_playersMessage(gentity_t *ent)
{
	G_refPrintf(ent, "Use the ^3players^7 command to find a valid player ID.");
}


// Localize current parameter setting.
void G_voteCurrentSetting(gentity_t *ent, const char *cmd, const char *setting)
{
	G_refPrintf(ent, "^2%s^7 is currently ^3%s\n", cmd, setting);
}


// Vote toggling
int G_voteProcessOnOff(gentity_t *ent, char *arg, char *arg2, qboolean fRefereeCmd, int curr_setting, int vote_allow, int vote_type)
{
	if(!vote_allow && ent && !ent->client->sess.referee) {
		G_voteDisableMessage(ent, aVoteInfo[vote_type].pszVoteName);
		G_voteCurrentSetting(ent, aVoteInfo[vote_type].pszVoteName, ((curr_setting) ? ENABLED : DISABLED));
		return(G_INVALID);
	}
	if(G_voteDescription(ent, fRefereeCmd, vote_type)) {
		G_voteCurrentSetting(ent, aVoteInfo[vote_type].pszVoteName, ((curr_setting) ? ENABLED : DISABLED));
		return(G_INVALID);
	}

	if((atoi(arg2) && curr_setting) || (!atoi(arg2) && !curr_setting)) {
		G_refPrintf(ent, "^3%s^5 is already %s!", aVoteInfo[vote_type].pszVoteName, ((curr_setting) ? ENABLED : DISABLED));
		return(G_INVALID);
	}

	Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);
	Com_sprintf(arg2, VOTE_MAXSTRING, "%s", (atoi(arg2)) ? ACTIVATED : DEACTIVATED);

	return(G_OK);
}


//
// Several commands to help with cvar setting
//
void G_voteSetOnOff(const char *desc, const char *cvar)
{
	AP(va("cpm \"^3%s is: ^5%s\n\"", desc, (atoi(level.voteInfo.vote_value)) ? ENABLED : DISABLED));
	//trap_SendConsoleCommand(EXEC_APPEND, va("%s %s\n", cvar, level.voteInfo.vote_value));
	trap_Cvar_Set( cvar, level.voteInfo.vote_value );
}

void G_voteSetValue(const char *desc, const char *cvar)
{
	AP(va("cpm \"^3%s set to: ^5%s\n\"", desc, level.voteInfo.vote_value));
	//trap_SendConsoleCommand(EXEC_APPEND, va("%s %s\n", cvar, level.voteInfo.vote_value));
	trap_Cvar_Set( cvar, level.voteInfo.vote_value );
}

void G_voteSetVoteString(const char *desc)
{
	AP(va("print \"^3%s set to: ^5%s\n\"", desc, level.voteInfo.vote_value));
	trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteInfo.voteString));
}






////////////////////////////////////////////////////////
//
// Actual vote command implementation
//
////////////////////////////////////////////////////////




// *** Load competition settings for current mode ***
int G_Comp_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(trap_Argc() > 2) {
			G_refPrintf(ent, "Usage: ^3%s %s%s\n", ((fRefereeCmd) ? "\\ref" : "\\callvote"), arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
			return(G_INVALID);
		} else if(vote_allow_comp.integer<=0 && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		}

	// Vote action (vote has passed)
	} else {
		// Load in comp settings for current gametype
		//G_configSet(g_gametype.integer, qtrue);
		trap_SendConsoleCommand(EXEC_APPEND, "exec default_comp.cfg\n");
		// CHRUKER: b042 - Was using cp, but subsequent cp calls
		//          quickly removed it, so now its using cpm 
		AP("cpm \"Competition Settings Loaded!\n\"");
	}

	return(G_OK);
}


void G_GametypeList(gentity_t *ent)
{
	int i;

	G_refPrintf(ent, "\nAvailable gametypes:\n--------------------");

	for(i=GT_WOLF; i<GT_MAX_GAME_TYPE; i++) {
		if( i != GT_WOLF_CAMPAIGN ) {
			G_refPrintf(ent, "  %d ^3(%s)", i, gameNames[i]);
		}
	}

	G_refPrintf(ent, "\n");
}

// *** GameType ***
int G_Gametype_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		int i = atoi(arg2);

		if(!vote_allow_gametype.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			G_GametypeList(ent);
			G_voteCurrentSetting(ent, arg, va("%d (%s)", g_gametype.integer, gameNames[g_gametype.integer]));
			return(G_INVALID);
		} else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) {
			G_GametypeList(ent);
			G_voteCurrentSetting(ent, arg, va("%d (%s)", g_gametype.integer, gameNames[g_gametype.integer]));
			return(G_INVALID);
		}

		if(i < GT_WOLF || i >= GT_MAX_GAME_TYPE || i == GT_WOLF_CAMPAIGN) {
			G_refPrintf(ent, "\n^3Invalid gametype: ^7%d", i);
			G_GametypeList(ent);
			return(G_INVALID);
		}

		if(i == g_gametype.integer) {
			G_refPrintf(ent, "\n^3Gametype^5 is already set to %s!", gameNames[i]);
			return(G_INVALID);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);
		Com_sprintf(arg2, VOTE_MAXSTRING, "%s", gameNames[i]);

	// Vote action (vote has passed)
	} else {
		// Set gametype
		G_voteSetValue("Gametype", "g_gametype");
		Svcmd_ResetMatch_f(qtrue, qtrue);
	}

	return(G_OK);
}


// *** Player Kick ***
int G_Kick_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd ) {
	// Vote request (vote is being initiated)
	if( arg ) {
		int pid;

		if( !vote_allow_kick.integer && ent && !ent->client->sess.referee ) {
			G_voteDisableMessage(ent, arg);
			return G_INVALID;
		} else if( G_voteDescription(ent, fRefereeCmd, dwVoteIndex) ) {
			return G_INVALID;
		} else if( ( pid = ClientNumberFromString( ent, arg2 ) ) == -1 ) {
			return G_INVALID;
		}

		if( level.clients[ pid ].sess.referee ) {
			G_refPrintf( ent, "Can't vote to kick referees!" );
			return G_INVALID;
		}
#ifdef GS_SHRUBBOT
		if(G_shrubbot_permission(&g_entities[pid], SBF_IMMUNITY)) {
			G_refPrintf( ent, "Can't vote to kick admins!" );
			return G_INVALID;
		}
#endif
		// pheno: prevent ettv slaves from being callvote kicked 
		if( level.clients[pid].sess.ettv &&
			( g_ettvFlags.integer & ETTV_IMMUNITY ) ) {
			G_refPrintf( ent, "Can't vote to kick ettv slaves!" );
			return G_INVALID;
		}

		if( !fRefereeCmd && ent ) {
			if( level.clients[ pid ].sess.sessionTeam != TEAM_SPECTATOR && level.clients[ pid ].sess.sessionTeam != ent->client->sess.sessionTeam ) {
				G_refPrintf( ent, "Can't vote to kick players on opposing team!" );
				return G_INVALID;
			}
		}

		Com_sprintf( level.voteInfo.vote_value, VOTE_MAXSTRING, "%d", pid );
		Com_sprintf( arg2, VOTE_MAXSTRING, "%s^7", level.clients[pid].pers.netname );

	// Vote action (vote has passed)
	} else {
		// Kick a player
		//trap_SendConsoleCommand( EXEC_APPEND, va( "clientkick %d\n", atoi( level.voteInfo.vote_value ) ) );
		// tjw: clientkick doesn't work in 2.60
		trap_DropClient(atoi(level.voteInfo.vote_value),
			"You have been kicked", 120);

		AP( va( "cp \"%s\n^3has been kicked!\n\"", level.clients[ atoi( level.voteInfo.vote_value ) ].pers.netname ) );
	}

	return G_OK;
}

// *** Player Mute ***
int G_Mute_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// yada - no its handled here now
	//if( fRefereeCmd )
	//	// handled elsewhere
	//	return(G_NOTFOUND);

	// Vote request (vote is being initiated)
	if(arg) {
		int pid;

		if(!vote_allow_muting.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		} else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) return(G_INVALID);
		else if((pid = ClientNumberFromString(ent, arg2)) == -1) return(G_INVALID);

		if(level.clients[pid].sess.referee) {
			G_refPrintf(ent, "Can't vote to mute referees!");
			return(G_INVALID);
		}
#ifdef GS_SHRUBBOT
		if(G_shrubbot_permission(&g_entities[pid], SBF_IMMUNITY)) {
			G_refPrintf( ent, "Can't vote to mute admins!" );
			return G_INVALID;
		}
#endif
		if(level.clients[pid].sess.auto_unmute_time != 0) {
			G_refPrintf(ent, "Player is already muted!");
			return(G_INVALID);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%d", pid);
		Com_sprintf(arg2, VOTE_MAXSTRING, "%s^7", level.clients[pid].pers.netname);

	// Vote action (vote has passed)
	} else {
		int pid = atoi(level.voteInfo.vote_value);

		// Mute a player
		if( level.clients[pid].sess.referee != RL_RCON ) {
			trap_SendServerCommand( pid, va( "cpm \"^3You have been muted\"") );
			level.clients[pid].sess.auto_unmute_time = -1;
			AP(va("cp \"%s\n^3has been muted!\n\"", level.clients[pid].pers.netname));
			ClientUserinfoChanged( pid );
		} else {
			G_Printf( "Cannot mute a referee.\n" );
		}
	}

	return(G_OK);
}

// *** Player Un-Mute ***
int G_UnMute_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// yada - no its handled here now
	//if( fRefereeCmd )
	//	// handled elsewhere
	//	return(G_NOTFOUND);

	// Vote request (vote is being initiated)
	if(arg) {
		int pid;

		if(!vote_allow_muting.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		} else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) return(G_INVALID);
		else if((pid = ClientNumberFromString(ent, arg2)) == -1) return(G_INVALID);
		// CHRUKER: b060 - Referees can always be unmuted
		/*
		if(level.clients[pid].sess.referee) {
			G_refPrintf(ent, "Can't vote to un-mute referees!");
			return(G_INVALID);
		}
		*/

		if(level.clients[pid].sess.auto_unmute_time == 0) {
			G_refPrintf(ent, "Player is not muted!");
			return(G_INVALID);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%d", pid);
		Com_sprintf(arg2, VOTE_MAXSTRING, "%s^7", level.clients[pid].pers.netname);

	// Vote action (vote has passed)
	} else {
		int pid = atoi(level.voteInfo.vote_value);

		// Mute a player
		if( level.clients[pid].sess.referee != RL_RCON ) {
			trap_SendServerCommand( pid, va( "cpm \"^3You have been un-muted\"") );
			level.clients[pid].sess.auto_unmute_time = 0;
			AP(va("cp \"%s\n^3has been un-muted!\n\"", level.clients[pid].pers.netname));
			ClientUserinfoChanged( pid );
		} else {
			G_Printf( "Cannot un-mute a referee.\n" );
		}
	}

	return(G_OK);
}

// *** Player PutSpec ***
int G_PutSpec_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// yada - my ass... this isnt handled elsewhere at all
	//if( fRefereeCmd ){
	//	// handled elsewhere
	//	return(G_NOTFOUND);
	//}

	// Vote request (vote is being initiated)
	if(arg) {
		int pid;

		if(!vote_allow_putspec.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		} else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) return(G_INVALID);
		else if((pid = ClientNumberFromString(ent, arg2)) == -1) return(G_INVALID);

		if(level.clients[pid].sess.referee) {
			G_refPrintf(ent, "Can't vote to PutSpec referees!");
			return(G_INVALID);
		}
#ifdef GS_SHRUBBOT
		if(G_shrubbot_permission(&g_entities[pid], SBF_IMMUNITY)) {
			G_refPrintf( ent, "Can't vote to PutSpec admins!" );
			return G_INVALID;
		}
#endif
		if(level.clients[pid].sess.sessionTeam == TEAM_SPECTATOR ||
			level.clients[pid].sess.sessionTeam != ent->client->sess.sessionTeam) {
			G_refPrintf(ent, "You can only PutSpec players in your own team!");
			return G_INVALID;
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%d", pid);
		Com_sprintf(arg2, VOTE_MAXSTRING, "%s^7", level.clients[pid].pers.netname);

	// Vote action (vote has passed)
	} else {
		int pid = atoi(level.voteInfo.vote_value);

		SetTeam( &g_entities[pid], "s", qtrue, -1, -1, qfalse );
		trap_SendServerCommand( pid, va( "cpm \"^3You have been moved to the Spectators\"") );
		AP(va("cp \"%s ^3has been\nmoved to the Spectators!\n\"", level.clients[pid].pers.netname));
		ClientUserinfoChanged( pid );
		
		if(g_gamestate.integer == GS_WARMUP || g_gamestate.integer == GS_WARMUP_COUNTDOWN) {
			G_readyMatchState();
		}
	}

	return(G_OK);
}

// *** Map - simpleton: we dont verify map is allowed/exists ***
int G_Map_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		char serverinfo[MAX_INFO_STRING];
		trap_GetServerinfo(serverinfo, sizeof(serverinfo));

		if(!vote_allow_map.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			G_voteCurrentSetting(ent, arg, Info_ValueForKey(serverinfo, "mapname"));
			return(G_INVALID);
		} else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) {
			G_voteCurrentSetting(ent, arg, Info_ValueForKey(serverinfo, "mapname"));
			return(G_INVALID);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);

	// Vote action (vote has passed)
	} else {
		int i;
		char s[MAX_STRING_CHARS];

		if(!level.intermissiontime){
			if(g_XPSave.integer &  XPSF_STORE_AT_RESTART){
				for( i = 0; i < level.numConnectedClients; i++ ) {
					G_xpsave_add(&g_entities[level.sortedClients[i]],qfalse);
				}
				G_xpsave_writeconfig();
			}	
		}
		G_reset_disconnects();

		if( g_gametype.integer == GT_WOLF_CAMPAIGN ) {
			trap_Cvar_VariableStringBuffer("nextcampaign", s, sizeof(s));
			trap_SendConsoleCommand(EXEC_APPEND, va("campaign %s%s\n", level.voteInfo.vote_value, ((*s) ? va("; set nextcampaign \"%s\"", s) : "")));
		} else {
			Svcmd_ResetMatch_f(qtrue, qfalse);
			trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
			trap_SendConsoleCommand(EXEC_APPEND, va("map %s%s\n", level.voteInfo.vote_value, ((*s) ? va("; set nextmap \"%s\"", s) : "")));
		}
	}

	return(G_OK);
}

// *** Campaign - simpleton: we dont verify map is allowed/exists ***
int G_Campaign_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		char serverinfo[MAX_INFO_STRING];
		trap_GetServerinfo(serverinfo, sizeof(serverinfo));

		if(!vote_allow_map.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			if( g_gametype.integer == GT_WOLF_CAMPAIGN ) {
				G_voteCurrentSetting(ent, arg, g_campaigns[level.currentCampaign].shortname );
			}
			return(G_INVALID);
		} else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) {
			if( g_gametype.integer == GT_WOLF_CAMPAIGN ) {
				G_voteCurrentSetting(ent, arg, g_campaigns[level.currentCampaign].shortname );
			}
			return(G_INVALID);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);

	// Vote action (vote has passed)
	} else {
		char s[MAX_STRING_CHARS];
		int i;

		if(!level.intermissiontime){
			if(g_XPSave.integer &  XPSF_STORE_AT_RESTART){
				for( i = 0; i < level.numConnectedClients; i++ ) {
					G_xpsave_add(&g_entities[level.sortedClients[i]],qfalse);
				}
				G_xpsave_writeconfig();
			}
		}
		G_reset_disconnects();

		trap_Cvar_VariableStringBuffer("nextcampaign", s, sizeof(s));
		trap_SendConsoleCommand(EXEC_APPEND, va("campaign %s%s\n", level.voteInfo.vote_value, ((*s) ? va("; set nextcampaign \"%s\"", s) : "")));
	}

	return(G_OK);
}

// *** Map Restart ***
int G_MapRestart_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(!vote_allow_maprestart.integer
			&& ent && !ent->client->sess.referee) {

			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		}
		else if(trap_Argc() != 2 &&
			G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) {

			return(G_INVALID);
		}
//		else if(trap_Argc() > 2) {
//			if(!Q_stricmp(arg2, "?")) {
//				G_refPrintf(ent, "Usage: ^3%s %s%s\n", ((fRefereeCmd) ? "\\ref" : "\\callvote"), arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
//				return(G_INVALID);
//			}
//		}

	// Vote action (vote has passed)
	} else {
		// Restart the map back to warmup
		Svcmd_ResetMatch_f(qfalse, qtrue);
		AP("cp \"^1*** Level Restarted! ***\n\"");
	}

	return(G_OK);
}


// *** Match Restart ***
int G_MatchReset_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(!vote_allow_matchreset.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		} else if( trap_Argc() != 2 && G_voteDescription(ent, fRefereeCmd, dwVoteIndex) ) {
			return(G_INVALID);
//		} else if(trap_Argc() > 2) {
//			if(!Q_stricmp(arg2, "?")) {
//				G_refPrintf(ent, "Usage: ^3%s %s%s\n", ((fRefereeCmd) ? "\\ref" : "\\callvote"), arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
//				return(G_INVALID);
//			}
		}

	// Vote action (vote has passed)
	} else {
		// Restart the map back to warmup
		Svcmd_ResetMatch_f(qtrue, qtrue);
		AP("cp \"^1*** Match Reset! ***\n\"");
	}

	return(G_OK);
}


// *** Mute Spectators ***
int G_Mutespecs_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		return(G_voteProcessOnOff(ent, arg, arg2, fRefereeCmd,
									!!(match_mutespecs.integer),
									vote_allow_mutespecs.integer,
									dwVoteIndex));

	// Vote action (vote has passed)
	} else {
		// Mute/unmute spectators
		G_voteSetOnOff("Spectator Muting", "match_mutespecs");
	}

	return(G_OK);
}


// *** Nextmap ***
int G_Nextmap_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	int i;
	// Vote request (vote is being initiated)
	if(arg) {
		if(trap_Argc() > 2) {
			G_refPrintf(ent, "Usage: ^3%s %s%s\n", ((fRefereeCmd) ? "\\ref" : "\\callvote"), arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
			return(G_INVALID);
		} else if(!vote_allow_nextmap.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		} else {
			char s[MAX_STRING_CHARS];
			if( g_gametype.integer == GT_WOLF_CAMPAIGN ) {
				trap_Cvar_VariableStringBuffer( "nextcampaign", s, sizeof(s) );
				if( !*s ) {
					G_refPrintf(ent, "'nextcampaign' is not set." );
					return(G_INVALID);
				}
			} else {
				trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
				if( !*s ) {
					G_refPrintf(ent, "'nextmap' is not set." );
					return(G_INVALID);
				}
			}
		}

	// Vote action (vote has passed)
	} else {

		if(!level.intermissiontime){
			if(g_XPSave.integer &  XPSF_STORE_AT_RESTART){
				for( i = 0; i < level.numConnectedClients; i++ ) {
					G_xpsave_add(&g_entities[level.sortedClients[i]],qfalse);
				}
				G_xpsave_writeconfig();
			}
		}
		G_reset_disconnects();

		if( g_gametype.integer == GT_WOLF_CAMPAIGN ) {
			g_campaignInfo_t *campaign;

			campaign = &g_campaigns[level.currentCampaign];
			if( campaign->current + 1 < campaign->mapCount ) {
				trap_Cvar_Set("g_currentCampaignMap",
					va( "%i", campaign->current + 1));
				trap_SendConsoleCommand(EXEC_APPEND,
					va( "map %s\n",
						campaign->mapnames[campaign->current + 1]));
				AP("cp \"^3*** Loading next map in campaign! ***\n\"");
			}
			else {
				// Load in the nextcampaign
				trap_SendConsoleCommand(EXEC_APPEND, "vstr nextcampaign\n");
				AP("cp \"^3*** Loading nextcampaign! ***\n\"");
			}
		}else if( g_gametype.integer == GT_WOLF_MAPVOTE && 
			g_mapVoteFlags.integer & MAPVOTE_NEXTMAP_VOTEMAP){
			// Dens: don't do this. This is awkward, since it is not done at
			// !nextmap nor nextcampaignvotes. Besides we don't want to store
			// mapstats of an unfinished map or spend resources at generating
			// playerstats
			// LogExit( "Nextmap vote passed" );
			AP("chat \"^3*** Nextmap vote passed! Choose a new map! ***\"");
		}else{
			// Load in the nextmap
			trap_SendConsoleCommand(EXEC_APPEND, "vstr nextmap\n");
			AP("cp \"^3*** Loading nextmap! ***\n\"");
		}
	}

	return(G_OK);
}


// *** Load public settings for current mode ***
int G_Pub_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(trap_Argc() > 2) {
			G_refPrintf(ent, "Usage: ^3%s %s%s\n", ((fRefereeCmd) ? "\\ref" : "\\callvote"), arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
			return(G_INVALID);
		} else if(vote_allow_pub.integer<=0 && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		}

	// Vote action (vote has passed)
	} else {
		// Load in pub settings for current gametype
		//G_configSet(g_gametype.integer, qfalse);
		trap_SendConsoleCommand(EXEC_APPEND, "exec default_pub.cfg\n");
		// CHRUKER: b042 - Was using cp, but subsequent cp calls
		//          quickly removed it, so now its using cpm 
		AP("cpm \"Public Settings Loaded!\n\"");
	}

	return(G_OK);
}


// *** Referee voting ***
int G_Referee_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		int pid;

		if(!vote_allow_referee.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		}

		if(ent && !ent->client->sess.referee && level.numPlayingClients < 3) { // yada - ent == NULL for console...
			G_refPrintf(ent, "Sorry, not enough clients in the game to vote for a referee");
			return(G_INVALID);
		}

		// yada - ent == NULL for console...
		if(	(!ent || ent->client->sess.referee) && trap_Argc() == 2) {
			G_playersMessage(ent);
			return(G_INVALID);
		} else if(ent && trap_Argc() == 2) pid = ent - g_entities; // yada - ent == NULL for console...
		else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) return(G_INVALID);
		else if((pid = ClientNumberFromString(ent, arg2)) == -1) return(G_INVALID);

		if(level.clients[pid].sess.referee) {
			G_refPrintf(ent, "[lof]%s [lon]is already a referee!", level.clients[pid].pers.netname);
			return(-1);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%d", pid);
		Com_sprintf(arg2, VOTE_MAXSTRING, "%s^7", level.clients[pid].pers.netname);

	// Vote action (vote has passed)
	} else {
		// Voting in a new referee
		gclient_t *cl = &level.clients[atoi(level.voteInfo.vote_value)];

		if(cl->pers.connected == CON_DISCONNECTED) {
			AP("print \"Player left before becoming referee\n\"");
		} else {
			cl->sess.referee = RL_REFEREE;	// FIXME: Differentiate voted refs from passworded refs
			cl->sess.spec_invite = TEAM_AXIS | TEAM_ALLIES;
			AP(va("cp \"%s^7 is now a referee\n\"", cl->pers.netname));
			ClientUserinfoChanged( atoi(level.voteInfo.vote_value) );
		}
	}
	return(G_OK);
}


// *** Shuffle teams
int G_ShuffleTeams_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(trap_Argc() > 2) {
			// CHRUKER: b047 - Removed unneeded linebreak
			G_refPrintf(ent, "Usage: ^3%s %s%s\n", ((fRefereeCmd) ? "\\ref" : "\\callvote"), arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
			return(G_INVALID);
		} else if(!vote_allow_shuffleteamsxp.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		}

	// Vote action (vote has passed)
	} else {
		// Swap the teams!
		Svcmd_ShuffleTeams_f();
	}

	return(G_OK);
}

// *** Shuffle teams
int G_ShuffleNoRestart_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(trap_Argc() > 2) {
			// CHRUKER: b047 - Removed unneeded linebreak
			G_refPrintf(ent, "Usage: ^3%s %s%s\n", ((fRefereeCmd) ? "\\ref" : "\\callvote"), arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
			return(G_INVALID);
		} else if(!vote_allow_shufflenorestart.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		}

	// Vote action (vote has passed)
	} else {
		G_shuffleTeams();
	}

	return(G_OK);
}


// *** Start Match ***
int G_StartMatch_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(trap_Argc() > 2) {
			if(!Q_stricmp(arg2, "?")) {
				G_refPrintf(ent, "Usage: ^3%s %s%s\n", ((fRefereeCmd) ? "\\ref" : "\\callvote"), arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
				return(G_INVALID);
			}
		}

		if(g_gamestate.integer == GS_PLAYING || g_gamestate.integer == GS_INTERMISSION) {
			G_refPrintf(ent, "^3Match is already in progress!");
			return(G_INVALID);
		}

		if(g_gamestate.integer == GS_WARMUP_COUNTDOWN) {
			G_refPrintf(ent, "^3Countdown already started!");
			return(G_INVALID);
		}

		if(level.numPlayingClients < match_minplayers.integer) {
			G_refPrintf(ent, "^3Not enough players to start match!");
			return(G_INVALID);
		}

	// Vote action (vote has passed)
	} else {
		// Set everyone to "ready" status
		G_refAllReady_cmd(NULL);
	}

	return(G_OK);
}


// *** Swap teams
int G_SwapTeams_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(trap_Argc() > 2) {
			G_refPrintf(ent, "Usage: ^3%s %s%s\n", ((fRefereeCmd) ? "\\ref" : "\\callvote"), arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
			return(G_INVALID);
		} else if(!vote_allow_swapteams.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		}

	// Vote action (vote has passed)
	} else {
		// Swap the teams!
		Svcmd_SwapTeams_f();
	}

	return(G_OK);
}


// *** Team Damage ***
int G_FriendlyFire_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		return(G_voteProcessOnOff(ent, arg, arg2, fRefereeCmd,
									!!(g_friendlyFire.integer),
									vote_allow_friendlyfire.integer,
									dwVoteIndex));

	// Vote action (vote has passed)
	} else {
		// Team damage (friendlyFire)
		G_voteSetOnOff("Friendly Fire", "g_friendlyFire");
	}

	return(G_OK);
}

// Anti-Lag
int G_AntiLag_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd )
{
	// Vote request (vote is being initiated)
	if(arg) {
		return(G_voteProcessOnOff(ent, arg, arg2, fRefereeCmd,
									!!(g_antilag.integer),
									vote_allow_antilag.integer,
									dwVoteIndex));

	// Vote action (vote has passed)
	} else {
		// Anti-Lag (g_antilag)
		G_voteSetOnOff("Anti-Lag", "g_antilag");
	}

	return(G_OK);
}

// Balanced Teams
int G_BalancedTeams_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd )
{
	// Vote request (vote is being initiated)
	if(arg) {
		return(G_voteProcessOnOff(ent, arg, arg2, fRefereeCmd,
									!!(g_balancedteams.integer),
									vote_allow_balancedteams.integer,
									dwVoteIndex));
	// Vote action (vote has passed)
	} else {
		// Balanced Teams (g_balancedteams)
		G_voteSetOnOff("Balanced Teams", "g_balancedteams");
		trap_Cvar_Set( "g_teamForceBalance", level.voteInfo.vote_value );
		trap_Cvar_Set( "g_lms_teamForceBalance", level.voteInfo.vote_value );
	}

	return(G_OK);
}

int G_Surrender_v( gentity_t *ent, unsigned int dwVoteIndex,
		char *arg, char *arg2, qboolean fRefereeCmd )
{
	team_t team;
	
	// Vote request (vote is being initiated)
	if(arg) {
		if(g_gamestate.integer != GS_PLAYING) {
			return G_INVALID;
		}
		if(!vote_allow_surrender.integer)
			return G_INVALID;
			
		// yada - noone ever seemes to have thought of refs calling this
		if(	!ent||
				ent->client->sess.sessionTeam==TEAM_SPECTATOR
		){
			if(trap_Argc()==2){
				G_refPrintf(ent,"Usage: \\%s surrender <team>",fRefereeCmd?"ref":"callvote");
				return G_INVALID;
			}
			team=TeamFromString(arg2);
			if(	team!=TEAM_AXIS&&
					team!=TEAM_ALLIES
			){
				G_refPrintf(ent,"Invalid team specified.");
				return G_INVALID;
			}
			level.voteInfo.voteTeam=team;
		}else{
			team=ent->client->sess.sessionTeam;
		}
		
		Q_strncpyz(arg2,
			(team == TEAM_AXIS) ?
				"[AXIS]" : "[ALLIES]",
			VOTE_MAXSTRING);
	}
	// Vote action (vote has passed)
	else if(g_gamestate.integer == GS_PLAYING){
		char cs[MAX_STRING_CHARS];

		trap_GetConfigstring(CS_MULTI_MAPWINNER, cs, sizeof(cs));
		Info_SetValueForKey(cs, "winner",
			(level.voteInfo.voteTeam == TEAM_AXIS) ? "1" : "0");
		trap_SetConfigstring(CS_MULTI_MAPWINNER, cs);
		LogExit(va("%s Surrender\n",
			(level.voteInfo.voteTeam == TEAM_AXIS) ?
			"Axis" : "Allies"));
		AP(va("chat \"%s have surrendered!\" -1",
			(level.voteInfo.voteTeam == TEAM_AXIS) ?
			"^1AXIS^7" : "^4ALLIES^7"));
	}
	return(G_OK);
}

int G_NextCampaign_v( gentity_t *ent, unsigned int dwVoteIndex,
		char *arg, char *arg2, qboolean fRefereeCmd )
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(!vote_allow_nextcampaign.integer)
			return G_INVALID;
		if(g_gametype.integer != GT_WOLF_CAMPAIGN)
			return G_INVALID;
	}
	// Vote action (vote has passed)
	else {
		char s[MAX_STRING_CHARS];
		int i;

		if(!level.intermissiontime){
			if(g_XPSave.integer &  XPSF_STORE_AT_RESTART){
				for( i = 0; i < level.numConnectedClients; i++ ) {
					G_xpsave_add(&g_entities[level.sortedClients[i]],qfalse);
				}
				G_xpsave_writeconfig();
			}
		}
		G_reset_disconnects();
		
		trap_Cvar_VariableStringBuffer("nextcampaign", s, sizeof(s));
		if(*s)
			trap_SendConsoleCommand(EXEC_APPEND, "vstr nextcampaign\n");
	}
	return(G_OK);
}

int G_RestartCampaign_v( gentity_t *ent, unsigned int dwVoteIndex,
		char *arg, char *arg2, qboolean fRefereeCmd )
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(!vote_allow_restartcampaign.integer)
			return G_INVALID;
		if(g_gametype.integer != GT_WOLF_CAMPAIGN)
			return G_INVALID;
	}
	// Vote action (vote has passed)
	else {
		char s[MAX_STRING_CHARS];
		int i;

		if(!level.intermissiontime){
			if(g_XPSave.integer &  XPSF_STORE_AT_RESTART){
				for( i = 0; i < level.numConnectedClients; i++ ) {
					G_xpsave_add(&g_entities[level.sortedClients[i]],qfalse);
				}
				G_xpsave_writeconfig();
			}
		}
		G_reset_disconnects();

		trap_Cvar_VariableStringBuffer("nextcampaign", s, sizeof(s));
		trap_SendConsoleCommand(EXEC_APPEND,
			va("campaign %s%s\n",
			g_campaigns[level.currentCampaign].shortname,
			((*s) ? va("; set nextcampaign \"%s\"", s) : "")));
	}
	return(G_OK);
}

int G_Poll_v( gentity_t *ent, unsigned int dwVoteIndex,
		char *arg, char *arg2, qboolean fRefereeCmd )
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(!vote_allow_poll.integer)
			return G_INVALID;
		Com_sprintf(arg2,
			VOTE_MAXSTRING, "%s", ConcatArgs(2));
	}
	return(G_OK);
}


// *** Timelimit ***
int G_Timelimit_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		if(!vote_allow_timelimit.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			G_voteCurrentSetting(ent, arg, g_timelimit.string);
			return(G_INVALID);
		} else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) {
			G_voteCurrentSetting(ent, arg, g_timelimit.string);
			return(G_INVALID);
		} else if(atoi(arg2) < 0) {
			G_refPrintf(ent, "Sorry, can't specify a timelimit < 0!");
			return(G_INVALID);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);

	// Vote action (vote has passed)
	} else {
		// Timelimit change
		G_voteSetVoteString("Timelimit");
	}

	return(G_OK);
}

// pheno: cointoss
int G_CoinToss_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd )
{
	// Vote request (vote is being initiated)
	if( arg ) {
		if( !vote_allow_cointoss.integer &&
			ent &&
			!ent->client->sess.referee ) {
			G_voteDisableMessage( ent, arg );
			return ( G_INVALID );
		}
	// Vote action (vote has passed)
	} else {
		char *side = rand() % 2 ? "HEADS" : "TAILS";

		AP( va( "cp \"Coin toss comes up^3 %s^7!\"", side ) );
		AP( va( "cpm \"Coin toss comes up^3 %s^7!\"", side ) );
	}

	return ( G_OK );
}

char *warmupType[] = { "None", "Enemies Only", "Everyone" };

void G_WarmupDamageTypeList(gentity_t *ent)
{
	int i;

	G_refPrintf(ent, "\nAvailable Warmup Damage types:\n------------------------------");
	for(i=0; i<(sizeof(warmupType) / sizeof(char *)); i++) G_refPrintf(ent, "  %d ^3(%s)", i, warmupType[i]);
	G_refPrintf(ent, "\n");
}

// *** Warmup Weapon Fire ***
int G_Warmupfire_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		int i = atoi(arg2), val = (match_warmupDamage.integer < 0) ? 0 :
									(match_warmupDamage.integer > 2) ? 2 :
																	   match_warmupDamage.integer;

		if(!vote_allow_warmupdamage.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			G_WarmupDamageTypeList(ent);
			G_voteCurrentSetting(ent, arg, va("%d (%s)", val, warmupType[val]));
			return(G_INVALID);
		} else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) {
			G_WarmupDamageTypeList(ent);
			G_voteCurrentSetting(ent, arg, va("%d (%s)", val, warmupType[val]));
			return(G_INVALID);
		}

		if(i < 0 || i > 2) {
			G_refPrintf(ent, "\n^3Invalid Warmup Damage type: ^7%d", i);
			G_WarmupDamageTypeList(ent);
			return(G_INVALID);
		}

		if(i == val) {
			G_refPrintf(ent, "\n^3Warmup Damage^5 is already set to %s!", warmupType[i]);
			return(G_INVALID);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);
		Com_sprintf(arg2, VOTE_MAXSTRING, "%s", warmupType[i]);


	// Vote action (vote has passed)
	} else {
		// Warmup damage setting
		// CHRUKER: b048 - Was using print, but this really should be
		//          displayed as a popup message.
		AP(va("cpm \"^3Warmup Damage set to: ^5%s\n\"", warmupType[atoi(level.voteInfo.vote_value)]));
		trap_SendConsoleCommand(EXEC_APPEND, va("match_warmupDamage %s\n", level.voteInfo.vote_value));
	}

	return(G_OK);
}


// *** Un-Referee voting ***
int G_Unreferee_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd)
{
	// Vote request (vote is being initiated)
	if(arg) {
		int pid;

		if(!vote_allow_referee.integer && ent && !ent->client->sess.referee) {
			G_voteDisableMessage(ent, arg);
			return(G_INVALID);
		}

		// yada - ent==NULL for console... 
		if( (!ent || ent->client->sess.referee) && trap_Argc() == 2) {
			G_playersMessage(ent);
			return(G_INVALID);
		} else if(ent && trap_Argc() == 2) pid = ent - g_entities; // yada - ent still NULL for console... 
		else if(G_voteDescription(ent, fRefereeCmd, dwVoteIndex)) return(G_INVALID);
		else if((pid = ClientNumberFromString(ent, arg2)) == -1) return(G_INVALID);

		if(level.clients[pid].sess.referee == RL_NONE) {
			G_refPrintf(ent, "[lof]%s [lon]isn't a referee!", level.clients[pid].pers.netname);
			return(G_INVALID);
		}

		if(level.clients[pid].sess.referee == RL_RCON) {
			G_refPrintf(ent, "[lof]%s's [lon]status cannot be removed", level.clients[pid].pers.netname);
			return(G_INVALID);
		}

		if( level.clients[pid].pers.localClient ) {
			G_refPrintf(ent, "[lof]%s [lon]^7is the Server Host", level.clients[pid].pers.netname);
			return(G_INVALID);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%d", pid);
		Com_sprintf(arg2, VOTE_MAXSTRING, "%s^7", level.clients[pid].pers.netname);

	// Vote action (vote has passed)
	} else {
		// Stripping of referee status
		gclient_t *cl = &level.clients[atoi(level.voteInfo.vote_value)];

		cl->sess.referee = RL_NONE;
		
		if( !cl->sess.shoutcaster ) { // don't remove shoutcaster's invitation
			cl->sess.spec_invite = 0;
		}
		
		AP(va("cp \"%s^7\nis no longer a referee\n\"", cl->pers.netname));
		ClientUserinfoChanged( atoi(level.voteInfo.vote_value) );
	}

	return(G_OK);
}

void G_IntermissionMapVote( gentity_t *ent )
{
	char arg[MAX_TOKEN_CHARS];
	char arg2[MAX_TOKEN_CHARS];
	int voteRank = 0, i;

	if ( g_gametype.integer != GT_WOLF_MAPVOTE ) {
		CP(va("print \"^3Map voting not enabled!\n\""));
		return;
	}
	
	if( g_gamestate.integer != GS_INTERMISSION ) {
		CP(va("print \"^3Can't vote until intermission\n\""));
		return;
	}

	if ( !level.intermissiontime ) {
		CP(va("print \"^3You can only vote during intermission\n\""));
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	// normal one-map vote
	if ( trap_Argc() == 2 ) {
		if( ent->client->ps.eFlags & EF_VOTED ) {
			level.mapvoteinfo[ent->client->sess.mapVotedFor[0]].numVotes--;
			level.mapvoteinfo[ent->client->sess.mapVotedFor[0]].totalVotes--;
		}
		ent->client->ps.eFlags |= EF_VOTED;
		level.mapvoteinfo[atoi(arg)].numVotes++;
		level.mapvoteinfo[atoi(arg)].totalVotes++;
		ent->client->sess.mapVotedFor[0] = atoi(arg);
	} else if ( trap_Argc() == 3 ) {
		trap_Argv(2, arg2, sizeof(arg2));
		voteRank = atoi(arg2);
		if ( voteRank < 1 || voteRank > 3 )
			return;
		for ( i = 0; i < 3; i++ ) {
			if ( voteRank - 1 == i )
				continue;
			if ( ent->client->sess.mapVotedFor[i] == atoi(arg) ) {
				CP(va("print \"^3Can't vote for the same map twice\n\""));
				return;
			}
		}
		if ( ent->client->sess.mapVotedFor[voteRank-1] != -1 ) {
			level.mapvoteinfo[ent->client->sess.mapVotedFor[voteRank-1]].numVotes -= voteRank;
			level.mapvoteinfo[ent->client->sess.mapVotedFor[voteRank-1]].totalVotes -= voteRank;		
		}
		level.mapvoteinfo[atoi(arg)].numVotes += voteRank;
		level.mapvoteinfo[atoi(arg)].totalVotes += voteRank;		
		ent->client->sess.mapVotedFor[voteRank-1] = atoi(arg);
		ent->client->ps.eFlags |= EF_VOTED;
	} else {
		return;
	}
}

void G_IntermissionMapList( gentity_t *ent )
{
	int i;
	char mapList[MAX_STRING_CHARS];
	int maxMaps;

	if ( g_gametype.integer != GT_WOLF_MAPVOTE ||
			!level.intermissiontime )
		return;

	maxMaps = g_maxMapsVotedFor.integer;
	if ( maxMaps > level.mapVoteNumMaps ) 
		maxMaps = level.mapVoteNumMaps;

	Q_strncpyz(mapList, 
			va("immaplist %d ", (g_mapVoteFlags.integer & MAPVOTE_MULTI_VOTE)),
			MAX_STRING_CHARS);

	for ( i = 0; i < maxMaps; i++ ) {
		Q_strcat(mapList, MAX_STRING_CHARS, 
				va("%s %d %d %d ", 
					level.mapvoteinfo[level.sortedMaps[i]].bspName,
					level.sortedMaps[i],
					level.mapvoteinfo[level.sortedMaps[i]].lastPlayed,
					level.mapvoteinfo[level.sortedMaps[i]].totalVotes ) );
	}

	trap_SendServerCommand(ent-g_entities, mapList);
	return;
}

void G_IntermissionVoteTally( gentity_t *ent )
{
	int i;
	char voteTally[MAX_STRING_CHARS];
	int maxMaps;

	if ( g_gametype.integer != GT_WOLF_MAPVOTE ||
			!level.intermissiontime )
		return;

	maxMaps = g_maxMapsVotedFor.integer;
	if ( maxMaps > level.mapVoteNumMaps ) 
		maxMaps = level.mapVoteNumMaps;

	Q_strncpyz(voteTally, "imvotetally ", MAX_STRING_CHARS);
	for ( i = 0; i < maxMaps; i++ ) {
		Q_strcat(voteTally, MAX_STRING_CHARS, 
				va("%d ", level.mapvoteinfo[level.sortedMaps[i]].numVotes ) );
	}
	trap_SendServerCommand(ent-g_entities, voteTally);
	return;
}

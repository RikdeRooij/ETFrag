//
// g_adminsys.c
// admin system
// many things are from shrubbot
//
// -------------------------------

#include "g_local.h"
#include "../ui/menudef.h"
#include "g_adminsys.h"

#ifdef GS_ADMINSYS

#define ANNOUNCE_SHIT // print stuff like loading, saving etc to console
#ifdef GS_DEBUG
//#define ANNOUNCE_DEBUG
void _as_debugprint( char *msg ) {

		AP( va( "print \"^5%s\"", msg ) );
}
void _as_debugprint2( char *msg ) {

		AP( va( "print \"^2%s\"", msg ) );
}
#define DP(x) _as_debugprint(x)
#define DP2(x) _as_debugprint2(x)
#endif


#define SBF_CLANCHAT '+'

// cmds
static const struct g_adminsys_cmd g_adminsys_cmds[] = {
	{"readconfig", 	G_adminsystem_Init, 		'#',		"read all data saved in g_adminsysDir",	"none"},
	{"writeconfig", G_adminsystem_writeadmins,	'$',		"write admins to g_adminsysDir",		"none"},
	// ---
	{"test", 		G_adminsys_cmd_test, 		'z',		"testing cmd",							"none"},

	{"pinfo", 		G_adminsys_cmd_pinfo, 		'y',		"display information about a player",	"(^3name|slot#^7)"	},
	// -------------------------------------------------------------------------------------------------------------------
	{"admintest", 	G_adminsys_cmd_admintest, 	'a',		"display admin level of a player",		"(^3name|slot#^7)"	},
	{"help", 		G_adminsys_cmd_help, 		'b',		"show list of avaible cmds",			"(^3command^7)"		},

	{"setlevel", 	G_adminsys_cmd_setlevel, 	'c',		"sets the admin level of a player",		"[^3name|slot#^7] [^3level^7]"},
	{"levlist", 	G_adminsys_cmd_levlist, 	'd',		"list of loaded admin levels",			"none"},

	{"mute", 		G_adminsys_cmd_mute,		'e',		"removes the chat capabilities of a player",		"[^3name|slot#^7] (^3time^7) (^3reason^7)"},
	{"unmute", 		G_adminsys_cmd_mute,		'f',		"restores the chat capabilities of a player",		"[^3name|slot#^7]"},
	{"kick", 		G_adminsys_cmd_kick,		'g',		"kick player from the server"						"(^3name|slot#^7) (^3reason^7)"},
	{"ban", 		G_adminsys_cmd_ban,			'h',		"ban player from the server"						"(^3name|slot#^7) (^3time^7) (^3reason^7)"},

	//{"ma",			G_adminsys_cmd_adminchat,	'~',		"write the adminchat",				"/ma <message>"},
	{"mc",			G_adminsys_cmd_clanchat,	SBF_CLANCHAT, "the clanchat",						"/mc <message>"},

	//{"ban", 		G_adminsys_ban,			"ban a player by IP and GUID",		"[^3name|slot#^7] (^htime^7) (^hreason^7)"},
	{"", NULL, '\0', "", ""}
};

int	g_adminsys_cmds_size = sizeof( g_adminsys_cmds ) / sizeof( g_adminsys_cmds[0] );

g_adminsys_level_t *g_adminsys_levels[MAX_ADMINSYS_LEVELS];
g_adminsys_admin_t *g_adminsys_admins[MAX_ADMINSYS_ADMINS];


void _as_print( gentity_t *ent, char *msg ) {
	char tempbuff[1024];
	char buffer[1024];
	char outmsg[1024] = { "" };
	int bufflen;
	int i;
	int j = 0;
	int k = 0;
	//int count = 0;

	if(!ent)
		return;

	// this can be done simpler... but this works fine
	
	Q_strncpyz( tempbuff, msg, sizeof(tempbuff) );
	Q_strncpyz( buffer, msg, sizeof(buffer) );
	bufflen = strlen( buffer );

	for( i=0; i<bufflen; i++ ) {
		k++;

		if( tempbuff[i] == '\n' ) { // newline
			buffer[k] = 0;
			Q_strncpyz( outmsg, buffer, k );

			CP( va( "adminsysCPM \"^f%s\"", outmsg ) );

			//count++;
			for( j=0; j<(bufflen-i); j++ ) {
				buffer[j] = tempbuff[j+i+1]; // and skip newline
				k = 0;
			}
			buffer[j] = '\0';
		}
		// end aint a newline
		else if( tempbuff[i+1] == '\0' ) { // && tempbuff[i] != '\n'
			i++;
			Q_strncpyz( outmsg, buffer, i+1 );

			CP( va( "adminsysCPM \"^f%s\"", outmsg ) );
			break;
		}
	}

	//if( count == 0 ) {
	//	CP( va( "adminsysCPM \"^f%s\"", msg ) );
	//}
}

void _as_cpmall( char *msg ) {

	AP( va( "cpm \"^1> ^f%s\"", msg ) );
}

#define CPP(x) _as_print(ent, x)
#define APM(x) _as_cpmall(x)

/*
void _as_print( gentity_t *ent, char *msg ) {

	if(ent)
		CP( va( "print \"^f%s\n\"", msg ) );
}

void _as_cpm( gentity_t *ent, char *msg ) {

	if(ent)
		CP( va( "cpm \"^f%s\n\"", msg ) );
}
void _as_chat( gentity_t *ent, char *msg ) {

	if(ent)
		CP( va( "chat \"^f%s\"", msg ) );
}
void _as_printall( char *msg ) {

		AP( va( "print \"^f%s\n\"", msg ) );
}
void _as_cpmall( char *msg ) {

		AP( va( "cpm \"^1> ^f%s\n\"", msg ) );
}

#define CPP(x) _as_print(ent, x)
#define CPM(x) _as_cpm(ent, x)
#define CPC(x) _as_chat(ent, x)

#define APM(x) _as_cpmall(x)
#define APP(x) _as_printall(x)
*/


void G_adminsys_cleanup_levels( void )
{
	int i = 0;

	for(i=0; g_adminsys_levels[i]; i++) {
		free(g_adminsys_levels[i]);
		g_adminsys_levels[i] = NULL;
	}
}
void G_adminsys_cleanup_admins( void )
{
	int i = 0;

	for(i=0; g_adminsys_admins[i]; i++) {
		free(g_adminsys_admins[i]);
		g_adminsys_admins[i] = NULL;
	}
}
void G_adminsys_cleanup_all( void )
{
	int i = 0;

	for(i=0; g_adminsys_levels[i]; i++) {
		free(g_adminsys_levels[i]);
		g_adminsys_levels[i] = NULL;
	}
	for(i=0; g_adminsys_admins[i]; i++) {
		free(g_adminsys_admins[i]);
		g_adminsys_admins[i] = NULL;
	}
}

// ------------------------


int G_adminsys_getlevel(gentity_t *ent)
{
	int i;
	//qboolean found = qfalse;

	// console
	if(!ent) {
		return MAX_ADMINSYS_ADMINS; // something high
	}
	//if(ent->client->sess.referee == RL_REFEREE ||
	//	ent->client->sess.referee == RL_RCON){
	//	return MAX_ADMINSYS_ADMINS-1;
	//}

	for(i=0; g_adminsys_admins[i]; i++) {
		if(!Q_stricmp(ent->client->sess.guid,
			g_adminsys_admins[i]->guid)) {

			return g_adminsys_admins[i]->level;
		}
	}

	//if(found) {
	//	return g_adminsys_admins[i]->level;
	//}

	return 0;
}


qboolean G_adminsys_permission(gentity_t *ent, char flag)
{
	int i;
	char *flags;
	int alevel = 0;

	// console always wins
	if(!ent)
		return qtrue;
	if(ent->client->sess.referee == RL_REFEREE ||
		ent->client->sess.referee == RL_RCON){
		return qtrue;
	}

	alevel = G_adminsys_getlevel(ent);

	//APP( va("LEVEL: %d", alevel) );

	for(i=0; g_adminsys_levels[i]; i++) {
		if(g_adminsys_levels[i]->level == alevel) {
			flags = g_adminsys_levels[i]->flags;

			while(*flags) {
				if(*flags == flag) {
					return qtrue;

				}
				if(*flags == '*') {
					while(*flags++) {
						if(*flags == flag)
							return qfalse;
					}
					// tjw: flags for individual admins
					//switch(flag) {
					//case SBF_IMMUTABLE:
					//case SBF_INCOGNITO:
					//	return qfalse;
					//default:
					//	return qtrue;
					//}
					return qtrue;
				}
				flags++;
			}
		}
	}
#ifndef GS_DEBUG
	return qfalse;
#else
	CPP( "DEBUG: ^7permission granted!" );
	return qtrue;
#endif
}

qboolean G_adminsys_admin_higher(gentity_t *ent, gentity_t *victim)
{
	int selflevel = 0;
	int otherlevel = 0;
	//int i;

	// console always wins
	if(!ent) return qtrue;
	if(ent->client->sess.referee == RL_REFEREE ||
		ent->client->sess.referee == RL_RCON){
		return qtrue;
	}

	// just in case
	if(!victim) return qtrue;

	selflevel = G_adminsys_getlevel(ent);
	otherlevel = G_adminsys_getlevel(victim);
	if( selflevel >= otherlevel ) {
		return qtrue;
	}

	return qfalse;
}


// ###############################################################################################

qboolean G_adminsystem_cmd_check(gentity_t *ent)
{
	int i;
	char command[MAX_ADMINSYS_CMD_LEN];
	char *cmd;
//	char *string;
	int skip = 0;
	char prefix[6];
	int prefixlen;

	//Q_strncpyz( prefix, "!", sizeof(prefix) );
	Q_strncpyz( prefix, g_adminsysPrefix.string, sizeof(prefix) );
	prefixlen = strlen(prefix);

	command[0] = '\0';
	Q_SayArgv(0, command, sizeof(command));

	if(!command[0])
		return qfalse;

// --
	if(!Q_stricmp(command, "say") ) {
		Q_SayArgv(1, command, sizeof(command));
		skip = 1;
	}
// --


	if( !Q_stricmpn( command, prefix, prefixlen ) ) {
		cmd = &command[prefixlen];
	}
	else if(ent == NULL) {
		cmd = &command[0];
	}
	else {
		return qfalse;
	}

	for (i=0; g_adminsys_cmds[i].keyword[0]; i++) {
		if(Q_stricmp(cmd, g_adminsys_cmds[i].keyword))
			continue;

		if(G_adminsys_permission(ent, g_adminsys_cmds[i].flag)) {

			g_adminsys_cmds[i].handler(ent, skip);

			return qtrue;
		}
		else {
			CPP(va("%s: ^7permission denied",
				g_adminsys_cmds[i].keyword));
			return qtrue;
		}
	}
	return qfalse;
}

void G_adminsys_TellClientsUI(gentity_t *ent)
{
	int i, j;
	int alevel = 0;
	char flags[64];

	//if( ent == NULL ) {
	//	return;
	//}


	*flags = '\0';

	if( ent ) {
		for(i=0;g_adminsys_admins[i];i++) {
			if(!Q_stricmp(g_adminsys_admins[i]->guid,
				ent->client->sess.guid)) {

				alevel = g_adminsys_admins[i]->level;

				trap_SendServerCommand(ent-g_entities, va("adminsystem_setlevel_client %i", alevel) );

				*flags = '\0';
				for (j=0; g_adminsys_cmds[j].keyword[0]; j++) {
					if(G_adminsys_permission(ent, g_adminsys_cmds[j].flag)) {
							Q_strcat( flags, sizeof(flags), va("%c",g_adminsys_cmds[j].flag) );
					}
				}
				if( flags[0] ) {
						trap_SendServerCommand(ent-g_entities, 
							va("adminsystem_setflags_client \"%s\"", flags) );
				}
				break;
			}
		}
// all clients connected
	} else {
		int k;
		//gentity_t *target;

		//G_Printf( "^3*** ^6G_adminsys_TellClientsUI\n" );


		//for( k = 0; k < MAX_CLIENTS; k++) {
		//	target = &g_entities[MAX_CLIENTS+k];
		for ( k=0; k<level.maxclients; k++ ) {
			gclient_t *target = &level.clients[k];
			int clientNum = k;

			if(target->pers.connected != CON_CONNECTED &&
				target->pers.connected != CON_CONNECTING) {
				continue;
			}

// --------------------------------
			for(i=0;g_adminsys_admins[i];i++) {
				if(!Q_stricmp(g_adminsys_admins[i]->guid,
					target->sess.guid)) {

					alevel = g_adminsys_admins[i]->level;

					trap_SendServerCommand(clientNum, 
						va("adminsystem_setlevel_client %d", alevel) );

					//G_Printf( va("^1*** ^7%s: %d\n", target->pers.netname, alevel ) );

					for(j=0; g_adminsys_levels[j]; j++) {
						if(g_adminsys_levels[j]->level == alevel) {
							trap_SendServerCommand(clientNum, 
								va("adminsystem_setflags_client %s", g_adminsys_levels[j]->flags) );

							//G_Printf( va("^2*** ^7flags: %s\n", g_adminsys_levels[j]->flags ) );
							break;
						}
					}
					

					//break;
				}
			}
// ------------------------------------
		}
	}
}


// ------------------------------------------------------------------------
// write


void _FS_Write_S( char *string, fileHandle_t handle )
{
	char buffer[1024];

	//Com_sprintf(buffer, sizeof(buffer), string);
	Q_strncpyz(buffer, string, sizeof(buffer));
	trap_FS_Write(buffer, strlen(buffer), handle);
}


qboolean G_adminsystem_write_admins(void)
{
	fileHandle_t handle;
	char *buffer;
	char *desFilename;
	char *dir;
	int i;

	dir = "";
	dir = g_adminsysDir.string;

	if( dir[0] ) {
		desFilename = va("%s/admins.cfg", dir);
	} else {
		//desFilename = "admins.cfg";
		return qfalse;
	}

#ifdef ANNOUNCE_SHIT
	G_Printf( "^fSaving admins to ^7'%s'\n", desFilename );
#endif

	if(trap_FS_FOpenFile(desFilename, &handle, FS_WRITE) < 0) {
		//G_Printf("Unable to open: %s for writing\n", desFilename);
		G_Printf("Unable to open admins file for writing\n");
		return qfalse;
	}

	for(i=0; g_adminsys_admins[i]; i++) {
		// don't write level 0 users
		if(g_adminsys_admins[i]->level == 0) continue;

		_FS_Write_S("[admin]\n", handle);
		_FS_Write_S("name   = ", handle);
		buffer = va("\"%s\"\n", g_adminsys_admins[i]->name);
		_FS_Write_S(buffer, handle);

		_FS_Write_S("guid   = ", handle);
		buffer = va("\"%s\"\n", g_adminsys_admins[i]->guid);
		_FS_Write_S(buffer, handle);

		_FS_Write_S("level  = ", handle);
		buffer = va("\"%d\"\n", g_adminsys_admins[i]->level);
		_FS_Write_S(buffer, handle);

//		_FS_Write_S("flags  = ", handle);
//		buffer = va("%s\n", g_adminsys_admins[i]->flags);
//		_FS_Write_S(buffer, handle);

		trap_FS_Write("\n", 1, handle);
	}

	trap_FS_FCloseFile(handle);

	return qtrue;
}

// for cmd
qboolean G_adminsystem_writeadmins(gentity_t *ent, int skiparg)
{
	if( !G_adminsystem_write_admins() )
		return qfalse;

	return qtrue;
}

// -------------------------------------------------------------------------
// read

qboolean _as_findInt( int handle, pc_token_t *token, int *value )
{
	// skip "=" if there is
	if ( !Q_stricmp( token->string, "=" ) ) {
		if ( !trap_PC_ReadToken( handle, token ) ) {
			return qfalse;
		}
	}

	if( Q_isnumeric( token->string[0] ) ) {
		*value = atoi(token->string);
		return qtrue;
	}

	return qfalse;
}

char *_as_findStr( int handle, pc_token_t *token )
{
	char *string;
	string = "";

	if ( !Q_stricmp( token->string, "=" ) ) {
		if ( !trap_PC_ReadToken( handle, token ) ) {
			return "";
		}
	}
	
	string = token->string;

	return string;
}

qboolean _as_findFlags( int handle, pc_token_t *token, g_adminsys_level_t *l )
{
	int i;
	int j;
	qboolean allfound = qfalse;
	qboolean allow = qtrue;
	char command[1024];

	if ( !Q_stricmp( token->string, "=" ) ) {
		if ( !trap_PC_ReadToken( handle, token ) ) {
			return qfalse;
		}
	}

	// we need a bracket
	if ( !Q_stricmp( token->string, "{" ) ) {
		while ( 1 ) {
			if ( !trap_PC_ReadToken( handle, token ) ) {
				break;
			}

			// we've reached the end
			if ( !Q_stricmp( token->string, "}" ) ) {
#ifdef ANNOUNCE_DEBUG
				DP( va( "FLAGS: ^7%s\n", l->flags ) );
#endif
				return qtrue;
			}

			if ( !Q_stricmp( token->string, "all" ) ) {
				allfound = qtrue;
				Q_strcat( l->flags, sizeof(l->flags), "*" );
#ifdef ANNOUNCE_DEBUG
				DP( va( "ALL\n" ) );
#endif
				continue;
			}

			allow = qtrue;

			if( token->string[0] == '!' || token->string[0] == '-' ) {
				allow = qfalse;
				if( strlen(token->string) <= 1 ) {
					// next token
					if ( !trap_PC_ReadToken( handle, token ) ) {
						break;
					}
					Q_strncpyz( command, token->string, sizeof(command) );
				} else {
					char commandTemp[1024];
					
					for( j=1; j<strlen(token->string); j++ ) {
						commandTemp[j-1] = token->string[j];
						commandTemp[j] = 0;
					}

					Q_strncpyz( command, commandTemp, sizeof(command) );
				}
			} else {
				Q_strncpyz( command, token->string, sizeof(command) );
			}

			for (i=0; g_adminsys_cmds[i].keyword[0]; i++) {
				if(!Q_stricmp(command, g_adminsys_cmds[i].keyword)) {

#ifdef ANNOUNCE_DEBUG
					DP( va("%s : %s\n", command, allow ? "true" : "false" ) );
#endif

					if( !allfound ) {
						if( allow ) {
							Q_strcat( l->flags, sizeof(l->flags), va("%c",g_adminsys_cmds[i].flag) );
						}
					} else {
						if( !allow ) {
							// everything after all will not be allowed
							Q_strcat( l->flags, sizeof(l->flags), va("%c",g_adminsys_cmds[i].flag) );
						}
					}
				}
			}
		}
	}

	return qfalse;
}

qboolean _as_findElement( int *handle, pc_token_t *token, char *element )
{
	qboolean found = qfalse;

	if(!Q_stricmp( token->string, "[" )) {
		while(1) {
			if ( !trap_PC_ReadToken( *handle, token ) ) {
				break;
			}

			if(!Q_stricmp( token->string, "]" )) {
				break;
			}

			if(!Q_stricmp( token->string, element )) {
				found = qtrue;
			}
		}
	}
	return found;
}




qboolean G_adminsystem_read_levels( void )
{
	int handle;
	pc_token_t token;
	char* srcFilename;
	char* dir;
	g_adminsys_level_t *l = NULL;
	int lc = 0;

	dir = "";
	dir = g_adminsysDir.string;

	if( dir[0] ) {
		srcFilename = va("%s/adminlevels.cfg", dir);
	} else {
		//srcFilename = "adminlevels.cfg";
		return qfalse;
	}


#ifdef ANNOUNCE_SHIT
	G_Printf( "^fLoading admin levels from ^7'%s'\n", srcFilename );
#endif

	handle = trap_PC_LoadSource( srcFilename );
	if (!handle) {
		//G_Printf("^1ERROR loading: '%s'\n", srcFilename);
		G_Printf("^1ERROR loading admin levels");
		return qfalse;
	}

	G_adminsys_cleanup_levels();

	#ifdef ANNOUNCE_DEBUG
	DP( "\n" );
	#endif

	while( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if(!Q_stricmp( Q_strlwr(token.string), "define" )) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				break;
			}
			if(!Q_stricmp( token.string, "cmds_prefix" )) {
				if ( !trap_PC_ReadToken( handle, &token ) ) {
					break;
				}
				if( token.string[0] ) {
					trap_Cvar_Set( "g_adminsysPrefix", va( "%s", token.string ) );
					#ifdef ANNOUNCE_DEBUG
					DP( va("define cmds_prefix %s\n\n", token.string ) );
					#endif
				}
			}
		}

		while( 1 ) {
		//if(!Q_stricmp( token.string, "[level]" )) {
			if( _as_findElement( &handle, &token, "level" ) ) {

				#ifdef ANNOUNCE_DEBUG
				DP( "[level]\n" );
				#endif
				
				l = malloc(sizeof(g_adminsys_level_t));
				l->level = 0;
				*l->name = '\0';
				*l->flags = '\0';

				while( 1 ) {
					int lvlvalue = 0;
					char *strname;

					if ( !trap_PC_ReadToken( handle, &token ) ) {
						break;
					}
					// next element...
					if(!Q_stricmp( token.string, "[" )) {
						break;
					}

					if(!Q_stricmp( token.string, "level" )) {
						if ( !trap_PC_ReadToken( handle, &token ) ) {
							break;
						}

						if( !_as_findInt( handle, &token, &lvlvalue ) ) {
							break;
						}
						#ifdef ANNOUNCE_DEBUG
						DP( va("level = %d\n", lvlvalue) );
						#endif
						l->level = lvlvalue;
					}
					else if(!Q_stricmp( token.string, "name" )) {
						if ( !trap_PC_ReadToken( handle, &token ) ) {
							break;
						}
						
						strname = _as_findStr( handle, &token );

						if( !strname[0] ) {
							strname = va( "Level %d User", lvlvalue );
						}

						#ifdef ANNOUNCE_DEBUG
						DP( va("name = %s\n", strname) );
						#endif
						Q_strncpyz( l->name, strname, sizeof(l->name) );
					}
					else if(!Q_stricmp( token.string, "flags" )) {
						if ( !trap_PC_ReadToken( handle, &token ) ) {
							break;
						}

						if( _as_findFlags( handle, &token, l ) ) {
							#ifdef ANNOUNCE_DEBUG
							DP( "\n" );
							#endif
							break;
						}

						//break;
					}
					else {
						//SPC(va("^/readconfig: ^7[level] parse error near "
						//	"%s on line %d",
						//	t,
						//	COM_GetCurrentParseLine()));
						#ifdef ANNOUNCE_DEBUG
						DP( va("^1ERROR2: readlevels (token: %s)\n", token.string) );
						#endif
					}
				} // while(1)

				if(lc >= MAX_ADMINSYS_LEVELS) {
					trap_PC_FreeSource( handle );
					G_Printf( "^1ERROR: readlevels exceeded max levels (%d)\n", MAX_ADMINSYS_LEVELS );
					return qfalse;
				}
				g_adminsys_levels[lc++] = l;

			} // [level]
			else {
				//#ifdef ANNOUNCE_DEBUG
				//DP( va("^1ERROR: readlevels (level) (token: %s)\n", token.string) );
				//#endif
				break;
			}
		}

	} // while(1)

	trap_PC_FreeSource( handle );

	return qtrue;
}


qboolean G_adminsystem_read_admins( void )
{
	int handle;
	pc_token_t token;
	char* srcFilename;
	g_adminsys_admin_t *a = NULL;
	int ac = 0;

	if( g_adminsysDir.string[0] ) {
		srcFilename = va("%s/admins.cfg", g_adminsysDir.string);
	} else {
		//srcFilename = "admins.cfg";
		return qfalse;
	}

	#ifdef ANNOUNCE_SHIT
	G_Printf( "^fLoading admins from ^7'%s'\n", srcFilename );
	#endif

	handle = trap_PC_LoadSource( srcFilename );
	if (!handle) {
		//G_Printf("^1ERROR loading: '%s'\n", srcFilename);
		G_Printf("^1ERROR loading admin levels\n");
		return qfalse;
	}

	G_adminsys_cleanup_admins();

	while( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		while( 1 ) {
			//if(!Q_stricmp( token.string, "[level]" )) {
			if( _as_findElement( &handle, &token, "admin" ) ) {

				#ifdef ANNOUNCE_DEBUG
				DP2( "[admin]\n" );
				#endif
				
				a = malloc(sizeof(g_adminsys_admin_t));
				a->level = 0;
				*a->name = '\0';
				*a->guid = '\0';
				//*a->flags = '\0';

				while( 1 ) {
					int lvlvalue = 0;
					char *strname;
					char *strguid;

					if ( !trap_PC_ReadToken( handle, &token ) ) {
						break;
					}
					// next element...
					if(!Q_stricmp( token.string, "[" )) {
						break;
					}

					if(!Q_stricmp( token.string, "name" )) {
						if ( !trap_PC_ReadToken( handle, &token ) ) {
							break;
						}
						
						strname = _as_findStr( handle, &token );

						if( !strname[0] ) {
							strname = va( "unknown" );
						}

						#ifdef ANNOUNCE_DEBUG
						DP2( va("name  = %s\n", strname) );
						#endif
						Q_strncpyz( a->name, strname, sizeof(a->name) );
					}
					else if(!Q_stricmp( token.string, "guid" )) {
						if ( !trap_PC_ReadToken( handle, &token ) ) {
							break;
						}
						
						strguid = _as_findStr( handle, &token );

						if( !strguid[0] ) {
							strguid = va( "unknown" );
						}

						#ifdef ANNOUNCE_DEBUG
						DP2( va("guid  = %s\n", strguid) );
						#endif
						Q_strncpyz( a->guid, strguid, sizeof(a->guid) );
					}
					else if(!Q_stricmp( token.string, "level" )) {
						if ( !trap_PC_ReadToken( handle, &token ) ) {
							break;
						}

						if( !_as_findInt( handle, &token, &lvlvalue ) ) {
							//break;
							lvlvalue = 0;
						}
						#ifdef ANNOUNCE_DEBUG
						DP2( va("level = %d\n", lvlvalue) );
						#endif
						a->level = lvlvalue;
					}
					else {
						//SPC(va("^/readconfig: ^7[level] parse error near "
						//	"%s on line %d",
						//	t,
						//	COM_GetCurrentParseLine()));

						G_Printf( "^1ERROR: readadmin (token: %s)\n", token.string );
					}
				} // while(1)

				if(ac >= MAX_ADMINSYS_LEVELS) {
					trap_PC_FreeSource( handle );
					G_Printf( "^1ERROR: readadmin exceeded max admins (%d)\n", MAX_ADMINSYS_ADMINS );
					return qfalse;
				}
				g_adminsys_admins[ac++] = a;

				#ifdef ANNOUNCE_DEBUG
				DP2( "\n" );
				#endif

			} // [level]
			else {
				//G_Printf( va("^1ERROR: readadmin (admin) (token: %s)\n", token.string) );
				break;
			}
		}

	} // while(1)

	trap_PC_FreeSource( handle );

	return qtrue;
}



// #########################################################

//void G_adminsystem_Init(gentity_t *ent)
qboolean G_adminsystem_Init(gentity_t *ent, int skiparg)
{
	G_adminsystem_read_levels();
	G_adminsystem_read_admins();

	// only works when called by !readconfig
	if( skiparg >= 0 ) {
		G_Printf("^5*** ent?: %s", ent ? "^2true" : "^1false" );
		G_adminsys_TellClientsUI(NULL);
	}

	return qtrue;
}

#ifdef GS_DEBUG
void G_adminsys_debug_givelevel(gentity_t *ent)
{
	int i;
	char command[MAX_ADMINSYS_CMD_LEN];
	int cmdlevel;
//	int skip = 0;
	qboolean found = qfalse;
	qboolean updated = qfalse;
	char name2[36];
	g_adminsys_admin_t *a;
//	char flags[64];

	if(ent == NULL) {
		G_Printf( "^1ent == NULL\n" );
	}

	if ( Q_SayArgc() < 2 ) {
		CPP( "usage:^7 givelevel [level]" );
		return;
	}

	command[0] = '\0';
	//Q_SayArgv(0, command, sizeof(command));
	Q_SayArgv( 1, command, sizeof( command ) );

	if(!command[0])
		return;

	cmdlevel = atoi( command );

	//G_adminsystem_read_levels();

	for(i=0; g_adminsys_levels[i]; i++) {
		if(g_adminsys_levels[i]->level == cmdlevel) {
//			Q_strncpyz(flags, g_adminsys_levels[i]->flags, sizeof(flags));
			found = qtrue;
			break;
		}
	}
	if( !found ) {
		CPP( va("^1level %d not defined", cmdlevel) );
		return;
	}

	if( strlen(ent->client->sess.guid) < 32 ) {
		CPP( va("^1player has invalid guid (%s) (%d)", ent->client->sess.guid, strlen(ent->client->sess.guid)) );
		return;
	}

	SanitizeString(ent->client->pers.netname, name2, qfalse);

	for(i=0;g_adminsys_admins[i];i++) {
		if(!Q_stricmp(g_adminsys_admins[i]->guid,
			ent->client->sess.guid)) {

			g_adminsys_admins[i]->level = cmdlevel;
			Q_strncpyz(g_adminsys_admins[i]->name, name2,
				sizeof(g_adminsys_admins[i]->name));
			updated = qtrue;
		}
	}
	// not in the 'database' yet
	if(!updated) {
		if(i == MAX_ADMINSYS_ADMINS) {
			SPC("setlevel: ^7too many admins");
			return;
		}
		a = malloc(sizeof(g_adminsys_admin_t));
		a->level = cmdlevel;
		Q_strncpyz(a->name, name2, sizeof(a->name));
		Q_strncpyz(a->guid, ent->client->sess.guid, sizeof(a->guid));
		//*a->flags = '\0';
		//Q_strncpyz(a->flags, flags, sizeof(a->flags));
		g_adminsys_admins[i] = a;
	}

	//G_adminsystem_write_admins(ent, 0);
	G_adminsystem_write_admins();
	G_adminsys_TellClientsUI(ent);

	//return qfalse;
}
#endif

// #################################################################################################################################
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CMDS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


qboolean G_adminsys_cmd_test(gentity_t *ent, int skiparg)
{
	AP( va("chat \"TROLLED!!!\" -1") );
	return qtrue;
}

qboolean G_adminsys_cmd_pinfo(gentity_t *ent, int skiparg)
{
	int i, j;
	int alevel = 0;
	char *lname = NULL;
	char *team;

	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	char name[MAX_NAME_LENGTH] = {""};
	char tempn[MAX_NAME_LENGTH] = {""};
	char akaname[MAX_NAME_LENGTH] = {""};

//	int otherlevel;
//	int ownlevel;
//	qboolean found = qfalse;
//	qboolean updated = qfalse;
	gentity_t *vic;

// ---------------------------------------------------

	if ( Q_SayArgc() < 2+skiparg ) {
		CPP( "usage: ^7pinfo [name|slot#]" );
		return qfalse;
	} else {
		Q_SayArgv( 1+skiparg, name, sizeof( name ) );

		if(ClientNumbersFromString(name, pids) != 1) {
			G_MatchOnePlayer(pids, err, sizeof(err));
			CPP(va("pinfo:^7 %s", err));
			return qfalse;
		}

		vic = &g_entities[pids[0]];
	}
// ---------------------------------------------------
	//vic = ent;
	if( !vic ) {
		return qfalse;
	}

	// Get the level number
	alevel = G_adminsys_getlevel( vic );

	// level name
	for(i=0; g_adminsys_levels[i]; i++) {
		if(g_adminsys_levels[i]->level == alevel) {

			lname = g_adminsys_levels[i]->name;
			break;
		}
	}

	// team
	team = "Spectator";
	if(vic->client->sess.sessionTeam == TEAM_ALLIES) {
		team = "Allies";
	}
	if(vic->client->sess.sessionTeam == TEAM_AXIS) {
		team = "Axis";
	}
	if(vic->client->pers.connected == CON_CONNECTING) {
		team = "Spectator (connecting)";
	}

	// aka
	akaname[0] = '\0';
	for(j=0; g_adminsys_admins[j]; j++) {
		if(!Q_stricmp(g_adminsys_admins[j]->guid,
			vic->client->sess.guid)) {

			SanitizeString(g_adminsys_admins[j]->name,
				tempn, qtrue);
			if(Q_stricmp(name, tempn)) {
				Q_strncpyz(akaname,
					g_adminsys_admins[j]->name,
					sizeof(akaname));
			}
			break;
		}
	}

	CPP(va("\
pinfo: \n\
^7name:  ^3%s^7%s\n\
^7slot:  ^3%d\n\
^7team:  ^3%s\n\
^7level: ^3%d ^3(%s^3)\
",
		vic->client->pers.netname,
		((*akaname) ? va(" (a.k.a. %s)", akaname) : ""),
		vic->client->ps.clientNum,
		team,
		alevel,
		lname
		));

	return qtrue;
}


qboolean G_adminsys_cmd_admintest(gentity_t *ent, int skiparg)
{
	int i;
	int alevel = 0;
	char *lname = NULL;

	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
//	int otherlevel;
//	int ownlevel;
//	qboolean found = qfalse;
//	qboolean updated = qfalse;
	gentity_t *vic;

// ---------------------------------------------------

	if ( Q_SayArgc() < 2+skiparg ) {
		//CPP( "usage: ^7admintest [name|slot#]" );
		if(!ent) {
			CPP("admintest: ^7you are on the console");
			return qtrue;
		}else{
			vic = ent;
		}
	} else {
		Q_SayArgv( 1+skiparg, name, sizeof( name ) );

		if(ClientNumbersFromString(name, pids) != 1) {
			G_MatchOnePlayer(pids, err, sizeof(err));
			CPP(va("admintest:^7 %s", err));
			return qfalse;
		}

		vic = &g_entities[pids[0]];
	}
// ---------------------------------------------------
	//vic = ent;

	// Get the level number
	alevel = G_adminsys_getlevel( vic );

	// level name
	for(i=0; g_adminsys_levels[i]; i++) {
		if(g_adminsys_levels[i]->level == alevel) {

			lname = g_adminsys_levels[i]->name;
			break;
		}
	}

	CPP(va("admintest: ^7%s^7 is a level %d user (%s^7)",
		vic->client->pers.netname,
		alevel,
		lname )
		);


	return qtrue;
}


qboolean G_adminsys_cmd_help(gentity_t *ent, int skiparg)
{
	int i;

	if(Q_SayArgc() < 2+skiparg) {
		//!help
		int j = 0;
		int count = 0;
		char *str = "";


		for (i=0; g_adminsys_cmds[i].keyword[0]; i++) {

			if(G_adminsys_permission(ent, g_adminsys_cmds[i].flag)) {
				if( j == 6){
					str = va( "%s\n^3%-12s", str, g_adminsys_cmds[i].keyword);
					j = 0;
				}else{
					str = va( "%s^3%-12s", str, g_adminsys_cmds[i].keyword);
				}
				j++;
				count++;
			}
		}
		SBP_begin();
		SBP(str);
		// tjw: break str into at least two parts to try to avoid
		//      1022 char limitation
		str = "";
		if(count)
			str = va("%s\n",str);
		SPC(va("^fhelp: ^7%i available commands", count));

		SBP( str );
		SBP("^7Type !help [^3command^7] for help with a specific command.\n");
		SBP_end();

		return qtrue;
	} else {
		//!help param
		char param[20];

		Q_SayArgv(1+skiparg, param, sizeof(param));
		SBP_begin();
		for( i=0; g_adminsys_cmds[i].keyword[0]; i++ ){
			if(	!Q_stricmp(param, g_adminsys_cmds[i].keyword ) ) {
				if(!G_adminsys_permission(ent, g_adminsys_cmds[i].flag)) {
					SBP(va("^/help: ^7you have no permission to use '%s'\n",
						g_adminsys_cmds[i].keyword ) );
					return qfalse;
				}
				SBP( va("^fhelp: ^7help for '%s':\n", g_adminsys_cmds[i].keyword ) );
				SBP( va("^3Function: ^7%s\n", g_adminsys_cmds[i].function ) );
				SBP( va("^3Syntax: ^7!%s %s\n", g_adminsys_cmds[i].keyword,
					g_adminsys_cmds[i].syntax ) );
				SBP( va("^3Flag: ^7'%c'\n", g_adminsys_cmds[i].flag) );
				SBP_end();
				return qtrue;
			}
		}
		SBP( va("^fhelp: ^7no help found for '%s'\n", param ) );
		SBP_end();
		return qfalse;
	}
}


qboolean G_adminsys_cmd_setlevel(gentity_t *ent, int skiparg)
{
	int i;
	int pids[MAX_CLIENTS];
	char lstr[MAX_ADMINSYS_CMD_LEN];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char name2[MAX_NAME_LENGTH];
	int newlevel;
	int ownlevel;
	qboolean found = qfalse;
	qboolean updated = qfalse;
	gentity_t *vic;

	g_adminsys_admin_t *a;
	//char flags[64];

	if(ent == NULL) {
		G_Printf( "setlevel: ^1ent == NULL\n" );
	}

	if ( Q_SayArgc() < 3+skiparg ) {
		CPP( "usage: ^7setlevel [name|slot#] [level]" );
		return qfalse;
	}

	//command[0] = '\0';
	//Q_SayArgv(0, command, sizeof(command));
	Q_SayArgv( 1+skiparg, name, sizeof( name ) );
	Q_SayArgv( 2+skiparg, lstr, sizeof( lstr ) );
	newlevel = atoi( lstr );

	ownlevel = G_adminsys_getlevel(ent);
	if(newlevel > ownlevel) {
		CPP("setlevel: ^7you may not setlevel higher than your current level");
		return qfalse;
	}

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		CPP(va("setlevel:^7 %s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];

	if(G_adminsys_getlevel(vic) > ownlevel) {
		CPP("setlevel: ^7target player has a higher adminlevel than you do");
		return qfalse;
	}

	//G_adminsystem_read_levels();

	for(i=0; g_adminsys_levels[i]; i++) {
		if(g_adminsys_levels[i]->level == newlevel) {
//			Q_strncpyz(flags, g_adminsys_levels[i]->flags, sizeof(flags));
			found = qtrue;
			break;
		}
	}
	if( !found ) {
		CPP( va("setlevel: ^1level %d not defined", newlevel) );
		return qfalse;
	}

	if( strlen(vic->client->sess.guid) < 32 ) {
		CPP( va("setlevel: ^1player has invalid guid (%s) (%d)", vic->client->sess.guid, strlen(vic->client->sess.guid)) );
		return qfalse;
	}

	SanitizeString(vic->client->pers.netname, name2, qfalse);

	for(i=0;g_adminsys_admins[i];i++) {
		if(!Q_stricmp(g_adminsys_admins[i]->guid,
			vic->client->sess.guid)) {

			g_adminsys_admins[i]->level = newlevel;
			Q_strncpyz(g_adminsys_admins[i]->name, name2,
				sizeof(g_adminsys_admins[i]->name));
			updated = qtrue;
		}
	}
	// not in the 'database' yet
	if(!updated) {
		if(i == MAX_ADMINSYS_ADMINS) {
			CPP("setlevel: ^7too many admins");
			return qfalse;
		}
		a = malloc(sizeof(g_adminsys_admin_t));
		a->level = newlevel;
		Q_strncpyz(a->name, name2, sizeof(a->name));
		Q_strncpyz(a->guid, vic->client->sess.guid, sizeof(a->guid));
		//*a->flags = '\0';
		//a->flags = flags;
//		Q_strncpyz(a->flags, flags, sizeof(a->flags));
		g_adminsys_admins[i] = a;
	}

	APM( va("setlevel: ^7%s^7 is now a level %d user",
		vic->client->pers.netname,
		newlevel) );

	G_adminsystem_write_admins();
	G_adminsys_TellClientsUI(vic);

	return qfalse;
}


qboolean G_adminsys_cmd_levlist(gentity_t *ent, int skiparg)
{
 //g_adminsys_levels
	int i;
	int llevel;
	char lname[MAX_NAME_LENGTH];

	if(ent) {
		CPP("\n^7 NR ^n| ^7NAME");
		CPP("^n-------------------------------------------");
	} else {
		G_Printf("\n^7 NR ^n| ^7NAME\n");
		G_Printf("^n-------------------------------------------\n");
	}

	for(i=0; g_adminsys_levels[i]; i++) {

		llevel = g_adminsys_levels[i]->level;

		Q_strncpyz(lname,
			g_adminsys_levels[i]->name,	sizeof(lname));


		if(ent) {
			CPP( va(" ^7%2d ^n| ^7%s", 
				llevel,
				lname) );
		} else {
			G_Printf( " ^7%2d ^n| ^7%s\n", 
				llevel,
				lname );
		}
	}

	return qtrue;
}

// ================================================================


qboolean G_adminsys_cmd_mute(gentity_t *ent, int skiparg)
{
	int pids[MAX_CLIENTS], seconds, modifier = 1;
	char *reason, secs[8];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char command[16], *cmd;
	gentity_t *vic;
//	int i;
//	int alevel = 0;
//	char *lname = NULL;

	Q_SayArgv( 0+skiparg, command, sizeof( command ) );
	cmd = command;
	//if(*cmd == '!'){
	//	cmd++;
	//}

	reason = "";

	if ( Q_SayArgc() < (2+skiparg) ) {
		if(!Q_stricmp(cmd, "mute") || !Q_stricmp(cmd, "!mute")){
			CPP("usage: ^7!mute [name|slot#] (time) (reason)");
		} else {
			// unmute
			CPP("usage: ^7!unmute [name|slot#]");
		}
		return qfalse;
	}


	Q_SayArgv( 1+skiparg, name, sizeof( name ) );


	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		CPP(va("%s:^7 %s", cmd, err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];

	seconds = 120;

	// time
	if ( Q_SayArgc() > (2+skiparg) ) {

		Q_SayArgv( 2+skiparg, secs, sizeof( secs ) );
		if(secs[0]) {
			int lastchar = strlen(secs) - 1;

			if(secs[lastchar] == 'w')
				modifier = 60*60*24*7;
			else if(secs[lastchar] == 'd')
				modifier = 60*60*24;
			else if(secs[lastchar] == 'h')
				modifier = 60*60;
			else if(secs[lastchar] == 'm')
				modifier = 60;

			if (modifier != 1)
				secs[lastchar] = '\0';
		}
		seconds = atoi(secs);
		if(seconds > 0){
			seconds *= modifier;
		}

		if(seconds <= 0){
			reason = Q_SayConcatArgs(2+skiparg);
		}else{
			reason = Q_SayConcatArgs(3+skiparg);
		}
	}

	if( !G_adminsys_admin_higher( ent, vic) ) {
		CPP( va("%s: ^7target has a higher admin level than you do", cmd) );
		return qfalse;
	}

	if(vic->client->sess.auto_unmute_time != 0) {
		if(!Q_stricmp(cmd, "mute")) {
			if(vic->client->sess.auto_unmute_time == -1){
				seconds = 0;
			}else{
				seconds = (vic->client->sess.auto_unmute_time - level.time)/1000;
			}
			CPP(va("mute: ^7player is already muted%s",
				seconds ? va(" for %i seconds", seconds) : ""));
			return qtrue;
		}
		vic->client->sess.auto_unmute_time = 0;
		CPx(pids[0], "cp \"^5You've been unmuted\n\"" );
		CPP(va("unmute: ^3%s^7 has been unmuted",
			vic->client->pers.netname ));
	}else{
		if(!Q_stricmp(cmd, "unmute")) {
			CPP("unmute: ^7player is not muted");
			return qtrue;
		}
		if(seconds<=0){
			vic->client->sess.auto_unmute_time = -1;
		}else{
			vic->client->sess.auto_unmute_time = (level.time + 1000*seconds);
		}
		CPx(pids[0], va("cp \"^5You've been muted%s%s\n\"",
			(seconds > 0) ? va(" for %i seconds",seconds): "",
			*reason ? va(" because: %s", reason): ""));
		//CPP(va("mute: ^3%s^7 has been muted%s",
		//	vic->client->pers.netname,
		//	(seconds > 0) ? va(" for %i seconds",seconds): ""));

		CPP(va("mute: ^3%s^7 has been muted%s%s",
			vic->client->pers.netname,
			(seconds > 0) ? va(" for %i seconds",seconds): "",
			*reason ? va(" (%s)", reason): ""));
	}
	ClientUserinfoChanged(pids[0]);
	return qtrue;
}


qboolean G_adminsys_cmd_kick(gentity_t *ent, int skiparg)
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], *reason, err[MAX_STRING_CHARS];
	int minargc, banTime = 0;

	//minargc = 3+skiparg;
	//if(G_adminsys_permission(ent, SBF_UNACCOUNTABLE))
		minargc = 2+skiparg;

	if(Q_SayArgc() < minargc) {
		CPP("usage: ^7!kick [name] [reason]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	reason = Q_SayConcatArgs(2+skiparg);
	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		CPP(va("kick: ^7%s", err));
		return qfalse;
	}
	if(!G_adminsys_admin_higher(ent, &g_entities[pids[0]])) {
		CPP("kick: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}

	if ( g_kickTime.integer > 0 ) {
		banTime = g_kickTime.integer;
		//G_shrubbot_tempban(pids[0],
		//	va("You have been kicked, Reason: %s",
		//	(*reason) ? reason : "kicked by admin"),
		//	banTime);
	}

	//josh: 2.60 won't kick from the engine. This will call
	//      ClientDisconnect
	trap_DropClient(pids[0],
		va("You have been kicked, Reason: %s\n%s",
		(*reason) ? reason : "kicked by admin", g_dropMsg.string),
		banTime);
	return qtrue;
}


qboolean G_adminsys_cmd_ban(gentity_t *ent, int skiparg)
{
	// [^3name|slot#^7] (^htime^7) (^hreason^7)
	//CPP("ban: ^7not implented yet");
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], *reason, err[MAX_STRING_CHARS];
	int minargc, banTime = -1;
	char stime[6];
	gentity_t *vic;
		//char *guid, *ip;

	minargc = 2+skiparg;

	if(Q_SayArgc() < minargc) {
		CPP("usage: ^7!ban [name] [time] [reason]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	reason = Q_SayConcatArgs(2+skiparg);
	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		CPP(va("ban: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!G_adminsys_admin_higher(ent, vic)) {
		CPP("ban: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}

	if(Q_SayArgc() > 2) {
		Q_SayArgv(2+skiparg, stime, sizeof(stime));
		if( stime[0] ) {
			banTime = atoi(stime);
		}
	}

	reason = Q_SayConcatArgs(3+skiparg);


	//guid = vic->client->sess.guid;
	//ip = vic->client->sess.ip;

	//josh: 2.60 won't kick from the engine. This will call
	//      ClientDisconnect
	trap_DropClient(pids[0],
		va("You have been banned, Reason: %s\n%s",
		(*reason) ? reason : "kicked by admin", g_dropMsg.string),
		banTime);



	return qtrue;
}



// SwatElite: Allow ClanChat

qboolean G_CanUseClanchat(gentity_t *ent){
	//int i;

	// console
	if(!ent){
		return qtrue;
	}
	if(ent->client->sess.referee == RL_REFEREE ||
		ent->client->sess.referee == RL_RCON){
		return qtrue;
	}

	//for( i=0; g_adminsys_cmds[i].keyword[0]; i++ ){
	//	if(	!Q_stricmp("mc", g_adminsys_cmds[i].keyword ) ) {
	//		if(!G_adminsys_permission(ent, g_adminsys_cmds[i].flag)) {
	//			SBP(va("^/help: ^7you have no permission to use '%s'\n",
	//				g_adminsys_cmds[i].keyword ) );
	//			return qfalse;
	//		}
	if(G_adminsys_permission(ent, SBF_CLANCHAT)){
		return qtrue;
	}

	return qfalse;
}

// SwatElite: ClanChat
qboolean G_adminsys_cmd_clanchat(gentity_t *ent, int skiparg) {
	char netname[MAX_NAME_LENGTH];
	char cmd[12];
	char *msg;
	char targets[1024];
	char *targetname;
	int i;
	//int skipargs = 0;
	gentity_t *other;


	if( !G_CanUseClanchat(ent) ) {
		CPP(va("You don't have the premission to use clan-chat\n"));
		return qfalse;
	}

	Q_SayArgv(0, cmd, sizeof(cmd));
/*
	if(!Q_stricmp(cmd, "say") ||
	   !Q_stricmp(cmd, "say_team") ||
	   !Q_stricmp(cmd, "say_buddy")) {
		skipargs = 1;
	}*/


	if(Q_SayArgc() < 2+skiparg) {
		CPP(va("usage: ^7mc [message]\n"));
		return qfalse;
	}

	msg = Q_SayConcatArgs(1+skiparg);

	if(ent) {
		Q_strncpyz(netname, 
			ent->client->pers.netname,
			sizeof(netname));
	} else {
		Q_strncpyz(netname, "console", sizeof(netname));
	}

	strncpy( targets, "", sizeof( targets ) );

	for(i=0; i<level.numConnectedClients; i++) {
		other = &g_entities[level.sortedClients[i]];
			targetname = "";

		if(ent && COM_BitCheck(
			other->client->sess.ignoreClients,
			(ent-g_entities))) {
			continue;
		}

		if(!G_CanUseClanchat(other)){
			continue;
		}

				
		CPx(other-g_entities, va("chat \"^7%s ^7(^8Clan-Chat^7)^8: %s\" %i ",
			netname, msg, ent ? ent-g_entities : -1));

		if(ent && ent == other){
			continue;
		} else {
			if( targets[0] ) {
				Q_strcat( targets, sizeof(targets), "^7, " );
			}
			//Q_strncpyz( targetname, other->client->pers.netname, sizeof(targetname) );
			targetname = va("%s", other->client->pers.netname);
			Q_strcat( targets, sizeof(targets), targetname );
		}
		
		CPx(other-g_entities, va("cp \"^8You received an Clan-Chat from ^7%s\n\"",
			netname));

	}
	unescape_string(msg); //mcwf
	G_LogPrintf( "clanchat: %s: %s\n", netname, msg );

	if( targets[0] ) {
		CPP("^7Clan-Chat has been sent to:\n");
		CPP(va("^7%s\n",targets));
	}
	

	return qtrue;
}



#endif
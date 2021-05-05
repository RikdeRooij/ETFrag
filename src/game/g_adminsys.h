#ifndef _G_ADMINSYS_H
#define _G_ADMINSYS_H

#ifdef GS_ADMINSYS

#define MAX_ADMINSYS_LEVELS 32
// tjw: some nutters !setlevel every player in sight
#define MAX_ADMINSYS_ADMINS 32768
#define MAX_ADMINSYS_BANS 1024
#define MAX_ADMINSYS_FLAGS 64
#define MAX_ADMINSYS_COMMANDS 64
#define MAX_ADMINSYS_WARNINGS 1024

#define MAX_ADMINSYS_CMD_LEN 17


struct g_adminsys_cmd {
	const char *keyword;
	qboolean (* const handler)(gentity_t *ent, int skiparg);
	char flag;
	const char *function;		// used for !help
	const char *syntax;		// used for !help
};

typedef struct g_adminsys_level {
	int level;
	char name[MAX_NAME_LENGTH];
	char flags[MAX_ADMINSYS_FLAGS];

} g_adminsys_level_t;

typedef struct g_adminsys_admin {
	char guid[33];
	char name[MAX_NAME_LENGTH];
	int level;
	//char flags[MAX_ADMINSYS_FLAGS];
} g_adminsys_admin_t;

void G_adminsys_cleanup_all( void );

#ifdef GS_DEBUG
void G_adminsys_debug_givelevel(gentity_t *ent);
#endif

void G_adminsys_TellClientsUI(gentity_t *ent);

//qboolean G_adminsystem_read_levels( void );
//qboolean G_adminsystem_read_admins( void );


// CMDS ========================================
qboolean G_adminsystem_cmd_check(gentity_t *ent);
// ----------------------------------------------------------------
qboolean G_adminsystem_Init(gentity_t *ent, int skiparg);
qboolean G_adminsystem_writeadmins(gentity_t *ent, int skiparg);
// --------------------------------------------------
qboolean G_adminsys_cmd_test(gentity_t *ent, int skiparg);
qboolean G_adminsys_cmd_pinfo(gentity_t *ent, int skiparg);
// -----
qboolean G_adminsys_cmd_admintest(gentity_t *ent, int skiparg);
qboolean G_adminsys_cmd_help(gentity_t *ent, int skiparg);

qboolean G_adminsys_cmd_setlevel(gentity_t *ent, int skiparg);
qboolean G_adminsys_cmd_levlist(gentity_t *ent, int skiparg);

qboolean G_adminsys_cmd_mute(gentity_t *ent, int skiparg);
qboolean G_adminsys_cmd_kick(gentity_t *ent, int skiparg);
qboolean G_adminsys_cmd_ban(gentity_t *ent, int skiparg);

qboolean G_adminsys_cmd_clanchat(gentity_t *ent, int skiparg);

#endif

#endif
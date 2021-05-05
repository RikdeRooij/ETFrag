#ifndef _G_XPSAVE_H
#define _G_XPSAVE_H

#define MAX_XPSAVES 32768
#define MAX_MAPSTATS 1024
#define MAX_DISCONNECTS 1024

typedef struct {
	char guid[33];
	char name[MAX_NAME_LENGTH];
	int time;
	float skill[SK_NUM_SKILLS];
	float hits;
	float team_hits;
	// tjw: lives moved into g_disconnect_t
	//int lives;
    
    // josh: killrating removed, now kill_rating AND kill_variance
    float kill_rating;
    float kill_variance;

	// Dens: store the real time when someone needs to be unmuted
	// 0 means not muted, -1 means permanent (never)
	int mutetime;

} g_xpsave_t;

typedef struct {
	g_xpsave_t *xpsave;
	int axis_time;
	int allies_time;
	team_t last_playing_team;
	int lives;
	float killrating;
	float killvariance;
	// These are temp variables for PR system usage, not really tracked
	float total_percent_time;
	float diff_percent_time;
} g_disconnect_t;

void G_xpsave_writeconfig();
void G_xpsave_readconfig();
qboolean G_xpsave_add(gentity_t *ent,qboolean disconnect);
qboolean G_xpsave_load(gentity_t *ent);
g_disconnect_t *G_xpsave_disconnect(int i);
void G_reset_disconnects();
void G_xpsave_resetxp();
void G_xpsave_cleanup();

// pheno
void G_xpsave_writexp();
#endif /* ifndef _G_XPSAVE_H */


#include "g_local.h"
#include "g_http_client.h"

g_matchinfo_t *top_match_info = NULL;

void print_playermatchinfo(g_playermatchinfo_t *player) {
	G_LogPrintf("Player Match info "
			"Name: %s "
			"guid: %s "
			"Axis: %d "
			"Allies: %d\n",
			player->name,
			player->guid,
			player->axis_time,
			player->allies_time);
}

void print_matchinfo(g_matchinfo_t *matchinfo) {
	int i;
	G_LogPrintf("Match info "
			"Map: %s "
			"Winner: %d "
			"Length: %d\n",
			matchinfo->map,
			matchinfo->winner,
			matchinfo->length);
	for (i = 0; i< matchinfo->num_players; ++i) {
		print_playermatchinfo(&matchinfo->players[i]);
	}
}

g_matchinfo_t *G_matchinfo_pop() {
	g_matchinfo_t *popped = top_match_info;
	top_match_info = popped->nextMatchInfo;
	popped->nextMatchInfo = NULL;
	return popped;
}

void G_matchinfo_delete(g_matchinfo_t * matchinfo) {
	free(matchinfo);
}


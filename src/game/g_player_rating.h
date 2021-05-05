#ifndef _G_PLAYER_RATING_H
#define _G_PLAYER_RATING_H

#define SIGMA2_THETA 1.0f // prior on player rating variance
#define SIGMA2_PSI 1.0f // prior on server rating variance
#define SIGMA2_GAMMA 1.0f // prior on allies map rating variance
#define SIGMA2_DELTA 1.0f // prior on kill rating variance

void G_UpdateKillRatings(gentity_t *killer, gentity_t *victim, int mod);
void G_LogKillGUID(gentity_t *killer, gentity_t *victim, int mod);
float G_GetAdjKillsPerDeath(float rating, float variance);

#endif /* ifndef _G_PLAYER_RATING_H */

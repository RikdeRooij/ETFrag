// g_player_rating.c: Handles player and kill ratings
// -------------------------
//

#include "g_local.h"


// these are just for logging, the client prints its own messages
char *modNames2[] =
{
	"MOD_UNKNOWN",
	"MOD_MACHINEGUN",
	"MOD_BROWNING",
	"MOD_MG42",
	"MOD_GRENADE",
	"MOD_ROCKET",

	// (SA) modified wolf weap mods
	"MOD_KNIFE",
	"MOD_LUGER",
	"MOD_COLT",
	"MOD_MP40",
	"MOD_THOMPSON",
	"MOD_STEN",
	"MOD_GARAND",
	"MOD_SNOOPERSCOPE",
	"MOD_SILENCER",	//----(SA)	
	"MOD_FG42",
	"MOD_FG42SCOPE",
	"MOD_PANZERFAUST",
	"MOD_GRENADE_LAUNCHER",
	"MOD_FLAMETHROWER",
	"MOD_GRENADE_PINEAPPLE",
	"MOD_CROSS",
	// end

	"MOD_MAPMORTAR",
	"MOD_MAPMORTAR_SPLASH",

	"MOD_KICKED",
	"MOD_GRABBER",

	"MOD_DYNAMITE",
	"MOD_AIRSTRIKE", // JPW NERVE
	"MOD_SYRINGE",	// JPW NERVE
	"MOD_AMMO",	// JPW NERVE
	"MOD_ARTY",	// JPW NERVE

	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
	"MOD_EXPLOSIVE",

	"MOD_CARBINE",
	"MOD_KAR98",
	"MOD_GPG40",
	"MOD_M7",
	"MOD_LANDMINE",
	"MOD_SATCHEL",
	"MOD_TRIPMINE",
	"MOD_SMOKEBOMB",
	"MOD_MOBILE_MG42",
	"MOD_SILENCED_COLT",
	"MOD_GARAND_SCOPE",

	"MOD_CRUSH_CONSTRUCTION",
	"MOD_CRUSH_CONSTRUCTIONDEATH",
	"MOD_CRUSH_CONSTRUCTIONDEATH_NOATTACKER",

	"MOD_K43",
	"MOD_K43_SCOPE",

	"MOD_MORTAR",

	"MOD_AKIMBO_COLT",
	"MOD_AKIMBO_LUGER",
	"MOD_AKIMBO_SILENCEDCOLT",
	"MOD_AKIMBO_SILENCEDLUGER",

	"MOD_SMOKEGRENADE",

	// RF
	"MOD_SWAP_PLACES",

	// OSP -- keep these 2 entries last
	"MOD_SWITCHTEAM",

	"MOD_GOOMBA",
	"MOD_FEAR",
	"MOD_THROWN_KNIFE",
	"MOD_REFLECTED_FF",
};


float g_of_x(float variance) {
	return 1.0 / sqrt(1.0+variance*3.0/(M_PI*M_PI));
}

float kill_probability(float net, float g_of_x_margin) {
	return 1.0 / (1.0+exp(-(net*g_of_x_margin)));
}

void UpdateKillRating(float* rating, float* variance, float input, float margin_variance, float net) {
	float
		g_of_x_margin
		,probability
		,error
		,deriv_error_net
		,first_derivative
		,second_derivative
	;
	g_of_x_margin = g_of_x(margin_variance);
	probability = kill_probability(net,g_of_x_margin);
	error = 1.0 - probability;
	deriv_error_net = probability*error;
	first_derivative = input*error*g_of_x_margin;
	second_derivative = g_of_x_margin*g_of_x_margin*input*input*deriv_error_net;
	(*variance) = 1.0 / (1.0/(*variance) + second_derivative);
	(*rating) += (*variance)*first_derivative;
}

void G_UpdateKillRatings(gentity_t *killer, gentity_t *victim, int mod) {
	float
		killer_variance
		,victim_variance
		,distance_variance
		,distance_weight
		,net
	;
	vec3_t distance_vector;
	//g_serverstat_t *server = G_xpsave_serverstat();
	float one = 1.0f;
	float zero = 0.0f;

	//  calculate distance apart
	VectorSubtract(killer->r.currentOrigin,victim->r.currentOrigin,distance_vector);
	distance_weight = VectorLength(distance_vector)/ 10000.0;

	// first do overall
	killer_variance = killer->client->sess.overall_killvariance;
	victim_variance = victim->client->sess.overall_killvariance;
//	distance_variance = server->distance_variance*distance_weight*distance_weight;
	distance_variance = 1.0f;

	net = killer->client->sess.overall_killrating
		- victim->client->sess.overall_killrating;
//		+ server->distance_rating*distance_weight;

	// distance update marginalizes out killer and victim variance
	UpdateKillRating(
//			&server->distance_rating
//			,&server->distance_variance
			&zero
			,&one
			,distance_weight
			,killer_variance+victim_variance
			,net
		);

	// killer update
	UpdateKillRating(
			&killer->client->sess.overall_killrating
			,&killer->client->sess.overall_killvariance
			,1.0
			,distance_variance+victim_variance
			,net
		);

	// victim update
	UpdateKillRating(
			&victim->client->sess.overall_killrating
			,&victim->client->sess.overall_killvariance
			,-1.0
			,distance_variance+killer_variance
			,net
		);
}

void G_LogKillGUID(gentity_t *killer, gentity_t *victim, int mod) {

	char 
		*killerClass = NULL
		,*victimClass = NULL
		, *obit = NULL
	;
	vec3_t distance_vector;
	float 
		distance_apart
	;

	if(mod < 0 
		|| mod >= sizeof(modNames2) 
		/ sizeof(modNames2[0])) {
		obit = "<bad obituary>";
	} else {
		obit = modNames2[mod];
	}

	if(mod < 0 
		|| mod >= sizeof(modNames2) 
		/ sizeof(modNames2[0])) {
		obit = "<bad obituary>";
	} else {
		obit = modNames2[mod];
	}

	//  calculate distance apart
	VectorSubtract(killer->r.currentOrigin,victim->r.currentOrigin,distance_vector);
	distance_apart = VectorLength(distance_vector);

	switch (killer->client->sess.playerType) {
		case PC_SOLDIER:
			killerClass = "SOLDIER";
			break;
		case PC_MEDIC:
			killerClass = "MEDIC";
			break;
		case PC_ENGINEER:
			killerClass = "ENGINEER";
			break;
		case PC_FIELDOPS:
			killerClass = "FIELDOPS";
			break;
		case PC_COVERTOPS:
			killerClass = "COVERTOPS";
			break;
	}
	switch (victim->client->sess.playerType) {
		case PC_SOLDIER:
			victimClass = "SOLDIER";
			break;
		case PC_MEDIC:
			victimClass = "MEDIC";
			break;
		case PC_ENGINEER:
			victimClass = "ENGINEER";
			break;
		case PC_FIELDOPS:
			victimClass = "FIELDOPS";
			break;
		case PC_COVERTOPS:
			victimClass = "COVERTOPS";
			break;
	}

	G_LogPrintf("KILL_RATING_DATASET: %s %i %s %i %i %f:"
		" \\%s\\ class: %s killed \\%s\\ class: %s by %s at distance %f\n",
		killer->client->sess.guid, 
		killer->client->sess.playerType,
		victim->client->sess.guid,
		victim->client->sess.playerType,
		mod,
		distance_apart,
		killer->client->pers.netname,
		killerClass,
		victim->client->pers.netname,
		victimClass,
		obit,
		distance_apart
	);
}

float G_GetAdjKillsPerDeath(float rating, float variance) {
	float probability = kill_probability(rating,g_of_x(variance));
	return probability / (1.0f - probability);
}

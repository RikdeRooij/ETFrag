// codePRK


#include "g_local.h"

#ifdef GS_AIMBOT

qboolean AimSafe(gentity_t *ent);


// ############################################################################################################

//int targetList[MAX_CLIENTS];
int targetCrossOffset[MAX_CLIENTS];
void CrosshairDistance( gentity_t *ent, int targetNum )
{
	vec3_t	trans;
	float	z;
	vec3_t forward, right, up;
	vec3_t point;
	float x;

	VectorCopy( g_entities[level.sortedClients[targetNum]].r.currentOrigin, point );

	AngleVectors( ent->client->ps.viewangles, forward, right, up);

	VectorSubtract( point, ent->client->ps.origin, trans );
	z = DotProduct( trans, forward );

	if( z <= .001f ) {
		return;
	}

	x = DotProduct( trans, right ) / ( z );

	targetCrossOffset[targetNum] = abs(x); // only want to know difference
}


int targetList[MAX_CLIENTS];
int numTargets;

void G_ListTargets( gentity_t *ent )
{
	int i;
	gentity_t *player = NULL;
	gentity_t *head = NULL;
	qboolean seebody = qfalse, seehead = qfalse;

	numTargets = 0;

	for( i = 0; i < level.numConnectedClients; i++ ) {
		player = &g_entities[level.sortedClients[i]];

		if ( OnSameTeam( player, ent ) ) {
			continue;
		}
		if (!AimSafe( player ) ) {
			continue;
		}
		if ( player == ent ) {
			continue;
		}
			
		// spawn protection
		if( player->client->ps.powerups[PW_INVULNERABLE] >= level.time ) {
			continue;
		}

		// can't see
		head = G_BuildHead(player);
		seehead = IsVisible(ent,head);
		seebody = IsVisible(ent,player);
		if (!seehead && !seebody) {
			G_FreeEntity( head );
			continue;
		}
		G_FreeEntity( head );


		//CrosshairDistance( ent, i );

		targetList[numTargets] = i;
		numTargets++;
	}
}


vec3_t entorg;
int QDECL SortDistance( const void *a, const void *b ) {
	gentity_t *player1 = &g_entities[level.sortedClients[*((int*)a)]];
	gentity_t *player2 = &g_entities[level.sortedClients[*((int*)b)]];
	float p1dis;
	float p2dis;

	p1dis = VectorDistance( entorg, player1->r.currentOrigin );
	p2dis = VectorDistance( entorg, player2->r.currentOrigin );

	if (p1dis < p2dis)
		return -1;
	else 
		return 1;

	return 0;
}

int QDECL SortCrosshair( const void *a, const void *b )
{
	//gentity_t *player1 = &g_entities[level.sortedClients[*((int*)a)]];
	//gentity_t *player2 = &g_entities[level.sortedClients[*((int*)b)]];

	if ( targetCrossOffset[*((int*)a)] < targetCrossOffset[*((int*)b)] )
		return -1;
	else
		return 1;
}

gentity_t *target;

void G_DoAimbotTest( gentity_t *ent, vec3_t vieworg )
{
	gentity_t *head = NULL;
	vec3_t			targetorigin;
	vec3_t			targetoriginmod;
	vec3_t			vieworgreal;
	//int			radius = 16000; // big enough to catchall
	float			fieldOfView = 16.0f; //360.0f;
	qboolean 		seebody = qfalse, seehead = qfalse;
	vec3_t org, ang;
	//vec3_t vieworg;
	//qboolean railtrail = qfalse;

	float predOther = 2000; //90;
	float predSelf = 10; //60;

	//if( !bullet_fire ) {
	//	predOther = 20;
	//	predSelf = 20;
	//}

	if(!ent->client->pers.aimbothack) {
		return;
	}

// ++++++++++++++++++++++++++++++++++++++++++++++++
	
	G_ListTargets( ent );

	if( numTargets > 0 ) {
		VectorCopy( ent->r.currentOrigin, entorg ); // SortDistance needs to know our org
		qsort(targetList, numTargets, sizeof(int), SortDistance);
		//qsort(targetList, numTargets, sizeof(int), SortCrosshair);

		target = &g_entities[level.sortedClients[targetList[0]]];;
	} else {
		target = NULL;
		return;
	}

	if ( !target ) {
		return;
	}
	if (VectorCompare( target->r.currentOrigin, vec3_origin)) {
		//CP( "print \"aimhack: ERROR\"" );
		target = NULL;
		return;
	}

	head = G_BuildHead(target);
	seehead = IsVisible(ent,head);
	seebody = IsVisible(ent,target);
	if (!seehead && !seebody) {
		// nobody to see
		G_FreeEntity( head );
		target = NULL;
		return;
	}
	if (seehead) { // always aim at the head if visible
		VectorCopy( head->r.currentOrigin, targetorigin );
		targetorigin[2] += 6.f; // codePRK
	} else {
		VectorCopy( target->r.currentOrigin, targetorigin );
	}
	G_FreeEntity( head );


	//return;
	// safe

	if ( VectorCompare(targetorigin, vec3_origin) ) {
		return;
	}


	if( VectorCompare( vieworg, vec3_origin ) ) {

		//railtrail = qtrue;

		VectorCopy(ent->client->ps.origin, vieworgreal);
		vieworgreal[2] += ent->client->ps.viewheight;

		SnapVector(vieworgreal);
	} else {
		VectorCopy( vieworg, vieworgreal );
	}


	// return;
	// not safe

// ----------------------
	// PREDICT

	// enemy
	VectorMA( targetorigin, predOther*0.001, target->s.pos.trDelta, targetoriginmod );
	// self
	VectorMA( targetorigin, -predSelf*0.001,  ent->client->ps.velocity, targetoriginmod );

// ----------------------

	//if( railtrail ) {
	//	G_RailTrail( ent->client->ps.clientNum, vieworgreal, targetoriginmod, colorRed, 10 );
	//}

	//}// else {
	//	G_RailTrail( ent->client->ps.clientNum, vieworgreal, targetoriginmod, colorCyan, 10 );
	//}

	// stop here if the weapon doesn't fire bullets
	if( !IS_BULLET_WEAPON(ent->client->ps.weapon) ) {
		target = NULL;
		return;
	}

	// not ready to shoot
	if( ent->client->ps.weaponTime > 500 ) {
		return;
	}

	if((ent->client->ps.weapHeat[ent->client->ps.weapon] > 0) && (
		ent->client->ps.weapHeat[ent->client->ps.weapon] >= 
		( GetAmmoTableData(ent->client->ps.weapon)->maxHeat - (GetAmmoTableData(ent->client->ps.weapon)->nextShotTime * 1.2) )) ) {
		return;
	}
	//G_Printf( "^1%d\n", ent->client->ps.weapHeat[ent->client->ps.weapon] );

	// Get the vector difference and calc the angle form it
	VectorSubtract(targetoriginmod, vieworgreal, org);
	vectoangles(org, ang);

	// Clip & normalize the angle
	#define ANG_CLIP(ang) if(ang>180.0f)ang-=360.0f;else if(ang<-180.0f)ang+=360.0f
	//ang[PITCH] = -ang[PITCH];
	ANG_CLIP(ang[YAW]);
	ANG_CLIP(ang[PITCH]);
	ANG_CLIP(ang[YAW]);
	ANG_CLIP(ang[PITCH]);

	if( ang[YAW] < -180.0 )
		ang[YAW] += 360;
	if( ang[YAW] > 180.0 )
		ang[YAW] -= 360;
	if( ang[PITCH] < -180.0 )
		ang[PITCH] += 360;
	if( ang[PITCH] > 180.0 )
		ang[PITCH] -= 360;

	if (ang[YAW] < -180.0 || ang[YAW] > 180.0 || ang[PITCH] < -180.0 || ang[PITCH] > 180.0) {
		target = NULL;
		return;
	}

	//SetClientViewAngle( ent, ang );
	if( 1 ) {
		// try 'humanized'
		vec3_t cangles;
		vec3_t modangles;
		float step;
		float minstep = 2; // never go under this value, prevent really slow aiming
		int humanize = 8;
		float diff;

		if( humanize > 3 )
			humanize -= floor( VectorDistance(ent->r.currentOrigin, target->r.currentOrigin)/(128.f) );

		if( humanize < 3 )
			humanize = 3;


		VectorCopy( ent->client->ps.viewangles, cangles );
		VectorCopy( ent->client->ps.viewangles, modangles );

// ------------------------------------------------------
// PITCH
		step = 3.f;
		if( humanize <= 1 ) {
			modangles[PITCH] = ang[PITCH]; // instant
		} else {

			// 90 | -90
			if( ang[PITCH] > cangles[PITCH] )
				modangles[PITCH] += step;
			else if( ang[PITCH] < cangles[PITCH] )
				modangles[PITCH] -= step;

			if( abs(ang[PITCH] - cangles[PITCH]) < step )
				modangles[PITCH] = ang[PITCH];
		}

// ------------------------------------------------------
// YAW
		if( humanize <= 1 ) {
			modangles[YAW] = ang[YAW]; // instant
		} else {

			// at the 180 to -180 flip it fucks up (obviously)
			if( ( ang[YAW] < -90 && cangles[YAW] > 90 ) || (ang[YAW] > 90 && cangles[YAW] < -90 ) ) {
				// -90 | -180/180 | 90
				if( ang[YAW] < -90 ) {
					ang[YAW] += 180;
				} else if( ang[YAW] > 90 ) {
					ang[YAW] -= 180;
				}

				if( cangles[YAW] < -90 ) {
					cangles[YAW] += 180;
				} else if( cangles[YAW] > 90 ){
					cangles[YAW] -= 180;
				}
				// result: 90 | 0 | -90

				// invert
				cangles[YAW] += 180;
				ang[YAW] += 180;
				// 270 180 90

				diff = (abs(ang[YAW] - cangles[YAW]) / humanize);
				if( diff < minstep ) { diff = minstep; }
				step = diff;

				if( ang[YAW] >= cangles[YAW] ) {
					modangles[YAW] += step;
				} else if( ang[YAW] < cangles[YAW] ) {
					modangles[YAW] -= step;
				}
			} else {
				// -90 | 0 | 90

				diff = (abs(ang[YAW] - cangles[YAW]) / humanize);
				if( diff < minstep ) { diff = minstep; }
				step = diff;

				if( ang[YAW] >= cangles[YAW] ) {
					modangles[YAW] += step;
				} else if( ang[YAW] < cangles[YAW] ) {
					modangles[YAW] -= step;
				}
			}

			if( abs(ang[YAW] - cangles[YAW]) < step ) { // step*(humanize/1.5)
				if ( humanize > 6 && (abs(ang[YAW] - cangles[YAW]) < step) ) {
					modangles[YAW] = ang[YAW]; // still do this
				} else {
					modangles[YAW] = ang[YAW];
				}
			}
		}
// ------------------------------------------------------

		// not in fov
		if( abs((ang[YAW]+180) - (cangles[YAW]+180)) > (fieldOfView / 2) ) {
			target = NULL;
			return;
		}

		if( abs((ang[PITCH]+180) - (cangles[PITCH]+180)) > (fieldOfView / 2) ) {
			target = NULL;
			return;
		}

		if(BG_IsScopedWeapon(ent->client->ps.weapon)) {
			//float pitchAmp, yawAmp;
			float pitchMinAmp, yawMinAmp;
			float spreadfrac, phase;

			spreadfrac = ent->client->currentAimSpreadScale;

			if( ent->client->ps.weapon == WP_FG42SCOPE ) {
				//pitchAmp = 4*ZOOM_PITCH_AMPLITUDE;
				//yawAmp = 4*ZOOM_YAW_AMPLITUDE;
				pitchMinAmp = 4*ZOOM_PITCH_MIN_AMPLITUDE;
				yawMinAmp = 4*ZOOM_YAW_MIN_AMPLITUDE;
			} else {
				//pitchAmp = ZOOM_PITCH_AMPLITUDE;
				//yawAmp = ZOOM_YAW_AMPLITUDE;
				pitchMinAmp = ZOOM_PITCH_MIN_AMPLITUDE;
				yawMinAmp = ZOOM_YAW_MIN_AMPLITUDE;
			}

			// rotate 'forward' vector by the sway
			phase = level.time / 1000.0 * ZOOM_PITCH_FREQUENCY * M_PI * 2;
			modangles[PITCH] -= ZOOM_PITCH_AMPLITUDE * sin( phase ) * (spreadfrac+pitchMinAmp);

			phase = level.time / 1000.0 * ZOOM_YAW_FREQUENCY * M_PI * 2;
			modangles[YAW] -= ZOOM_YAW_AMPLITUDE * sin( phase ) * (spreadfrac+yawMinAmp);
		}

		SetClientViewAngle( ent, modangles );

	}

	// Back to the future
//	G_HistoricalTraceEnd( ent );
}

#endif


#include "g_local.h"

// ========================================================================================================
#ifdef GS_TURRETS

/*
=======================
Turret code. for spawning and seting the spawn location of turrets
=======================
*/
#define TURRET_RANGE 2000
#define TURRET_RATEOFFIRE 60

//#define TURRET_DEBUG


// ==========================================================

// --------------------------------------------------------------------------
// gives origin and angels, and returns if it is ok for it 2 be there or not
// --------------------------------------------------------------------------

qboolean GetTurretSpawnLoc( gentity_t *player, vec3_t origin, vec3_t angles ) {
	vec3_t		temporigin;
	vec3_t		tempangles;
	trace_t		trace;
	vec3_t		start;
	vec3_t		end;
	vec3_t		forward;
	qboolean	placesafe = qtrue;

	VectorCopy( player->s.pos.trBase, start );
	AngleVectors( player->s.apos.trBase, forward, NULL, NULL );
	start[2] += 32;
	VectorMA( start, 64, forward, start );

	VectorCopy( start, end );
	end[2] -= 64;
	
	trap_Trace( &trace, start, NULL, NULL, end, player->s.number, MASK_SOLID );
	if ( trace.fraction == 1.0 || trace.startsolid ) { // we didn't hit anything :o
		placesafe = qfalse;
	}

	VectorCopy( trace.endpos, temporigin ); // on the ground pls
	temporigin[2] += 1.0;

// ----------------

	if ( trace.plane.normal[2] != 1.0f && trace.fraction != 1.0 ) { // not flat
		float yawtest;
		vectoangles( trace.plane.normal, tempangles );
		tempangles[0] += 90;

		yawtest = tempangles[1] - player->s.apos.trBase[1];
		if( yawtest > 90 && yawtest < 270 ) {
			// yaw invert   (1)
			// pitch invert (0)
			tempangles[1] += 180;
			tempangles[0] = -tempangles[0];
		}
	} else {
		VectorCopy( player->s.apos.trBase, tempangles );
		tempangles[0] = tempangles[2] = 0.0f;
	}

	//if ( trace.fraction == 1.0 ) {
	//}
	if( (tempangles[0] < -90) || (tempangles[0] > 90) ) {
		placesafe = qfalse;
		tempangles[0] += 180;
	}

	if( origin )
		VectorCopy( temporigin, origin );
	if( angles )
		VectorCopy( tempangles, angles );

	return placesafe;
}


void SetTurretBase( gentity_t *base, qboolean test ) {
	vec3_t		origin;
	vec3_t		angles;
	qboolean	placesafe;

	placesafe = GetTurretSpawnLoc( base->parent, origin, angles );

	if( test && (!placesafe) ) {
		base->s.powerups = STATE_CANTBUILDHERE;
	} else if( test ) {
		base->s.powerups = STATE_UNDERCONSTRUCTION;
	}

	G_SetOrigin( base, origin );
	G_SetAngle( base, angles );
}


void SetTurretGun( gentity_t *turret, gentity_t *base, qboolean test ) {
	vec3_t	forward, up;
	vec3_t		origin;
	vec3_t		angles;

	if( !base ) {
		//SetTurretBase( turret, qtrue );
		//VectorCopy( turret->s.pos.trBase, origin );
		//VectorCopy( turret->s.apos.trBase, angles );
		VectorCopy( turret->tagParent->s.pos.trBase, origin );
		VectorCopy( turret->tagParent->s.apos.trBase, angles );

		if( test && (turret->tagParent->s.powerups == STATE_CANTBUILDHERE) ) {
			turret->s.powerups = STATE_CANTBUILDHERE;
		} else if( test ) {
			turret->s.powerups = STATE_UNDERCONSTRUCTION;
		}
	} else {
		VectorCopy( base->s.pos.trBase, origin );
		VectorCopy( base->s.apos.trBase, angles );

		if( test && (base->s.powerups == STATE_CANTBUILDHERE) ) {
			turret->s.powerups = STATE_CANTBUILDHERE;
		} else if( test ) {
			turret->s.powerups = STATE_UNDERCONSTRUCTION;
		}
	}
	// -------------------------

	AngleVectors( angles, forward, NULL, up );
	// origin[2] += 48.f;
//	VectorMA( origin, 48, up, origin );
//	VectorMA( origin, 12, forward, origin );
	VectorMA( origin, 43, up, origin );
//	VectorMA( origin, 12, forward, origin );

	G_SetOrigin( turret, origin );
	G_SetAngle( turret, angles );

	VectorCopy( angles, turret->backorigin );
}


qboolean checkent(gentity_t *ent){

	if (!ent) // Do we have a target?
		return qfalse;
	if (!ent->inuse) // Does the target still exist?
		return qfalse;
	if(!ent->client) // is the target a bot or player?
		return qfalse;
	if(ent->client->sess.sessionTeam != TEAM_AXIS 
		&& ent->client->sess.sessionTeam != TEAM_ALLIES)
		return qfalse;
	if((ent->client->ps.pm_flags & PMF_LIMBO))
		return qfalse;
	if(ent->health <= 0)
		return qfalse;
	if(!(ent->client->ps.pm_type == PM_NORMAL))
		return qfalse;

	if( !g_turrets.integer )
		return qfalse;

	return qtrue;
}

qboolean checktarget(gentity_t *self,gentity_t *target){
	vec3_t 		distance;
	vec3_t 		forward;
	trace_t		trace;
	float		dot;
	gentity_t *	owner = self->parent->parent;

	if(!(owner && owner->inuse && owner->client)) {
		return qfalse;
	}

	//	returns qfalse if the target is not valid. returns qtrue if it is

	if (!target) // Do we have a target?
		return qfalse;
	if (!target->inuse) // Does the target still exist?
		return qfalse;
	if(!target->client) // is the target a bot or player?
		return qfalse;
	if(target->client->sess.sessionTeam != TEAM_AXIS 
		&& target->client->sess.sessionTeam != TEAM_ALLIES)
		return qfalse;
	if((target->client->ps.pm_flags & PMF_LIMBO))
		return qfalse;
	if(target->health <= 0)
		return qfalse;
	if(!(target->client->ps.pm_type == PM_NORMAL))
		return qfalse;
	if (!target->takedamage)
		return qfalse;

	// impossible, turret is not a client
	//if (target==self)
	//	return qfalse;


#ifndef TURRET_DEBUG
	if (target==owner) // is the target the owner of the turret?
		return qfalse;

	if (OnSameTeam(owner, target)) // is the target on same team?
		return qfalse;
#endif


	VectorSubtract(target->s.pos.trBase, self->s.pos.trBase, distance);
	if (VectorLength(distance)>TURRET_RANGE) // is the target within range?
		return qfalse;
	
	if (VectorLength(distance)<40) // is the target not to close
		return qfalse;

	trap_Trace (&trace, self->s.pos.trBase, NULL, NULL, target->s.pos.trBase, self->s.number, MASK_SHOT );
	if ( trace.contents & CONTENTS_SOLID ) // can we see the target?
		return qfalse;


	AngleVectors( self->backorigin, forward, NULL, NULL );
	VectorNormalize( distance );
	dot = DotProduct( distance, forward );
	if(!(dot > 0.3))  //is it in front of us?
		return qfalse;


	return qtrue;
}


void turret_findenemy( gentity_t *ent){
	gentity_t *target;
	int i;

	//target = g_entities;
	//for (; target < &g_entities[level.num_entities]; target++)
	//{
	//ent->enemy = NULL;

	for( i = 0; i < level.numConnectedClients; i++ ) {
		target = &g_entities[level.sortedClients[i]];

		//trap_SendServerCommand( -1, va("print \"^2SEARCHING TARGET\n\"") );

		if(!checktarget(ent,target))
			continue;

		ent->enemy=target;
		//trap_SendServerCommand( -1, va("print \"^2FOUND TARGET\n\"") );
		return;
	}

	ent->enemy=NULL;
}


void turret_trackenemyslow( vec3_t currentAngles, vec3_t targetAngles, vec3_t outAngles ) {
	//vec3_t outAngles;
	float trackspeed;

	if( VectorCompare( currentAngles, targetAngles ) ) {
		VectorCopy( targetAngles, outAngles );
		return;
	}

	trackspeed = 4.0f;
	VectorCopy( currentAngles, outAngles );

#define ANG_CLIP(angle) if( angle > 180.0f ) { angle -= 360.0f; } else if( angle < -180.0f ) { angle += 360.0f; }

	ANG_CLIP(targetAngles[YAW]);
	ANG_CLIP(targetAngles[PITCH]);
	ANG_CLIP(currentAngles[YAW]);
	ANG_CLIP(currentAngles[PITCH]);

// ------------------------------------------------------
// PITCH

	// 90 | -90
	if( targetAngles[PITCH] > currentAngles[PITCH] )
		outAngles[PITCH] += trackspeed;
	else if( targetAngles[PITCH] < currentAngles[PITCH] )
		outAngles[PITCH] -= trackspeed;

	if( abs(targetAngles[PITCH] - currentAngles[PITCH]) < trackspeed )
		outAngles[PITCH] = targetAngles[PITCH];

// ------------------------------------------------------
// YAW

	// at the 180 to -180 flip it fucks up (obviously)
	if( ( targetAngles[YAW] < -90 && currentAngles[YAW] > 90 ) || (targetAngles[YAW] > 90 && currentAngles[YAW] < -90 ) ) {
		if( targetAngles[YAW] < -90 )
			targetAngles[YAW] += 180;
		else if( targetAngles[YAW] > 90 )
			targetAngles[YAW] -= 180;

		if( currentAngles[YAW] < -90 )
			currentAngles[YAW] += 180;
		else if( currentAngles[YAW] > 90 )
			currentAngles[YAW] -= 180;

		currentAngles[YAW] += 180;
		targetAngles[YAW] += 180;
	}

	if( targetAngles[YAW] >= currentAngles[YAW] ) 
		outAngles[YAW] += trackspeed;
	else if( targetAngles[YAW] < currentAngles[YAW] )
		outAngles[YAW] -= trackspeed;

	if( abs(targetAngles[YAW] - currentAngles[YAW]) < trackspeed )
		outAngles[YAW] = targetAngles[YAW];

}

void turret_trackenemy( gentity_t *ent ) {
	vec3_t dir;
	vec3_t enemyorg;
	vec3_t viewang;
//	int i;


	if (!checktarget(ent,ent->enemy)) {
		return;
	}

	VectorCopy(ent->s.apos.trBase, viewang);

	VectorCopy(ent->enemy->s.pos.trBase, enemyorg);
	enemyorg[2] += 32;
	VectorSubtract(enemyorg, ent->s.pos.trBase, dir);
	VectorNormalize(dir);
	vectoangles(dir, dir);
//	VectorCopy( dir, ent->s.apos.trBase );

	turret_trackenemyslow( viewang, dir, ent->s.apos.trBase );

	//trap_SendServerCommand( -1, va("print \"^3TRACKING TARGET\n\"") );
}

void turret_idle( gentity_t *ent ) {
	vec3_t idleAngles;
	vec3_t viewang;

	VectorCopy(ent->s.apos.trBase, viewang);

	idleAngles[PITCH] = ent->backorigin[PITCH];
	idleAngles[YAW] = ent->backorigin[YAW];
	idleAngles[ROLL] = ent->backorigin[ROLL];

	idleAngles[YAW] += (sin( ((float)level.time /3000.f) * M_PI ) * 60.f);

	turret_trackenemyslow( viewang, idleAngles, ent->s.apos.trBase );

}

void turret_fireonenemy( gentity_t *ent){
	vec3_t	forward, right, up;
	vec3_t	muzzle;
	
	if (!checktarget(ent,ent->enemy)) {
		return;
	}

	AngleVectors( ent->s.apos.trBase, forward, right, up );
	VectorCopy( ent->s.origin, muzzle );
	//muzzle[2] += other->client->ps.viewheight;
	VectorMA( muzzle, 58, forward, muzzle );
	SnapVector( muzzle );

	//fire_lead( ent, ent->s.origin, ent->s.origin2, 5 );
	Fire_Lead_Ext( ent, ent->parent->parent, MG42_SPREAD_MP, 5, muzzle, forward, right, up, MOD_BROWNING );
	G_AddEvent( ent, EV_FIRE_WEAPON, 0 );

	//trap_SendServerCommand( -1, va("print \"^1SHOOTING TARGET\n\"") );

	ent->count=level.time+TURRET_RATEOFFIRE;
}


void turret_think( gentity_t *ent ) {

	ent->nextthink=level.time+20;

	if( !checkent(ent->parent->parent) )
		return;

	//turret_correctorg(ent);

	if( !checktarget(ent,ent->enemy) ) {
		turret_findenemy(ent);
	}


	// changes angles
	if( !ent->enemy ) {
		turret_idle(ent);
		return;
	}

	turret_trackenemy(ent);

	if( ent->count<level.time ) {
		turret_fireonenemy(ent);
	}

}


void mg42_touch (gentity_t *self, gentity_t *other, trace_t *trace);
void mg42_think (gentity_t *self);
void mg42_use (gentity_t *ent, gentity_t *other, gentity_t *activator );

/*
void CreateTurretGun( gentity_t *ent ){
	gentity_t *turret = NULL; 	// The object to hold the turrets details.

//	ent->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
//	ent->r.contents = CONTENTS_SOLID;
//	ent->s.pos.trType = TR_STATIONARY;

	turret=G_Spawn();
	turret->parent=ent->parent;
	turret->tagParent=ent;
	turret->eventTime=level.time; // makes the firing sequence go away after 200 milliseconds.
	turret->s.weapon = WP_MOBILE_MG42; // which weapon will be fired (graphics only)
	turret->classname="turret";
	//turret->s.modelindex =	G_ModelIndex("models/multiplayer/browning/thirdperson.md3");
	//turret->model =						("models/multiplayer/browning/thirdperson.md3");
	//turret->s.modelindex2 = G_ModelIndex("models/multiplayer/browning/thirdperson.md3");
	turret->s.modelindex =	G_ModelIndex("models/browning_thirdperson.md3");
	turret->model =						("models/browning_thirdperson.md3");
	turret->s.modelindex2 = G_ModelIndex("models/browning_thirdperson.md3");

	
	// scale is stored in 'angles2'
	//turret->s.eType = ET_GAMEMODEL;
	turret->s.angles2[0] = 1.5f;
	turret->s.angles2[1] = 1.5f;
	turret->s.angles2[2] = 1.5f;
	VectorSet( turret->r.mins, -24, -24, -8 );
	VectorSet( turret->r.maxs, 24, 24, 48);
	turret->clipmask =			CONTENTS_SOLID;
	turret->r.contents =		CONTENTS_TRIGGER;
	turret->r.svFlags =		0;
	turret->s.eType =			ET_MG42_BARREL;
	turret->mg42BaseEnt = ent->s.number;
	ent->chain = turret;

		turret->touch =			mg42_touch;
        turret->think =			mg42_think;
		turret->use =			mg42_use;
		//turret->die =			mg42_die;

	//	turret->think=turret_think; // what the turret does

	turret->harc =				57.5;
	turret->varc =				45.0;
	turret->s.origin2[0] =		57.5;
	turret->s.origin2[1] =		45.0;
	turret->damage = 10;
	turret->accuracy = 10.0;
	turret->health = 300;
	turret->mg42weapHeat	 =		0;

	turret->nextthink=level.time+1000; // when the turret will activate
	turret->timestamp = level.time + 1000;
	turret->s.number = turret - g_entities;
	G_SetEntState( turret, ent->entstate );

	SetTurretGun( turret, ent, qfalse );
	turret->s.apos.trType = TR_LINEAR_STOP;
	VectorCopy (turret->s.angles, turret->s.angles2);


	trap_LinkEntity (turret); // adds the finalized turret.

}*/



void turret_die (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod) {
	gentity_t	*gun;
	gentity_t	*owner;
	trace_t		tr;
		
	// DHM - Nerve :: self->chain not set if no tripod
	if( self->chain ) {
		gun = self->chain;
	} else {
		gun = self;
	}

	gun->takedamage = qfalse;
	gun->health = 0;

	self->takedamage = qfalse;
	self->health = 0;

	owner = &g_entities[gun->r.ownerNum];

	if( owner && owner->client ) {
		// Restore original position if current position is bad
		trap_Trace (&tr, owner->r.currentOrigin, owner->r.mins, owner->r.maxs, owner->r.currentOrigin, owner->s.number, MASK_PLAYERSOLID);
		if( tr.startsolid ) {
			VectorCopy( owner->TargetAngles, owner->client->ps.origin );
			VectorCopy( owner->TargetAngles, owner->r.currentOrigin );
			owner->r.contents = CONTENTS_CORPSE;			// this will correct itself in ClientEndFrame
		}
		owner->client->ps.eFlags &= ~EF_MG42_ACTIVE;			// DHM - Nerve :: unset flag

		owner->client->ps.persistant[PERS_HWEAPON_USE] = 0;
		owner->active = qfalse;

		owner->client->ps.viewlocked = 0;	// let them look around

		//self->r.ownerNum = self->s.number;
		//self->s.otherEntityNum = self->s.number;
		gun->mg42weapHeat = 0;
		gun->backupWeaponTime = 0;

		owner->client->ps.weaponTime = owner->backupWeaponTime;

		self->active = qfalse;
		gun->active = qfalse;
	}

	//trap_LinkEntity (self);

	//if( self->chain ) {
		//G_FreeEntity( gun );
	//}
	G_FreeEntity( self );
}

void Cmd_SpawnTurret_f( gentity_t *ent ) {
	gentity_t	*base = NULL;
	gentity_t	*turret = NULL;
	//gentity_t	*testBox = NULL;
	qboolean	turretremoved = qfalse;
	qboolean	baseremoved = qfalse;
	int health = 300;
	int damage = 10;
	float accuracy = 10.0;

	if(! checkent(ent) )
		return;

	if( !GetTurretSpawnLoc( ent, NULL, NULL ) ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Turret: INVALID LOCATION!\n\""));
		return;
	}

// +++++++++++++
	while( (turret = G_Find(turret, FOFS(classname), "misc_aagun")) != NULL ) { // turret
		if( turret->parent ) {
			base = turret->parent;
			if( base->parent == ent ) {
				G_FreeEntity( turret );
				turretremoved = qtrue;
				G_FreeEntity( base );
				baseremoved = qtrue;
				break;
			}
		}
	}


	if( turretremoved && baseremoved ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Old Turret Removed!\n\""));
	} else if( turretremoved || baseremoved ) {
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR removing Old Turret!\n\""));
		return;
	}


	base = NULL;
	turret = NULL;
	health = 300;
	damage = 10;
	accuracy = 10.0;
// +++++++++++++

	base=G_Spawn();
	base->classname="turretbase"; // misc_mg42base
	base->parent = ent;

	base->s.modelindex =	G_ModelIndex("models/mapobjects/weapons/mg42b.md3");
	// scale is stored in 'angles2'
	//base->s.angles2[0] = base->s.angles2[1] = base->s.angles2[2] = 1.0f;

	base->clipmask = CONTENTS_SOLID;
	base->r.contents = CONTENTS_SOLID;
	base->r.svFlags = 0;
	base->s.eType = ET_GENERAL;
	base->takedamage = qtrue;
	base->die = turret_die;

	VectorSet( base->r.mins, -16, -16, 2 );
	VectorSet( base->r.maxs, 16, 16, 48);

	//G_SetOrigin( base, ent->s.pos.trBase );
	//G_SetAngle( base, ent->s.apos.trBase );

	base->sound3to2 = -1;
	base->s.dmgFlags = HINT_MG42;
	base->health = health;

	//base->think=CreateTurretGun;
	//base->nextthink=level.time+100;

	trap_LinkEntity (base);

	SetTurretBase( base, qfalse );
	G_SetEntState( base, ent->entstate );

// ========================================================================

	turret=G_Spawn();
	turret->classname="misc_aagun"; // turret
	turret->parent=base; // base->parent

	//turret->eventTime=level.time; // makes the firing sequence go away after 200 milliseconds.
	turret->s.weapon = WP_MOBILE_MG42; // which weapon will be fired (graphics only)

	turret->clipmask =			CONTENTS_SOLID;
	turret->r.contents =		CONTENTS_TRIGGER;
	turret->r.svFlags =			0;
	turret->s.eType =			ET_MG42_BARREL;
	turret->health =			base->health; // JPW NERVE
	turret->sound3to2 =			-1;

	//turret->s.modelindex = G_ModelIndex ("models/multiplayer/mg42/mg42.md3");
	//turret->s.modelindex = G_ModelIndex("models/multiplayer/browning/thirdperson.md3");
	turret->s.modelindex =	G_ModelIndex("models/browning_thirdperson.md3");
	// scale is stored in effect3Time
	turret->s.effect3Time = (1.3 * 100);

	VectorSet( turret->r.mins, -24, -24, -8 );
	VectorSet( turret->r.maxs, 24, 24, 48);

	//G_SetAngle( turret, ent->s.apos.trBase );
	SetTurretGun( turret, base, qfalse );
	turret->s.apos.trTime =		0;
	turret->s.apos.trDuration =	0;
	turret->s.apos.trType =		TR_LINEAR_STOP;
	VectorCopy (turret->s.angles, turret->s.angles2);
	

	if( g_turrets.integer > 1 ) {
		turret->think=  turret_think; // what the turret does
		turret->die =  turret_die;
	} else {
		turret->touch =			mg42_touch;
        turret->think =			mg42_think;
		turret->use =			mg42_use;
		turret->die =			turret_die;
	}

	turret->nextthink=level.time+1000; // when the turret will activate
	turret->timestamp = level.time + 1000;
	turret->s.number = turret - g_entities;

	turret->harc =				57.5;
	turret->varc =				45.0;
	turret->s.origin2[0] =		57.5;
	turret->s.origin2[1] =		45.0;

	turret->takedamage =	qtrue;
	turret->damage =		damage;
	turret->accuracy =		accuracy;
	turret->mg42weapHeat =	0;

	G_SetEntState( turret, ent->entstate );

	turret->mg42BaseEnt = base->s.number;
	base->chain = turret;

	trap_LinkEntity (turret); // adds the finalized turret.
	
}

// -------------------------------------------

void testturretbase( gentity_t *ent ) {

	if( !ent->parent->AASblocking ) {
		return;
	}

	ent->nextthink=level.time+20;

	if(! checkent(ent->parent) )
		return;

	SetTurretBase( ent, qtrue );
}


void testturretgun( gentity_t *ent ) {

	if( !ent->parent->AASblocking ) {
		return;
	}

	ent->nextthink=level.time+20;

	if(! checkent(ent->parent) )
		return;

	SetTurretGun( ent, NULL, qtrue );
}

void Cmd_TestTurret_f( gentity_t *ent ) {
	gentity_t	*base = NULL;
	gentity_t	*turret = NULL;
//	vec3_t		origin;
//	vec3_t		angles;

	if( !ent->AASblocking ) {
		ent->AASblocking = qtrue;
	} else {
		ent->AASblocking = qfalse;
		// +++++++++++++
			while( (turret = G_Find(turret, FOFS(classname), "turrettest")) != NULL ) {
				if( turret->parent == ent ) {
					G_FreeEntity( turret );
					while( (base = G_Find(base, FOFS(classname), "turretbasetest")) != NULL ) {
						if( base->parent == ent ) {
							G_FreeEntity( base );
							trap_SendServerCommand( ent-g_entities, va("print \"Test Turret Removed!\n\""));
							break;
						}
					}
					break;
				}
			}
		// +++++++++++++
		return;
	}

	if(! checkent(ent) )
		return;

// ================================================

	base=G_Spawn();
	base->classname="turretbasetest";
	base->parent=ent;
	base->s.modelindex =	G_ModelIndex("models/mapobjects/weapons/mg42b.md3");
	base->model =						("models/mapobjects/weapons/mg42b.md3");
	base->s.modelindex2 =	G_ModelIndex("models/mapobjects/weapons/mg42b.md3");
	// scale is stored in 'angles2'
	//base->s.eType = ET_GAMEMODEL;
	//base->s.angles2[0] = base->s.angles2[1] = base->s.angles2[2] = 1.0f;
	base->think=testturretbase;
	base->nextthink=level.time+100;

	G_SetOrigin( base, ent->s.pos.trBase );
	G_SetAngle( base, ent->s.apos.trBase );

	trap_LinkEntity (base);

	SetTurretBase( base, qtrue );


// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	turret=G_Spawn();
	turret->parent=ent;
	turret->tagParent=base;
	turret->classname="turrettest";
	//turret->s.modelindex =	G_ModelIndex("models/multiplayer/browning/thirdperson.md3");
	//turret->model =						("models/multiplayer/browning/thirdperson.md3");
	//turret->s.modelindex2 = G_ModelIndex("models/multiplayer/browning/thirdperson.md3");
	turret->s.modelindex =	G_ModelIndex("models/browning_thirdperson.md3");
	turret->model =						("models/browning_thirdperson.md3");
	turret->s.modelindex2 = G_ModelIndex("models/browning_thirdperson.md3");

	// scale is stored in 'angles2'
	turret->s.eType = ET_GAMEMODEL;
	turret->s.angles2[0] = 1.5f;
	turret->s.angles2[1] = 1.5f;
	turret->s.angles2[2] = 1.5f;

	turret->think=testturretgun; // what the turret does
	turret->nextthink=level.time+100; // when the turret will activate

	//G_SetOrigin( turret, ent->s.pos.trBase );
	//G_SetAngle( turret, ent->s.apos.trBase );
	SetTurretGun( turret, base, qtrue );

	trap_LinkEntity (turret); // adds the finalized turret.

}

#endif
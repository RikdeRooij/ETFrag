#include "cg_local.h"

static mapEntityData_t mapEntities[MAX_GENTITIES];
static int mapEntityCount = 0;
static int mapEntityTime = 0;
static qboolean expanded = qfalse;

extern playerInfo_t pi;

qboolean ccInitial = qtrue;

void CG_TransformToCommandMapCoord( float *coord_x, float *coord_y ) {
	*coord_x = CC_2D_X + ((*coord_x - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * CC_2D_W;
	*coord_y = CC_2D_Y + ((*coord_y - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * CC_2D_H;
}

// START	xkan, 9/19/2002
//static float automapZoom = 3.583;	// apporoximately 1.2^7
//static float automapZoom = 5.159;

int CG_CurLayerForZ( int z ) {
	int curlayer = 0;

	while( z > cgs.ccLayerCeils[curlayer] && curlayer < cgs.ccLayers )
		curlayer++;

	if( curlayer == cgs.ccLayers ) {
		CG_Printf( "^3Warning: no valid command map layer for z\n" );
		curlayer = 0;
	}

	return curlayer;
}

static qboolean CG_ScissorEntIsCulled( mapEntityData_t* mEnt, mapScissor_t *scissor ) {
	int passpad = 16;

	if( !scissor->circular ) {
		if( mEnt->automapTransformed[0] <	 (scissor->tl[0] - passpad)
			|| mEnt->automapTransformed[0] > (scissor->br[0] + passpad)
			|| mEnt->automapTransformed[1] < (scissor->tl[1] - passpad)
			|| mEnt->automapTransformed[1] > (scissor->br[1] + passpad)
			)
			return qtrue;
	} else {
		float distSquared;
		vec2_t distVec;

		distVec[0] = mEnt->automapTransformed[0] - ( scissor->tl[0] + ( 0.5f * ( scissor->br[0] - scissor->tl[0] ) ) );
		distVec[1] = mEnt->automapTransformed[1] - ( scissor->tl[1] + ( 0.5f * ( scissor->br[1] - scissor->tl[1] ) ) );
		distSquared = distVec[0]*distVec[0] + distVec[1]*distVec[1];

		if( (distSquared - passpad) > Square( 0.5f * (scissor->br[0] - scissor->tl[0]) ) )
			return qtrue;
	}

	return qfalse;
}

static qboolean CG_ScissorPointIsCulled( vec2_t vec, mapScissor_t *scissor ) {
	if( !scissor->circular ) {
		if( vec[0] < scissor->tl[0]
			|| vec[0] > scissor->br[0]
			|| vec[1] < scissor->tl[1]
			|| vec[1] > scissor->br[1] )
			return qtrue;
	} else {
		float distSquared;
		vec2_t distVec;

		distVec[0] = vec[0] - ( scissor->tl[0] + ( 0.5f * ( scissor->br[0] - scissor->tl[0] ) ) );
		distVec[1] = vec[1] - ( scissor->tl[1] + ( 0.5f * ( scissor->br[1] - scissor->tl[1] ) ) );
		distSquared = distVec[0]*distVec[0] + distVec[1]*distVec[1];

		if( distSquared > Square( 0.5f * (scissor->br[0] - scissor->tl[0]) ) )
			return qtrue;
	}

	return qfalse;
}

/*
=====================================================================================
CG_TransformAutomapEntity: calculate the scaled (zoomed) yet unshifted coordinate for
each map entity within the automap
=====================================================================================
*/
void CG_TransformAutomapEntity( void )
{
	int i;

	// kw: fixes icons not being drawn on the right spot
	//     on the compass for non-standard compass sizes.
	float size = cg.hud.compass[2] - ((cg.hud.compass[2] * 32.f) / 132.f);

	for (i=0; i<mapEntityCount; i++) {
		mapEntityData_t* mEnt = &mapEntities[i];

		// calculate the screen coordinate of this entity for the automap, consider the zoom value
		mEnt->automapTransformed[0] = (mEnt->x - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0] * size * cg_mapzoom.value;
		mEnt->automapTransformed[1] = (mEnt->y - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1] * size * cg_mapzoom.value;
	}
}

void CG_AdjustAutomapZoom(int zoomIn)
{
	float mapZoom = cg_mapzoom.value;
	if (zoomIn) {
		mapZoom *= 1.2;
		if (mapZoom > 7.43)  // approximately 1.2^11
			mapZoom = 7.43;
	} else {
		mapZoom /= 1.2;
		// zoom value of 1 corresponds to the most zoomed out view. The whole map is displayed 
		// in the automap
		if ( mapZoom < 1)
			mapZoom = 1;
	}
	trap_Cvar_Set("cg_mapzoom",va("%f",mapZoom));
	// recalculate the screen coordinates since the zoom changed
	CG_TransformAutomapEntity();
}
// END		xkan, 9/19/2002

void CG_ParseMapEntity( int* mapEntityCount, int* offset, team_t team ) {
	mapEntityData_t* mEnt = &mapEntities[(*mapEntityCount)];
	char buffer[16];

	trap_Argv((*offset)++, buffer, 16);
	mEnt->type = atoi(buffer);

	switch( mEnt->type ) {
		case ME_CONSTRUCT: // Gordon: these ones don't need much info
		case ME_DESTRUCT:
		case ME_DESTRUCT_2:
		case ME_COMMANDMAP_MARKER:
			break;

		case ME_TANK:
		case ME_TANK_DEAD:
			trap_Argv((*offset)++, buffer, 16);
			mEnt->x = atoi(buffer) * 128;

			trap_Argv((*offset)++, buffer, 16);
			mEnt->y = atoi(buffer) * 128;

			if( cgs.ccLayers ) {
				trap_Argv((*offset)++, buffer, 16);
				mEnt->z = atoi(buffer) * 128;
			}
			break;

		default:
			trap_Argv((*offset)++, buffer, 16);
			mEnt->x = atoi(buffer) * 128;

			trap_Argv((*offset)++, buffer, 16);
			mEnt->y = atoi(buffer) * 128;

			if( cgs.ccLayers ) {
				trap_Argv((*offset)++, buffer, 16);
				mEnt->z = atoi(buffer) * 128;
			}

			trap_Argv((*offset)++, buffer, 16);
			mEnt->yaw = atoi(buffer);
			break;
	}

	trap_Argv((*offset)++, buffer, 16);
	mEnt->data = atoi(buffer);

	mEnt->transformed[0] = (mEnt->x - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0] * CC_2D_W;
	mEnt->transformed[1] = (mEnt->y - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1] * CC_2D_H;

	mEnt->team = team;

	(*mapEntityCount)++;
}

/*
=======================
CG_ParseMapEntityInfo
=======================
*/
void CG_ParseMapEntityInfo( int axis_number, int allied_number ) {
	int i, offset;

	mapEntityCount = 0;
	mapEntityTime = cg.time;

	offset = 3;

	for( i = 0; i < axis_number; i++ ) {
		CG_ParseMapEntity( &mapEntityCount, &offset, TEAM_AXIS );
	}

	for(i = 0; i < allied_number; i++) {
		CG_ParseMapEntity( &mapEntityCount, &offset, TEAM_ALLIES );
	}

	CG_TransformAutomapEntity();
}

static qboolean gridInitDone = qfalse;
static vec2_t gridStartCoord, gridStep;

static void CG_DrawGrid( float x, float y, float w, float h, mapScissor_t *scissor ) {
	vec2_t step;
	vec2_t dim_x, dim_y;
	vec4_t line;
	float xscale, yscale;
	float grid_x, grid_y;
	vec2_t dist;
	vec4_t gridColour;

	dist[0] = cg.mapcoordsMaxs[0] - cg.mapcoordsMins[0];
	dist[1] = cg.mapcoordsMaxs[1] - cg.mapcoordsMins[1];

	if( !gridInitDone ) {
		gridStep[0] = 1200.f;
		gridStep[1] = 1200.f;

		// ensure minimal grid density
		while( ( cg.mapcoordsMaxs[0] - cg.mapcoordsMins[0] ) / gridStep[0] < 7 )
			gridStep[0] -= 50.f;
		while( ( cg.mapcoordsMins[1] - cg.mapcoordsMaxs[1] ) / gridStep[1] < 7 )
			gridStep[1] -= 50.f;

		gridStartCoord[0] = .5f * ( ( ( ( cg.mapcoordsMaxs[0] - cg.mapcoordsMins[0] ) / gridStep[0] ) - ( (int)(( cg.mapcoordsMaxs[0] - cg.mapcoordsMins[0] ) / gridStep[0] ) ) ) * gridStep[0] );
		gridStartCoord[1] = .5f * ( ( ( ( cg.mapcoordsMins[1] - cg.mapcoordsMaxs[1] ) / gridStep[1] ) - ( (int)(( cg.mapcoordsMins[1] - cg.mapcoordsMaxs[1] ) / gridStep[1] ) ) ) * gridStep[1] );

		gridInitDone = qtrue;
	}

	if( scissor ) {
	//if( scissor ) {
		dim_x[0] = cg.mapcoordsMins[0];
		dim_x[1] = cg.mapcoordsMaxs[0];

		dim_y[0] = cg.mapcoordsMaxs[1];
		dim_y[1] = cg.mapcoordsMins[1];

		// transform
		xscale = ( w * scissor->zoomFactor ) / dist[0];
		yscale = ( h * scissor->zoomFactor ) / -dist[1];

		dim_x[0] = (dim_x[0] - cg.mapcoordsMins[0]) * xscale;
		dim_x[1] = (dim_x[1] - cg.mapcoordsMins[0]) * xscale;

		dim_y[0] = (dim_y[0] - cg.mapcoordsMaxs[1]) * yscale;
		dim_y[1] = (dim_y[1] - cg.mapcoordsMaxs[1]) * yscale;

		grid_x = ((gridStartCoord[0] / dist[0]) * w * scissor->zoomFactor) - scissor->tl[0];
		grid_y = ((-gridStartCoord[1] / dist[1]) * h * scissor->zoomFactor) - scissor->tl[1];

		step[0] = gridStep[0] * xscale;
		step[1] = gridStep[1] * yscale;

		// draw
		Vector4Set( gridColour, clrBrownLine[0], clrBrownLine[1], clrBrownLine[2], .4f );
		trap_R_SetColor( gridColour );
		for( ; grid_x < dim_x[1]; grid_x += step[0] )
		{
			if( grid_x < dim_x[0] )
				continue;

			if( grid_x > w )
				break;

			if( scissor->circular ) {
				// clip line against circle
				float xc, yc;

				line[0] = x + grid_x;
				xc = line[0] >= x + .5f * w ? line[0] - (x + .5f * w) : (x + .5f * w) - line[0];
				yc = SQRTFAST( Square(.5f * w) - Square(xc) );
				line[1] = y + (.5f * h) - yc;
				line[2] = 1.f;
				line[3] = 2 * yc;
			} else {
				Vector4Set( line, x + grid_x, y + dim_y[0], 1.f, h );
			}
			line[0] *= cgs.screenXScale;
			line[1] *= cgs.screenYScale;
			line[3] *= cgs.screenYScale;
			trap_R_DrawStretchPic( line[0], line[1], line[2], line[3], 0, 0, 0, 1, cgs.media.whiteShader );
		}

		for( ; grid_y < dim_y[1]; grid_y += step[1] )
		{
			if( grid_y < dim_y[0] )
				continue;

			if( grid_y > h )
				break;

			if( scissor->circular ) {
				// clip line against circle
				float xc, yc;

				line[1] = y + grid_y;
				yc = line[1] >= y + .5f * h ? line[1] - (y + .5f * h) : (y + .5f * h) - line[1];
				xc = SQRTFAST( Square(.5f * h) - Square(yc) );
				line[0] = x + (.5f * w) - xc;
				line[2] = 2 * xc;
				line[3] = 1.f;
			} else {
				Vector4Set( line, x + dim_x[0], y + grid_y, w, 1 );
			}			
			line[0] *= cgs.screenXScale;
			line[1] *= cgs.screenYScale;
			line[2] *= cgs.screenXScale;
			trap_R_DrawStretchPic( line[0], line[1], line[2], line[3], 0, 0, 0, 1, cgs.media.whiteShader );
		}
		trap_R_SetColor( NULL );
	} else {
		char coord_char[3], coord_int;
		float text_width, text_height;
		vec2_t textOrigin;

		dim_x[0] = cg.mapcoordsMins[0];
		dim_x[1] = cg.mapcoordsMaxs[0];

		dim_y[0] = cg.mapcoordsMaxs[1];
		dim_y[1] = cg.mapcoordsMins[1];

		// transform
		xscale = w / dist[0];
		yscale = h / -dist[1];

		dim_x[0] = (dim_x[0] - cg.mapcoordsMins[0]) * xscale;
		dim_x[1] = (dim_x[1] - cg.mapcoordsMins[0]) * xscale;

		dim_y[0] = (dim_y[0] - cg.mapcoordsMaxs[1]) * yscale;
		dim_y[1] = (dim_y[1] - cg.mapcoordsMaxs[1]) * yscale;

		grid_x = gridStartCoord[0] * xscale;
		grid_y = gridStartCoord[1] * yscale;

		step[0] = gridStep[0] * xscale;
		step[1] = gridStep[1] * yscale;

		// draw
		textOrigin[0] = grid_x;
		textOrigin[1] = grid_y;

		Vector4Set( gridColour, clrBrownLine[0], clrBrownLine[1], clrBrownLine[2], 1.f );

		coord_char[1] = '\0';
		for( coord_char[0] = ('A' - 1); grid_x < dim_x[1]; grid_x += step[0], coord_char[0]++ )
		{
			if( coord_char[0] >= 'A' ) {
				text_width = CG_Text_Width_Ext( coord_char, 0.2f, 0, &cgs.media.font3 );
				text_height = CG_Text_Height_Ext( coord_char, 0.2f, 0, &cgs.media.font3 );
				CG_Text_Paint_Ext( ( x + grid_x ) - (.5f * step[0]) - (.5f * text_width), y + dim_y[0] + textOrigin[1] + 1.5f * text_height, 0.2f, 0.2f, colorBlack, coord_char, 0, 0, 0, &cgs.media.font3 );
			}
			trap_R_SetColor( gridColour );

			Vector4Set( line, x + grid_x, y + dim_y[0], 1, dim_x[1] - dim_x[0] );
			line[0] *= cgs.screenXScale;
			line[1] *= cgs.screenYScale;
			line[3] *= cgs.screenYScale;
			trap_R_DrawStretchPic( line[0], line[1], line[2], line[3], 0, 0, 0, 1, cgs.media.whiteShader );
		}

		for( coord_int = -1; grid_y < dim_y[1]; grid_y += step[1], coord_int++ )
		{
			if( coord_int >= 0 ) {
				Com_sprintf( coord_char, sizeof(coord_char), "%i", coord_int );
				text_width = CG_Text_Width_Ext( "0", 0.2f, 0, &cgs.media.font3 );
				text_height = CG_Text_Height_Ext( coord_char, 0.2f, 0, &cgs.media.font3 );
				CG_Text_Paint_Ext( x + dim_x[0] + textOrigin[0] + .5f * text_width, (y + grid_y) - (.5f * step[1]) + (.5f * text_height), 0.2f, 0.2f, colorBlack, coord_char, 0, 0, 0, &cgs.media.font3 );
			}
			trap_R_SetColor( gridColour );

			Vector4Set( line, x + dim_x[0], y + grid_y, dim_y[1] - dim_y[0], 1 );
			line[0] *= cgs.screenXScale;
			line[1] *= cgs.screenYScale;
			line[2] *= cgs.screenXScale;
			trap_R_DrawStretchPic( line[0], line[1], line[2], line[3], 0, 0, 0, 1, cgs.media.whiteShader );
		}
		trap_R_SetColor( NULL );
	}
}


#define COMMANDMAP_PLAYER_ICON_SIZE 6
#define AUTOMAP_PLAYER_ICON_SIZE 5

// xkan: extracted from CG_DrawCommandMap.
// drawingCommandMap - qfalse: command map; qtrue: auto map (upper left in main game view)

void CG_DrawMapEntity( mapEntityData_t *mEnt, float x, float y, float w, float h, int mEntFilter, mapScissor_t *scissor, qboolean interactive, snapshot_t* snap, int icon_size ) {
	int j = 1;
	qhandle_t pic;
	clientInfo_t* ci;
	bg_playerclass_t* classInfo;
	centity_t *cent;
	const char* name;
	vec4_t c_clr = {1.f, 1.f, 1.f, 1.f};
	vec2_t icon_extends, icon_pos, string_pos;
	int customimage = 0;
	oidInfo_t* oidInfo = NULL;

	memset( &string_pos, 0, sizeof( string_pos ) );

	switch( mEnt->type ) {
	case ME_PLAYER_DISGUISED:
	case ME_PLAYER_REVIVE:
	case ME_PLAYER:
		ci = &cgs.clientinfo[mEnt->data];
		if(!ci->infoValid) {
			return;
		}

		// kw: if your in limbo don't draw your playericon.
		if( snap->ps.pm_flags & PMF_LIMBO && mEnt->data == cg.clientNum) {
			return;
		}

		if(ci->team == TEAM_AXIS) {
			if(mEntFilter & CC_FILTER_AXIS) {
				return;
			}
		} else if(ci->team == TEAM_ALLIES) {
			if(mEntFilter & CC_FILTER_ALLIES) {
				return;
			}
		} else {
			return;
		}

		cent = &cg_entities[mEnt->data];

		if( mEnt->type == ME_PLAYER_DISGUISED && !(cent->currentState.powerups & (1 << PW_OPS_DISGUISED)) ) {
			return;
		}

		classInfo = CG_PlayerClassForClientinfo(ci, cent);

		// For these, if availaible, ignore the coordinate data and grab the most up to date pvs data
		if( cent-cg_entities == cg.snap->ps.clientNum ) {
			if( !scissor ) {
				mEnt->transformed[0] = ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
				mEnt->transformed[1] = ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
			} else {
				mEnt->automapTransformed[0] = ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
				mEnt->automapTransformed[1] = ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
			}
			
			mEnt->yaw = cg.predictedPlayerState.viewangles[YAW];
		} else if( ci->team == snap->ps.persistant[PERS_TEAM] && cent->currentValid ) {
			if( !scissor ) {
				mEnt->transformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
				mEnt->transformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
			} else {
				mEnt->automapTransformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
				mEnt->automapTransformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
			}

			mEnt->yaw = cent->lerpAngles[YAW];
		} else {
			// Gordon: only see revivables for own team, duh :)
			if(mEnt->type == ME_PLAYER_REVIVE) {
				return;
			}
		}


		// now check to see if the entity is within our clip region
		if( scissor && CG_ScissorEntIsCulled( mEnt, scissor ) ) {
			return;
		}

		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( mEnt->z ) != cgs.ccSelectedLayer )
                return;
		}

		if( scissor ) {
			// CHRUKER: b085 - Compass icons were offset when zooming in or out
			icon_pos[0] = mEnt->automapTransformed[0] - scissor->tl[0] + x - (icon_size * (scissor->zoomFactor / 5.159));
			icon_pos[1] = mEnt->automapTransformed[1] - scissor->tl[1] + y - (icon_size * (scissor->zoomFactor / 5.159));
		} else { 
			icon_pos[0] = x + mEnt->transformed[0] - icon_size;
			icon_pos[1] = y + mEnt->transformed[1] - icon_size;
			string_pos[0] = x + mEnt->transformed[0];
			string_pos[1] = y + mEnt->transformed[1] + icon_size;
		}

		icon_extends[0] = 2 * icon_size;
		icon_extends[1] = 2 * icon_size;
		if( scissor ) {
			icon_extends[0] *= (scissor->zoomFactor / 5.159);
			icon_extends[1] *= (scissor->zoomFactor / 5.159);
		}

		if( mEnt->type == ME_PLAYER_REVIVE ) {
			float msec;
			vec4_t reviveClr = { 1.f, 1.f, 1.f, 1.f };

			if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS) {
				msec = (cg_redlimbotime.integer - (cg.time%cg_redlimbotime.integer)) / (float)cg_redlimbotime.integer;
			} else if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_ALLIES) {
				msec = (cg_bluelimbotime.integer - (cg.time%cg_bluelimbotime.integer)) / (float)cg_bluelimbotime.integer;
			} else {
				msec = 0;
			}

			reviveClr[3] = .5f + .5f * ((sin(sqrt(msec) * 25 * 2 * M_PI) + 1) * .5f);

			trap_R_SetColor( reviveClr );
			CG_DrawPic( icon_pos[0] + 3, icon_pos[1] + 3, icon_extends[0] - 3, icon_extends[1] - 3, cgs.media.medicIcon);
			trap_R_SetColor( NULL );
		} else {
			if( cg.clientNum == mEnt->data ) {
				if( ci->ccSelected ) {
					trap_R_SetColor( colorRed );
				} else {
					trap_R_SetColor( colorYellow );
				}

				CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.ccPlayerHighlight);
				trap_R_SetColor( NULL );

				if(cg.predictedPlayerEntity.voiceChatSpriteTime > cg.time) {
					// kw: voicechat icon position now depends on zoom and compass size
					CG_DrawPic( icon_pos[0] + icon_extends[0], icon_pos[1], icon_extends[0] * 0.5f, icon_extends[1] * 0.5f,  cg.predictedPlayerEntity.voiceChatSprite);
				}
			} else if( mEnt->type == ME_PLAYER_DISGUISED ) {
				trap_R_SetColor( colorOrange );
				CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.ccPlayerHighlight);
				trap_R_SetColor( NULL );
			} else if( /*!(cgs.ccFilter & CC_FILTER_BUDDIES) &&*/ CG_IsOnSameFireteam( cg.clientNum, mEnt->data )) {
				if( ci->ccSelected ) {
					trap_R_SetColor( colorRed );
				}

				CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.ccPlayerHighlight);
				trap_R_SetColor( NULL );
			
				if( !scissor ) {
					CG_Text_Paint_Ext( string_pos[0], string_pos[1], 0.2f, 0.2f, colorWhite, ci->name, 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
//					CG_DrawStringExt_Shadow( string_pos[0], string_pos[1], ci->name, colorWhite, qfalse, 1, 8, 12, 0 );
				}

				if(cent->voiceChatSpriteTime > cg.time) {
					CG_DrawPic( icon_pos[0] + icon_extends[0], icon_pos[1], icon_extends[0] * 0.5f, icon_extends[1] * 0.5f, cent->voiceChatSprite);
				}
			} else if( ci->team == snap->ps.persistant[PERS_TEAM] ) {
				if( ci->ccSelected ) {
					trap_R_SetColor( colorRed );
					CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.ccPlayerHighlight);
					trap_R_SetColor( NULL );
				}

				if(cent->voiceChatSpriteTime > cg.time) {
					CG_DrawPic( icon_pos[0] + icon_extends[0], icon_pos[1], icon_extends[0] * 0.5f, icon_extends[1] * 0.5f,  cent->voiceChatSprite);
				}
			}

			c_clr[3] = 1.0f;

			trap_R_SetColor(c_clr);
			CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], classInfo->icon);

			if(ci->team == snap->ps.persistant[PERS_TEAM] ) {
				if( 
					cent->currentState.powerups & ( 1 << PW_REDFLAG  ) ||
					cent->currentState.powerups & ( 1 << PW_BLUEFLAG )
				) {
					CG_DrawPic( 
						icon_pos[0], 
						icon_pos[1], 
						icon_extends[0], 
						icon_extends[1], 
						cgs.media.objectiveShader
					);
				}

				if(cent->currentState.powerups & ( 1 << PW_OPS_DISGUISED ) ) {
					CG_DrawPic( 
						icon_pos[0], 
						icon_pos[1], 
						icon_extends[0], 
						icon_extends[1], 
						cgs.media.friendShader
					);
				}
			}

			CG_DrawRotatedPic( 
				icon_pos[0]-1, 
				icon_pos[1]-1, 
				icon_extends[0]+2, 
				icon_extends[1]+2, 
				classInfo->arrow, 
				(0.5 - ( mEnt->yaw - 180.f ) / 360.f)
			);	
			trap_R_SetColor(NULL);
		}
		return;
	case ME_CONSTRUCT:
	case ME_DESTRUCT:
	case ME_DESTRUCT_2:
	case ME_TANK:
	case ME_TANK_DEAD:
	case ME_COMMANDMAP_MARKER:
		cent = NULL;
		if( mEnt->type == ME_TANK || mEnt->type == ME_TANK_DEAD ) {
			oidInfo = &cgs.oidInfo[ mEnt->data ];

			for( j = 0; j < cg.snap->numEntities; j++ ) {
				if( cg.snap->entities[j].eType == ET_OID_TRIGGER && cg.snap->entities[j].teamNum == mEnt->data ) {
					cent = &cg_entities[cg.snap->entities[j].number];
					if( !scissor ) {
						mEnt->transformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
						mEnt->transformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
					} else {
						mEnt->automapTransformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
						mEnt->automapTransformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
					}
					break;
				}
			}
		} else if( mEnt->type == ME_CONSTRUCT || mEnt->type == ME_DESTRUCT || mEnt->type == ME_DESTRUCT_2 ) {
			cent = &cg_entities[mEnt->data];

			oidInfo = &cgs.oidInfo[ cent->currentState.modelindex2 ];

			if( !scissor ) {
				mEnt->transformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
				mEnt->transformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
			} else {
				mEnt->automapTransformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
				mEnt->automapTransformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
			}
		} else if( mEnt->type == ME_COMMANDMAP_MARKER ) {
			oidInfo = &cgs.oidInfo[ mEnt->data ];

			if( !scissor ) {
				mEnt->transformed[0] = ((oidInfo->origin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
				mEnt->transformed[1] = ((oidInfo->origin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
			} else {
				mEnt->automapTransformed[0] = ((oidInfo->origin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
				mEnt->automapTransformed[1] = ((oidInfo->origin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
			}
		}

		// now check to see if the entity is within our clip region
		if( scissor && CG_ScissorEntIsCulled( mEnt, scissor ) ) {
			return;
		}

		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( mEnt->z ) != cgs.ccSelectedLayer )
                return;
		}

		if( oidInfo ) {
			customimage = mEnt->team == TEAM_AXIS ? oidInfo->customimageaxis : oidInfo->customimageallies;
		}

/*		if((mEnt->yaw & 0xFF) & (1 << (atoi(CG_ConfigString(mEnt->team == TEAM_AXIS ? CS_MAIN_AXIS_OBJECTIVE : CS_MAIN_ALLIES_OBJECTIVE)) - 1))) {
			trap_R_SetColor( colorYellow );
		}*/

		if(mEnt->type == ME_CONSTRUCT) {
			if(mEntFilter & CC_FILTER_CONSTRUCTIONS) {
				return;
			}
			pic = mEnt->team == TEAM_AXIS ? cgs.media.ccConstructIcon[0] : cgs.media.ccConstructIcon[1];
		} else if(mEnt->type == ME_TANK) {
			if(mEntFilter & CC_FILTER_OBJECTIVES) {
				return;
			}
			pic = cgs.media.ccTankIcon;
		} else if(mEnt->type == ME_TANK_DEAD) {
			if(mEntFilter & CC_FILTER_OBJECTIVES) {
				return;
			}
			pic = cgs.media.ccTankIcon;
			trap_R_SetColor( colorRed );
		} else if( mEnt->type == ME_COMMANDMAP_MARKER ) {
			pic = 0;
		} else if( mEnt->type == ME_DESTRUCT_2 ) {
			pic = 0;
		} else {
			if( mEntFilter & CC_FILTER_DESTRUCTIONS ) {
				return;
			}
			pic = mEnt->team == TEAM_AXIS ? cgs.media.ccDestructIcon[cent->currentState.effect1Time][0] : cgs.media.ccDestructIcon[cent->currentState.effect1Time][1];
		}

		{
			int info = 0;

			if( oidInfo ) {
				info = oidInfo->spawnflags;
			}

			if( info & (1 << 4) ) {
				if( mEntFilter & CC_FILTER_OBJECTIVES ) {
					return;
				}
			}
			if( info & (1 << 5) ) {
				if( mEntFilter & CC_FILTER_HACABINETS ) {
					return;
				}
			}
			if( info & (1 << 6) ) {
				if( mEnt->type == ME_DESTRUCT_2 ) {
					pic = mEnt->team == TEAM_AXIS ? cgs.media.ccCmdPost[0] : cgs.media.ccCmdPost[1];
				}
				if( mEntFilter & CC_FILTER_CMDPOST ) {
					return;
				}
			}
		}

		if( customimage ) {
			pic = customimage;
		}

		if( scissor ) {
			icon_pos[0] = mEnt->automapTransformed[0] - scissor->tl[0] + x;
			icon_pos[1] = mEnt->automapTransformed[1] - scissor->tl[1] + y;
		} else {
			icon_pos[0] = x + mEnt->transformed[0];
			icon_pos[1] = y + mEnt->transformed[1];
		}

#define CONST_ICON_NORMAL_SIZE 32.f
#define CONST_ICON_EXPANDED_SIZE 48.f

		if( interactive && !expanded && BG_RectContainsPoint( x + mEnt->transformed[0] - (CONST_ICON_NORMAL_SIZE*0.5f), y + mEnt->transformed[1] - (CONST_ICON_NORMAL_SIZE*0.5f), CONST_ICON_NORMAL_SIZE, CONST_ICON_NORMAL_SIZE, cgDC.cursorx, cgDC.cursory )) {
			float w;

			icon_extends[0] = CONST_ICON_EXPANDED_SIZE;
			icon_extends[1] = CONST_ICON_EXPANDED_SIZE;
			if( mEnt->type == ME_TANK_DEAD || mEnt->type == ME_TANK ) {
				icon_extends[1] *= 0.5f;
			}
			if( scissor ) {
				icon_extends[0] *= (scissor->zoomFactor / 5.159);
				icon_extends[1] *= (scissor->zoomFactor / 5.159);
			} else {
				icon_extends[0] *= cgs.ccZoomFactor;
				icon_extends[1] *= cgs.ccZoomFactor;
			}

			CG_DrawPic( icon_pos[0] - (icon_extends[0]*0.5f), icon_pos[1] - (icon_extends[1]*0.5f), icon_extends[0], icon_extends[1], pic );

			if( oidInfo ) {
				name = oidInfo->name;
			} else {
				name = va( "%i", j );
			}

			w = CG_Text_Width_Ext( name, 0.2f, 0, &cgs.media.font3 );
			CG_CommandMap_SetHighlightText( name, icon_pos[0] - ( w * 0.5f ), icon_pos[1] - 8 );
		} else if ( interactive && (mEnt->yaw & 0xFF) & (1 << cgs.ccSelectedObjective) ) {
			float scalesize;
			int time = cg.time % 1400;

			if(time <= 700) {
				scalesize = 12 * (time) / 700.f;
			} else {
				scalesize = 12 * (1 - ((time - 700) / 700.f));
			}

			icon_extends[0] = CONST_ICON_NORMAL_SIZE + scalesize;
			icon_extends[1] = CONST_ICON_NORMAL_SIZE + scalesize;
			if( scissor ) {
				icon_extends[0] *= (scissor->zoomFactor / 5.159);
				icon_extends[1] *= (scissor->zoomFactor / 5.159);
			} else {
				icon_extends[0] *= cgs.ccZoomFactor;
				icon_extends[1] *= cgs.ccZoomFactor;
			}
			if( mEnt->type == ME_TANK_DEAD || mEnt->type == ME_TANK ) {
				icon_extends[1] *= 0.5f;
			}

			CG_DrawPic( icon_pos[0] - (icon_extends[0]*0.5f), icon_pos[1] - (icon_extends[1]*0.5f), icon_extends[0], icon_extends[1], pic );
		} else {
			icon_extends[0] = CONST_ICON_NORMAL_SIZE;
			icon_extends[1] = CONST_ICON_NORMAL_SIZE;
			if( mEnt->type == ME_TANK_DEAD || mEnt->type == ME_TANK ) {
				icon_extends[1] *= 0.5f;
			}
			if( scissor ) {
				icon_extends[0] *= (scissor->zoomFactor / 5.159);
				icon_extends[1] *= (scissor->zoomFactor / 5.159);
			} else {
				icon_extends[0] *= cgs.ccZoomFactor;
				icon_extends[1] *= cgs.ccZoomFactor;
			}

			CG_DrawPic( icon_pos[0] - (icon_extends[0]*0.5f), icon_pos[1] - (icon_extends[1]*0.5f), icon_extends[0], icon_extends[1], pic );
		}
		trap_R_SetColor( NULL );
		return;
	case ME_LANDMINE:
/*			if(mEntFilter & CC_FILTER_LANDMINES) {
			continue;
		}*/
		// kw: fixed landmines placed incorrectly on non-standard compass sizes
		if( scissor) {
			mEnt->automapTransformed[0] = (mEnt->x - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0] * w * scissor->zoomFactor;
			mEnt->automapTransformed[1] = (mEnt->y - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1] * h * scissor->zoomFactor;
		}


		// now check to see if the entity is within our clip region
		if( scissor && CG_ScissorEntIsCulled( mEnt, scissor ) ) {
			return;
		}

		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( mEnt->z ) != cgs.ccSelectedLayer )
                return;
		}

		if( mEnt->data == TEAM_AXIS ) {
			pic = cgs.media.commandCentreAxisMineShader;
		} else { // TEAM_ALLIES
			pic = cgs.media.commandCentreAlliedMineShader;
		}			

		c_clr[3] = 1.0f;

		if( scissor ) {
			icon_pos[0] = mEnt->automapTransformed[0] - scissor->tl[0] + x;
			icon_pos[1] = mEnt->automapTransformed[1] - scissor->tl[1] + y;
		} else {
			icon_pos[0] = x + mEnt->transformed[0];
			icon_pos[1] = y + mEnt->transformed[1];
		}

		icon_extends[0] = 12;
		icon_extends[1] = 12;
		if( scissor ) {
			icon_extends[0] *= (scissor->zoomFactor / 5.159);
			icon_extends[1] *= (scissor->zoomFactor / 5.159);
		} else {
			icon_extends[0] *= cgs.ccZoomFactor;
			icon_extends[1] *= cgs.ccZoomFactor;
		}

		trap_R_SetColor(c_clr);
		CG_DrawPic( icon_pos[0] - icon_extends[0]*0.5f, icon_pos[1] - icon_extends[1]*0.5f, icon_extends[0], icon_extends[1], pic);
		trap_R_SetColor(NULL);

		j++;
		return;
	default:
		return;
	}
}


void CG_DrawSquareMapBar( float x, float y, float w, float h )
{
	// codePRK square compass
	float angle;
	static float lastangle = 0;
	static float anglespeed = 0;
	float diff;
	int test;
	float xmod;
	int smoothness = 1000; // is smoothness a word?
	vec4_t redtrans = { 1.0f, 0.0f, 0.0f, 0.6f };

	CG_AdjustFrom640( &x, &y, &w, &h );
// ------------------------------
	angle = ( cg.predictedPlayerState.viewangles[YAW] + 180.f ) / 360.f - ( 0.125f );	
	diff = AngleSubtract( angle * 360, lastangle * 360 ) / 360.f;
	anglespeed /= 1.08f;
	anglespeed += diff * 0.01f;
	if( Q_fabs(anglespeed) < 0.00001f ) {
		anglespeed = 0;
	}
	lastangle += anglespeed;
// ------------------------------

	test = ((int)(lastangle * smoothness) % smoothness);
	if( test < 0 ) {
		test += smoothness;
	}
	// test is now 0 to 1 * smoothness
	test = smoothness - test; // invert
	test += (0.5 * smoothness); // result will flip

	// line them right
	test += (0.04 * smoothness);

	xmod = ( x - (float)test * (256.f / smoothness) ); // 0 to 256
	//y += 6;

	trap_R_DrawStretchPic( xmod,		y, 256, 16, 0, 0, 1, 1, cgs.media.compass3 );
	trap_R_DrawStretchPic( xmod + 256,	y, 256, 16, 0, 0, 1, 1, cgs.media.compass3 );

	//CG_Text_Paint_Ext( basex + 4 - (float)test * 2.56f, 200, 0.3f, 0.3f, colorRed, "|", 0, 0, 0, &cgs.media.limboFont2 );

	trap_R_SetColor( redtrans );
	trap_R_DrawRotatedPic( x + w/2 - 6, y + 10, 12, 12, 0, 0, 1, 1, cgs.media.ccMortarTargetArrow, (45.f+270.f)/360.f );
	trap_R_SetColor( NULL );
}

void CG_DrawMap( float x, float y, float w, float h, int mEntFilter, mapScissor_t *scissor, qboolean interactive, float alpha, qboolean borderblend ) {
	int i/*, j = 1*/;
	snapshot_t* snap;
//	vec4_t c_clr = {1.f, 1.f, 1.f, 1.f};
	mapEntityData_t* mEnt = &mapEntities[0];
	int icon_size;
	int exspawn;

	expanded = qfalse;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

//	x -= 4;
//	y -= 4;
//	w += 8;
//	h += 8;
	if( cg_drawCompass.integer >= 2 ) {
		// yea it gets bigger, so you wont have to
		// change the location when switching between round and square
		// (square takes less room cos of no border)
		x -= 4;
		y -= 4;
		w += 8;
		h += 8;
	}

	if ( scissor ) {
		float s0, s1, t0, t1;

		icon_size = AUTOMAP_PLAYER_ICON_SIZE;

		if( cgs.hardcore ) {
			return;
		}

		if ( scissor->br[0] >= scissor->tl[0] ) {
			float sc_x, sc_y, sc_w, sc_h;

			sc_x = x;
			sc_y = y;
			sc_w = w;
			sc_h = h;

			if( cg_drawCompass.integer >= 2 ) { // codePRK square compass
				CG_DrawPic( sc_x, sc_y, sc_w, sc_h, cgs.media.SquareAutomapMaskShader );
			} else {
				CG_DrawPic( sc_x, sc_y, sc_w, sc_h, cgs.media.commandCentreAutomapMaskShader );
			}

			s0 = (scissor->tl[0])/(w*scissor->zoomFactor);
			s1 = (scissor->br[0])/(w*scissor->zoomFactor);
			t0 = (scissor->tl[1])/(h*scissor->zoomFactor);
			t1 = (scissor->br[1])/(h*scissor->zoomFactor);

			if( cg_drawCompass.integer >= 2 ) { // codePRK square compass
				CG_FillRect( x, y, w, h, colorBlack );
			}

			CG_AdjustFrom640( &sc_x, &sc_y, &sc_w, &sc_h );
			if( cgs.ccLayers ) {
				trap_R_DrawStretchPic( sc_x, sc_y, sc_w, sc_h, s0, t0, s1, t1, cgs.media.commandCentreAutomapShader[cgs.ccSelectedLayer] );
			} else {
				trap_R_DrawStretchPic( sc_x, sc_y, sc_w, sc_h, s0, t0, s1, t1, cgs.media.commandCentreAutomapShader[0] );
			}
			trap_R_DrawStretchPic( 0, 0, 0, 0, 0, 0, 0, 0, cgs.media.whiteShader ); // HACK : the code above seems to do weird things to 
																					// the next trap_R_DrawStretchPic issued. This works
																					// around this.

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			if( cg_drawCompass.integer == 3 ) {
				CG_DrawPic( x+1, y+1, w-2, h-2, cgs.media.compassColorTest_oran );
			} else
			if( cg_drawCompass.integer == 4 ) {
				CG_DrawPic( x+1, y+1, w-2, h-2, cgs.media.compassColorTest_blue );
			}
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

		}

		// Draw the grid
		CG_DrawGrid( x, y, w, h, scissor );
	} else {
		icon_size = COMMANDMAP_PLAYER_ICON_SIZE;

		{
			vec4_t color;
			Vector4Set( color, 1.f, 1.f, 1.f, alpha );
			trap_R_SetColor( color );

			// codePRK http://www.splashdamage.com/forums/showthread.php?t=12320
			CG_DrawPic( x, y, w, h, cgs.media.blackmask );

			if( cgs.ccLayers ) {
				CG_DrawPic(x, y, w, h, cgs.media.commandCentreMapShaderTrans[cgs.ccSelectedLayer] );
			} else {
				CG_DrawPic(x, y, w, h, cgs.media.commandCentreMapShaderTrans[0] );
			}
			trap_R_SetColor( NULL );
		}

		// Draw the grid
		CG_DrawGrid( x, y, w, h, NULL );
	}

	if( borderblend ) {
		vec4_t clr = { 0.f, 0.f, 0.f, 0.75f };
		trap_R_SetColor( clr );
		CG_DrawPic( x, y, w, h, cgs.media.limboBlendThingy );
		trap_R_SetColor( NULL );
	}

	exspawn = CG_DrawSpawnPointInfo( x, y, w, h, qfalse, scissor, -1 );

	if( !cgs.hardcore ) {
		for(i = 0, mEnt = &mapEntities[0]; i < mapEntityCount; i++, mEnt++ ) {
			// kw: spectators can see icons of both teams
			if( (interactive && mEnt->team != CG_LimboPanel_GetRealTeam()) ||
				(mEnt->team != snap->ps.persistant[PERS_TEAM] &&
					snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) ) {
				continue;
			}

			if( mEnt->type == ME_PLAYER ||
				mEnt->type == ME_PLAYER_DISGUISED ||
				mEnt->type == ME_PLAYER_REVIVE )
				continue;

			CG_DrawMapEntity( mEnt, x, y, w, h, mEntFilter, scissor, interactive, snap, icon_size );
		}
	}


	CG_DrawSpawnPointInfo( x, y, w, h, qtrue, scissor, exspawn );

	CG_DrawMortarMarker( x, y, w, h, qtrue, scissor, exspawn );

	if( !cgs.hardcore ) {
		for(i = 0, mEnt = &mapEntities[0]; i < mapEntityCount; i++, mEnt++ ) {
			if( mEnt->team != CG_LimboPanel_GetRealTeam() ) {
				continue;
			}

			if( mEnt->type != ME_PLAYER &&
				mEnt->type != ME_PLAYER_DISGUISED &&
				mEnt->type != ME_PLAYER_REVIVE )
				continue;

			CG_DrawMapEntity( mEnt, x, y, w, h, mEntFilter, scissor, interactive, snap, icon_size );
		}
	}

	// kw: draw spectator position and direction
	if( cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR ) {
		vec2_t pos, size;
		bg_playerclass_t* classInfo;

		if( snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
			size[0] = size[1] = 16;
		} else {
			size[0] = size[1] = 12;
		}

		if( scissor ) {
			size[0] *= (scissor->zoomFactor / 5.159);
			size[1] *= (scissor->zoomFactor / 5.159);
		} else {
			size[0] *= cgs.ccZoomFactor;
			size[1] *= cgs.ccZoomFactor;
		}
		if( scissor ) {
			pos[0] = ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor - scissor->tl[0] + x;
			pos[1] = ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor - scissor->tl[1] + y;
		} else {
			pos[0] = x + ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
			pos[1] = y + ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
		}

		if( snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
			// kw: draw a arrow when free-spectating.
			// FIXME: don't reuse the ccMortarTargetArrow
			CG_DrawRotatedPic( pos[0]- (size[0]*0.5f), pos[1]- (size[1]*0.5f), size[0], size[1], cgs.media.ccMortarTargetArrow, ( 0.625 - ( cg.predictedPlayerState.viewangles[YAW] - 180.f ) / 360.f));
		} else {
			//PLAYER ICON

			// kw: FIXME: make a severside change so it also sends player compass info
			//	when spectating and remove this hack.

			classInfo = CG_PlayerClassForClientinfo(&cgs.clientinfo[snap->ps.clientNum], &cg_entities[snap->ps.clientNum]);
			CG_DrawPic( pos[0]- (size[0]*0.5f), pos[1] - (size[1]*0.5f), size[0], size[1], classInfo->icon);
			CG_DrawRotatedPic( pos[0] -(size[0]*0.5f) - 1, pos[1]- (size[1]*0.5f) - 1, size[0]+2, size[1]+2, classInfo->arrow, (0.5 - ( cg.predictedPlayerState.viewangles[YAW] - 180.f ) / 360.f));
		}
	}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	if( cg_drawCompass.integer >= 2 ) {
		if( cg_drawCompass.integer == 5 ) {
			CG_DrawPic( x+1, y+1, w-2, h-2, cgs.media.compassColorTest_oran );
		} else
		if( cg_drawCompass.integer == 6 ) {
			CG_DrawPic( x+1, y+1, w-2, h-2, cgs.media.compassColorTest_blue );
		}


		// asumes AdjustFrom640
		CG_DrawSquareMapBar( x, y, w, h );

		CG_DrawRect( x, y, w, h, 1, colorDkGrey );
	}
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
}

void CG_DrawExpandedAutoMap( void ) {
	float x, y, w, h;
	float b_x, b_y, b_w, b_h;
	float s1, t1, s2, t2;
//	vec4_t colour = { 1.f, 1.f, 1.f, .5f };

	x = SCREEN_WIDTH + 10.f;
	y = 20.f;

	w = CC_2D_W;
	h = CC_2D_H;


	if( cgs.autoMapExpanded ) {	
		if( cg.time - cgs.autoMapExpandTime < 250.f ) {
			x -= ( ( cg.time - cgs.autoMapExpandTime ) / 250.f ) * ( w + 30.f );
		} else {
			x = SCREEN_WIDTH - w - 20.f;
		}
	} else {
		if( cg.time - cgs.autoMapExpandTime < 250.f ) {
			x = ( SCREEN_WIDTH - w - 20.f ) + ( ( cg.time - cgs.autoMapExpandTime ) / 250.f ) * ( w + 30.f );
		} else {
			return;
		}
	}

	CG_DrawMap( x, y, w, h, cgs.ccFilter, NULL, qfalse, .7f, qfalse );

	// Draw the border

	// top left
	s1 = 0;
	t1 = 0;
	s2 = 1;
	t2 = 1;
	b_x = x - 8;
	b_y = y - 8;
	b_w = 8;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapCornerShader );

	// top
	s2 = w / 256.f;
	b_x = x;
	b_y = y - 8;
	b_w = w;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapBorderShader );

	// top right
	s1 = 1;
	t1 = 0;
	s2 = 0;
	t2 = 1;
	b_x = x + w;
	b_y = y - 8;
	b_w = 8;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapCornerShader );

	// right
	s1 = 1;
	t1 = h / 256.f;
	s2 = 0;
	t2 = 0;
	b_x = x + w;
	b_y = y;
	b_w = 8;
	b_h = h;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapBorder2Shader );

	// bottom right
	s1 = 1;
	t1 = 1;
	s2 = 0;
	t2 = 0;
	b_x = x + w;
	b_y = y + h;
	b_w = 8;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapCornerShader );

	// bottom
	s1 = w / 256.f;
	b_x = x;
	b_y = y + h;
	b_w = w;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapBorderShader );

	// bottom left
	s1 = 0;
	t1 = 1;
	s2 = 1;
	t2 = 0;
	b_x = x - 8;
	b_y = y + h;
	b_w = 8;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapCornerShader );

	// left
	s1 = 0;
	t1 = 0;
	s2 = 1;
	t2 = h / 256.f;
	b_x = x - 8;
	b_y = y;
	b_w = 8;
	b_h = h;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapBorder2Shader );
}

void CG_DrawAutoMap( void ) {
	float x, y, w, h;
	int scaleVal;
	mapScissor_t mapScissor;
	vec2_t automapTransformed;

	memset( &mapScissor, 0, sizeof(mapScissor) );

	if( cgs.ccLayers ) {
		cgs.ccSelectedLayer = CG_CurLayerForZ( (int)cg.predictedPlayerEntity.lerpOrigin[2] );
	}

	scaleVal = ((cg.hud.compass[2] * 32.f) / 132.f);
	x = cg.hud.compass[0] + (scaleVal / 2);
	y = cg.hud.compass[1] + (scaleVal / 2);
	w = cg.hud.compass[2] - scaleVal; // 100 normally (compass[2] = 132)
	h = cg.hud.compass[2] - scaleVal; // 100 normally

	if( cgs.autoMapExpanded ) {
		if( x < (SCREEN_WIDTH - CC_2D_W - 20.f) ) {
			// no need to hide
			CG_DrawExpandedAutoMap();
		} else if( y < 16 ) {
			// go up
			if( cg.time - cgs.autoMapExpandTime < 100.f ) {
				y -= ( ( cg.time - cgs.autoMapExpandTime ) / 100.f ) * 128.f;
				CG_DrawExpandedAutoMap();
			} else {
				CG_DrawExpandedAutoMap();
				return;
			}
		} else if ( y > (480 - w - 16) ) {
			// go down
			if( cg.time - cgs.autoMapExpandTime < 100.f ) {
				y += ( ( cg.time - cgs.autoMapExpandTime ) / 100.f ) * 128.f;
				CG_DrawExpandedAutoMap();
			} else {
				CG_DrawExpandedAutoMap();
				return;
			}
		} else {
			// just hide
			CG_DrawExpandedAutoMap();
			return;
		}
	} else {
		if( x < (SCREEN_WIDTH - CC_2D_W - 20.f) ) {
			// no need to hide
			// allow expended map to go away
			if( cg.time - cgs.autoMapExpandTime <= 250.f ) {
				CG_DrawExpandedAutoMap();
			}
		} else if( y < 16 ) {
			// from up back down
			if( cg.time - cgs.autoMapExpandTime <= 150.f ) {
				CG_DrawExpandedAutoMap();
				return;
			} else if( ( cg.time - cgs.autoMapExpandTime > 150.f ) && ( cg.time - cgs.autoMapExpandTime < 250.f ) ) {
				y = ( y - 128.f ) + ( ( cg.time - cgs.autoMapExpandTime - 150.f ) / 100.f ) * 128.f;
				CG_DrawExpandedAutoMap();
			}
		} else if ( y > (480 - w - 16) ) {
			// from down back up
			if( cg.time - cgs.autoMapExpandTime <= 150.f ) {
				CG_DrawExpandedAutoMap();
				return;
			} else if( ( cg.time - cgs.autoMapExpandTime > 150.f ) && ( cg.time - cgs.autoMapExpandTime < 250.f ) ) {
				y = ( y + 128.f ) - ( ( cg.time - cgs.autoMapExpandTime - 150.f ) / 100.f ) * 128.f;
				CG_DrawExpandedAutoMap();
			}
		} else {
			// wait a bit
			if( cg.time - cgs.autoMapExpandTime <= 250.f ) {
				CG_DrawExpandedAutoMap();
				return;
			}
		}
	}

	// codePRK square compass
	if( cg_drawCompass.integer < 2 ) {
		mapScissor.circular = qtrue;
	} else {
		mapScissor.circular = qfalse;
	}

	mapScissor.zoomFactor = cg_mapzoom.value;

	mapScissor.tl[0] = mapScissor.tl[1] = 0;
	mapScissor.br[0] = mapScissor.br[1] = -1;

	automapTransformed[0] = ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * mapScissor.zoomFactor;
	automapTransformed[1] = ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * mapScissor.zoomFactor;

	// update clip region (for next drawing). clip region has a size kAutomap_width x kAutomap_height
	// and it is after zooming is accounted for.
	//
	// first try to center the clip region around the player. then make sure the region
	// stays within the world map.
	mapScissor.tl[0] = automapTransformed[0] - w/2;
	if( mapScissor.tl[0] < 0 )
		mapScissor.tl[0] = 0;
	mapScissor.br[0] = mapScissor.tl[0] + w;
	if( mapScissor.br[0] > (w * mapScissor.zoomFactor) ) {
		mapScissor.br[0] = w * mapScissor.zoomFactor;
		mapScissor.tl[0] = mapScissor.br[0] - w;
	}

	mapScissor.tl[1] = automapTransformed[1] - h/2;
	if( mapScissor.tl[1] < 0 )
		mapScissor.tl[1] = 0;
	mapScissor.br[1] = mapScissor.tl[1] + h;
	if ( mapScissor.br[1] > (h * mapScissor.zoomFactor)) {
		mapScissor.br[1] = h * mapScissor.zoomFactor;
		mapScissor.tl[1] = mapScissor.br[1] - h;
	}

	CG_DrawMap( x, y, w, h, cgs.ccFilter, &mapScissor, qfalse, 1.f, qfalse );
}

/*void CG_DrawWaypointInfo( int x, int y, int w, int h ) {
	snapshot_t	*snap;
	vec2_t		pos;
	int			i;

	if( cgs.ccFilter & CC_FILTER_WAYPOINTS ) {
		return;
	}

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	for ( i = 0; i < snap->numEntities; i++ ) {
		entityState_t *ent = &snap->entities[i];

		if ( ent->eType != ET_WAYPOINT )
			continue;

		// see if the waypoint owner is someone that you accept waypoints from
		if(!CG_IsOnSameFireteam( cg.clientNum, ent->clientNum )) {
			continue;
		}

		pos[0] = x + ((ent->pos.trBase[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
		pos[1] = y + ((ent->pos.trBase[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;

		switch( ent->frame ) {
			case WAYP_ATTACK:	CG_DrawPic( pos[0] - 6, pos[1] - 6, 12, 12, cgs.media.waypointAttackShader ); break;
			case WAYP_DEFEND:	CG_DrawPic( pos[0] - 6, pos[1] - 6, 12, 12, cgs.media.waypointDefendShader ); break;
			case WAYP_REGROUP:	CG_DrawPic( pos[0] - 6, pos[1] - 6, 12, 12, cgs.media.waypointRegroupShader ); break;
		}
	}
}*/

int CG_DrawSpawnPointInfo( int px, int py, int pw, int ph, qboolean draw, mapScissor_t *scissor, int expand ) {
	int i;
	char buffer[64];
	vec2_t point;
	int e = -1;
	vec2_t icon_extends;

	team_t team = CG_LimboPanel_GetRealTeam();

	if( cgs.ccFilter & CC_FILTER_SPAWNS ) {
		return -1;
	}

#define FLAGSIZE_EXPANDED 48.f
#define FLAGSIZE_NORMAL 32.f
#define FLAG_LEFTFRAC (25/128.f)
#define FLAG_TOPFRAC (95/128.f)
#define SPAWN_SIZEUPTIME 1000.f
	for ( i = 1; i < cg.spawnCount; i++ ) {
		float changetime = 0;
		if(cg.spawnTeams_changeTime[i]) {
			changetime = (cg.time - cg.spawnTeams_changeTime[i]);
			if(changetime > SPAWN_SIZEUPTIME || changetime < 0) {
				changetime = cg.spawnTeams_changeTime[i] = 0;
			}
		}

		// rain - added parens around ambiguity
		if (((cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR) &&
			(cg.spawnTeams[i] != team)) ||
			((cg.spawnTeams[i] & 256) && !changetime)) {

			continue;
		}

		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( (int)cg.spawnCoords[i][2] ) != cgs.ccSelectedLayer )
				break;
		}

		if( scissor ) {
			point[0] = ((cg.spawnCoordsUntransformed[i][0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw * scissor->zoomFactor;
			point[1] = ((cg.spawnCoordsUntransformed[i][1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph * scissor->zoomFactor;
		} else {
			point[0] = px + (((cg.spawnCoordsUntransformed[i][0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw );
			point[1] = py + (((cg.spawnCoordsUntransformed[i][1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph );
		}

		if( scissor && CG_ScissorPointIsCulled( point, scissor ) ) {
			continue;
		}

		if( scissor ) {
			point[0] += px - scissor->tl[0];
			point[1] += py - scissor->tl[1];
		}

		icon_extends[0] = FLAGSIZE_NORMAL;
		icon_extends[1] = FLAGSIZE_NORMAL;
		if( scissor ) {
			icon_extends[0] *= (scissor->zoomFactor / 5.159);
			icon_extends[1] *= (scissor->zoomFactor / 5.159);
		} else {
			icon_extends[0] *= cgs.ccZoomFactor;
			icon_extends[1] *= cgs.ccZoomFactor;
		}

		point[0] -= (icon_extends[0] * (39 / 128.f));
		point[1] += (icon_extends[1] * (31 / 128.f));

		if( changetime ) {
			if( draw ) {
				float size;

				if(cg.spawnTeams[i] == team) {
					size = 20 * (changetime / SPAWN_SIZEUPTIME);
				} else {
					size = 20 * (1 - (changetime / SPAWN_SIZEUPTIME));
				}

				if( scissor ) {
					size *= (scissor->zoomFactor / 5.159);
				} else {
					size *= cgs.ccZoomFactor;
				}

				CG_DrawPic( point[0] - FLAG_LEFTFRAC*size, point[1] - FLAG_TOPFRAC*size, size, size, cgs.media.commandCentreSpawnShader[  cg.spawnTeams[i] == TEAM_AXIS ? 0 : 1 ] );
			}
		} else if( (draw && i == expand) || (!expanded && BG_RectContainsPoint( point[0] - FLAGSIZE_NORMAL*0.5f, point[1] - FLAGSIZE_NORMAL*0.5f, FLAGSIZE_NORMAL, FLAGSIZE_NORMAL, cgDC.cursorx, cgDC.cursory )) ) {
			if( draw ) {
				float size = FLAGSIZE_EXPANDED;
				if( scissor ) {
					size *= (scissor->zoomFactor / 5.159);
				} else {
					size *= cgs.ccZoomFactor;
				}

				CG_DrawPic( point[0] - FLAG_LEFTFRAC*size, point[1] - FLAG_TOPFRAC*size, size, size, cgs.media.commandCentreSpawnShader[  cg.spawnTeams[i] == TEAM_AXIS ? 0 : 1 ] );
			} else {			
				if( !scissor ) {
					float w;

					Com_sprintf( buffer, sizeof( buffer ), "%s (Troops: %i)", cg.spawnPoints[i], cg.spawnPlayerCounts[i] );
					w = CG_Text_Width_Ext( buffer, 0.2f, 0, &cgs.media.font3 );
					CG_CommandMap_SetHighlightText( buffer, point[0] - ( w * 0.5f ), point[1] - 8 );
				}

				e = i;
			}
		} else {
			if( draw ) {
				float size = FLAGSIZE_NORMAL;
				if( scissor ) {
					size *= (scissor->zoomFactor / 5.159);
				} else {
					size *= cgs.ccZoomFactor;
				}

				CG_DrawPic( point[0] - FLAG_LEFTFRAC*size, point[1] - FLAG_TOPFRAC*size, size, size, cgs.media.commandCentreSpawnShader[  cg.spawnTeams[i] == TEAM_AXIS ? 0 : 1 ] );
				
				if( !scissor ) {
					float w;

					Com_sprintf( buffer, sizeof( buffer ), "(Troops: %i)", cg.spawnPlayerCounts[i] );
					// kw: fixed automap text getting drawn outside the automap and/or screen
					w = CG_Text_Width_Ext( buffer, 0.2f, 0, &cgs.media.font3 );
					if( point[0] + FLAGSIZE_NORMAL*0.25f + w > px + pw)
						CG_Text_Paint_Ext( px + pw - w, point[1], 0.2f, 0.2f, colorWhite, buffer, 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
					else
						CG_Text_Paint_Ext( point[0] + FLAGSIZE_NORMAL*0.25f, point[1], 0.2f, 0.2f, colorWhite, buffer, 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
				}
			}
		}
	}

	return e;
}

void CG_DrawMortarMarker( int px, int py, int pw, int ph, qboolean draw, mapScissor_t *scissor, int expand ) {
	if( cg.lastFiredWeapon == WP_MORTAR_SET && cg.mortarImpactTime >= 0 ) {
		if( cg.snap->ps.weapon != WP_MORTAR_SET ) {
			cg.mortarImpactTime = 0;
		} else {

			//vec4_t colour = { .77f, .1f, .1f, 1.f };
			vec4_t colour = { 1.f, 1.f, 1.f, 1.f };
			vec3_t point;
//			int fadeTime = 0; // cg.time - (cg.predictedPlayerEntity.muzzleFlashTime + 5000);

			if( scissor ) {
				point[0] = ((cg.mortarImpactPos[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw * scissor->zoomFactor;
				point[1] = ((cg.mortarImpactPos[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph * scissor->zoomFactor;
			} else {
				point[0] = px + (((cg.mortarImpactPos[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw );
				point[1] = py + (((cg.mortarImpactPos[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph );
			}

			// rain - don't return if the marker is culled, just don't
			// draw it.
			if (!(scissor && CG_ScissorPointIsCulled( point, scissor ))) {
				if( scissor ) {
					point[0] += px - scissor->tl[0];
					point[1] += py - scissor->tl[1];
				}

				if( cg.mortarImpactOutOfMap ) {
					if( !scissor ) {
						// near the edge of the map, fit into it
						if( point[0] + 8.f > (px + pw) )
							point[0] -= 8.f;
						else if( point[0] - 8.f < px )
							point[0] += 8.f;

						if( point[1] + 8.f > (py + ph) )
							point[1] -= 8.f;
						else if( point[1] - 8.f < py )
							point[1] += 8.f;
					}
					colour[3] = .5f;
				}

				trap_R_SetColor( colour );
				CG_DrawRotatedPic( point[0] - 8.f, point[1] - 8.f, 16, 16, cgs.media.ccMortarHit, .5f - ( cg.mortarFireAngles[YAW] /*- 180.f */+ 45.f ) / 360.f );
				trap_R_SetColor( NULL );
			}
		}
	}

	if( COM_BitCheck( cg.snap->ps.weapons, WP_MORTAR_SET ) ) {
		int i;

		for( i = 0; i < MAX_CLIENTS; i++ ) {
			//vec4_t colour = { .23f, 1.f, .23f, 1.f };
			vec4_t colour = { 1.f, 1.f, 1.f, 1.f };
			vec3_t point;
			int fadeTime = cg.time - (cg.artilleryRequestTime[i] + 25000);

			if( fadeTime < 5000 ) {
				if( fadeTime > 0 ) {
					colour[3] = 1.f - (fadeTime/5000.f);
				}

				if( scissor ) {
					point[0] = ((cg.artilleryRequestPos[i][0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw * scissor->zoomFactor;
					point[1] = ((cg.artilleryRequestPos[i][1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph * scissor->zoomFactor;
				} else {
					point[0] = px + (((cg.artilleryRequestPos[i][0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw );
					point[1] = py + (((cg.artilleryRequestPos[i][1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph );
				}

				// rain - don't return if the marker is culled, just skip
				// it (so we draw the rest, if any)
				if( scissor && CG_ScissorPointIsCulled( point, scissor ) ) {
					continue;
				}

				if( scissor ) {
					point[0] += px - scissor->tl[0];
					point[1] += py - scissor->tl[1];
				}

				trap_R_SetColor( colour );
				CG_DrawPic( point[0] - 8.f, point[1] - 8.f, 16, 16, cgs.media.ccMortarTarget );
				trap_R_SetColor( NULL );

				//CG_FillRect( point[0] - 8.f, point[1] - 8.f, 16, 16, colour );
			}
		}
	}
}

/*void CG_DrawCommandCentreLayers( void ) {
	int x, y;
	int i;
	char *s;

	if( !cgs.ccLayers )
		return;

	x = CC_2D_X + CC_2D_W - 32; 
	y = CC_2D_Y + CC_2D_H - 32;

	for( i = 0; i < cgs.ccLayers; i++ ) {
		if( i == cgs.ccSelectedLayer )
			CG_FillRect( x, y, 32, 32, clrBrownTextLt ); 
		else
			CG_FillRect( x, y, 32, 32, clrBrownText ); 
		CG_DrawRect( x, y, 32, 32, 1, colorBlack );

		if( i == 0 )
			s = "G";
		else
			s =  va( "%i", i );

		CG_Text_Paint( x + 1 + ( ( 32 - CG_Text_Width( s, .5f, 0 ) ) * .5f ),
					   y + 16 +( ( 32 - CG_Text_Height( s, .5f, 0 ) ) *.5f ),
					   .5f, clrBrownTextDk, s, 0, 0, 0 );
		y -= 34;
	}
}*/

mapEntityData_t* CG_ScanForCommandCentreEntity( void ) {
	vec_t	rangeSquared = Square(1000);
	int		ent = 0;
	mapEntityData_t* mEnt = &mapEntities[0];
	int i;

	if( mapEntityCount <= 0 ) {
		return NULL;
	}

	for(i = 0; i < mapEntityCount; i++, mEnt++ ) {
		float rngSquared;

		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( mEnt->z ) != cgs.ccSelectedLayer )
                continue;
		}

		rngSquared = Square(CC_2D_X + mEnt->transformed[0] - cgDC.cursorx) + Square(CC_2D_Y + mEnt->transformed[1] - cgDC.cursory);

		if( i == 0 || rngSquared < rangeSquared ) {
			rangeSquared = rngSquared;
			ent = i;
		}
	}

	if( rangeSquared < Square(8)) {
		return &mapEntities[ent];
	}
	return NULL;
}

typedef enum {
	MT_CONSTRUCT,
	MT_CONSTRUCT_BARE,
	MT_DESTRUCT,
	MT_DESTRUCT_BARE,
	MT_TANK_BARE,
	MT_PLAYER,
	MT_INFO
} menuType_t;

qboolean CG_PlayerSelected( void ) {
	snapshot_t* snap;
	int i;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	for( i = 0; i < MAX_CLIENTS; i++ ) {
		if( cgs.clientinfo[i].team == snap->ps.persistant[PERS_TEAM] ) {
			if( cgs.clientinfo[i].ccSelected ) {
				return qtrue;
			}
		}
	}
	return qfalse;
}

#define CC_MENU_WIDTH 150

qboolean CG_CommandCentreLayersClick( void ) {
	int x, y;
	int i;

	if( !cgs.ccLayers )
		return qfalse;

	x = CC_2D_X + CC_2D_W - 32; 
	y = CC_2D_Y + CC_2D_H - 32;

	for( i = 0; i < cgs.ccLayers; i++ ) {
		if( BG_RectContainsPoint( x, y, 32, 32, cgDC.cursorx, cgDC.cursory ) ) {
			cgs.ccSelectedLayer = i;
			return qtrue;
		}
		y -= 34;
	}

	return qfalse;
}

qboolean CG_CommandCentreSpawnPointClick( void ) {
	int i;
	vec2_t point;

	if( cgs.ccFilter & CC_FILTER_SPAWNS ) {
		return qfalse;
	}

	for ( i = 1; i < cg.spawnCount; i++ ) {
		if( (cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR) && cg.spawnTeams[i] && cg.spawnTeams[i] != CG_LimboPanel_GetRealTeam())
			continue;

		if( cg.spawnTeams[i] & 256 )
			continue;

		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( (int)cg.spawnCoords[i][2] ) != cgs.ccSelectedLayer )
                continue;
		}

		point[0] = cg.spawnCoords[i][0];
		point[1] = cg.spawnCoords[i][1];

		if( BG_RectContainsPoint( point[0] - FLAGSIZE_NORMAL*0.5f, point[1] - FLAGSIZE_NORMAL*0.5f, FLAGSIZE_NORMAL, FLAGSIZE_NORMAL, cgDC.cursorx, cgDC.cursory ) ) {
			trap_SendConsoleCommand( va( "setspawnpt %i\n", i ) );
			cg.selectedSpawnPoint = i;
			cgs.ccRequestedObjective = -1;
			return qtrue;
		}
	}

	return qfalse;
}

void CG_CommandCentreClick( int key ) {
	switch( key ) {
		case K_MOUSE1:
			if( CG_CommandCentreSpawnPointClick() ) {
				return;
			}
			break;
	}
}

char		cg_highlightText[256];
rectDef_t	cg_highlightTextRect;
void CG_CommandMap_SetHighlightText( const char* text, float x, float y ) {
	Q_strncpyz( cg_highlightText, text, sizeof( cg_highlightText ) );
	cg_highlightTextRect.x = x;
	cg_highlightTextRect.y = y;
	expanded = qtrue;
}

void CG_CommandMap_DrawHighlightText( void ) {
	CG_Text_Paint_Ext( cg_highlightTextRect.x, cg_highlightTextRect.y, 0.25f, 0.25f, colorWhite, cg_highlightText, 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.font3 );
	*cg_highlightText = '\0';
}


// =====================================================================================================


void CG_DrawMapOverlaySimple( float x, float y, float w, float h ) {
	int i;
	snapshot_t* snap;
	mapEntityData_t* mEnt = &mapEntities[0];
	int icon_size;
	int exspawn;

	expanded = qfalse;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	{
		vec4_t clr = { 0.f, 0.f, 0.f, 0.75f };

		icon_size = COMMANDMAP_PLAYER_ICON_SIZE;

		// Draw the grid
		CG_DrawGrid( x, y, w, h, NULL );

		trap_R_SetColor( clr );
		CG_DrawPic( x, y, w, h, cgs.media.limboBlendThingy );
		trap_R_SetColor( NULL );

		exspawn = CG_DrawSpawnPointInfo( x, y, w, h, qfalse, NULL, -1 );

		for(i = 0, mEnt = &mapEntities[0]; i < mapEntityCount; i++, mEnt++ ) {
			if( mEnt->team != CG_LimboPanel_GetRealTeam() ) {
				continue;
			}

			if( mEnt->type == ME_PLAYER ||
				mEnt->type == ME_PLAYER_DISGUISED ||
				mEnt->type == ME_PLAYER_REVIVE )
				continue;

			CG_DrawMapEntity( mEnt, x, y, w, h, cgs.ccFilter, NULL, qtrue, snap, icon_size );
		}

		CG_DrawSpawnPointInfo( x, y, w, h, qtrue, NULL, exspawn );

		CG_DrawMortarMarker( x, y, w, h, qtrue, NULL, exspawn );

		for(i = 0, mEnt = &mapEntities[0]; i < mapEntityCount; i++, mEnt++ ) {
			if( mEnt->team != CG_LimboPanel_GetRealTeam() ) {
				continue;
			}

			if( mEnt->type != ME_PLAYER &&
				mEnt->type != ME_PLAYER_DISGUISED &&
				mEnt->type != ME_PLAYER_REVIVE )
				continue;

			CG_DrawMapEntity( mEnt, x, y, w, h, cgs.ccFilter, NULL, qtrue, snap, icon_size );
		}
	}
}

//	CG_DrawMap( x, y, w, h, cgs.ccFilter, &mapScissor, qfalse, 1.f, qfalse );
void CG_DrawMapOverlay( float x, float y, float w, float h, int mEntFilter, mapScissor_t *scissor, float alpha ) {
	int i/*, j = 1*/;
	snapshot_t* snap;
	mapEntityData_t* mEnt = &mapEntities[0];
	int icon_size;
	int exspawn;

	//expanded = qfalse;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	if ( scissor ) {
		// Draw the grid
		CG_DrawGrid( x, y, w, h, scissor );
	} else {
		icon_size = COMMANDMAP_PLAYER_ICON_SIZE;
		
		{
			vec4_t color;
			Vector4Set( color, 1.f, 1.f, 1.f, alpha );
			trap_R_SetColor( color );
			if( cgs.ccLayers ) {
				CG_DrawPic(x, y, w, h, cgs.media.commandCentreMapShaderTrans[cgs.ccSelectedLayer] );
			} else {
				CG_DrawPic(x, y, w, h, cgs.media.commandCentreMapShaderTrans[0] );
			}
			trap_R_SetColor( NULL );
		}

		// Draw the grid
		CG_DrawGrid( x, y, w, h, NULL );
	}

	exspawn = CG_DrawSpawnPointInfo( x, y, w, h, qfalse, scissor, -1 );

	for(i = 0, mEnt = &mapEntities[0]; i < mapEntityCount; i++, mEnt++ ) {
		if( mEnt->team != CG_LimboPanel_GetRealTeam() ) {
			continue;
		}

		if( mEnt->type == ME_PLAYER ||
			mEnt->type == ME_PLAYER_DISGUISED ||
			mEnt->type == ME_PLAYER_REVIVE )
			continue;

        CG_DrawMapEntity( mEnt, x, y, w, h, mEntFilter, scissor, qfalse, snap, icon_size );
	}

	CG_DrawSpawnPointInfo( x, y, w, h, qtrue, scissor, exspawn );

	CG_DrawMortarMarker( x, y, w, h, qtrue, scissor, exspawn );

	for(i = 0, mEnt = &mapEntities[0]; i < mapEntityCount; i++, mEnt++ ) {
		if( mEnt->team != CG_LimboPanel_GetRealTeam() ) {
			continue;
		}

		if( mEnt->type != ME_PLAYER &&
			mEnt->type != ME_PLAYER_DISGUISED &&
			mEnt->type != ME_PLAYER_REVIVE )
			continue;

        CG_DrawMapEntity( mEnt, x, y, w, h, mEntFilter, scissor, qfalse, snap, icon_size );
	}
}
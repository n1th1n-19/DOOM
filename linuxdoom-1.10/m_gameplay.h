// Runtime gameplay tunables, edited from the in-game Options > Gameplay menu.
//
// gp   = what the player set (saved in the config file)
// geff = what the game reads. Equals vanilla while a demo plays/records or in a
//        netgame, so demos stay in sync and peers cannot diverge.

#ifndef __M_GAMEPLAY__
#define __M_GAMEPLAY__

#include "doomtype.h"
#include "m_fixed.h"

typedef struct
{
    int	movespeed;	// percent
    int	gravity;	// percent
    int	friction;	// percent
    int	monhealth;	// percent
    int	monspeed;	// percent
    int	mondamage;	// percent, monster damage to the player
    int	plrdamage;	// percent, player damage to monsters
    int	infammo;	// bool
    int	god;		// bool
    int	respawn;	// bool, monsters respawn
    int	jump;		// bool, Space jumps
    int	alwaysrun;	// bool, not locked: it only changes what the ticcmd records
    int	crosshair;	// bool, not locked: drawn by the video layer
} gameplay_t;

extern gameplay_t	gp;
extern gameplay_t	geff;
// geff.gravity / geff.friction hold ready-to-use fixed_t values.

boolean GP_Locked (void);
void GP_Reset (void);
void GP_Update (void);		// recompute geff, apply respawn flag

#endif

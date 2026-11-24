#include "doomdef.h"
#include "doomstat.h"
#include "m_gameplay.h"

#define VANILLA_FRICTION	0xe800

gameplay_t gp  = { 100, 100, 100, 100, 100, 100, 100, 0, 0, 0, 0, 0 };
gameplay_t geff = { 100, FRACUNIT, VANILLA_FRICTION, 100, 100, 100, 100, 0, 0, 0, 0, 0 };

extern boolean	respawnparm;
extern boolean	netdemo;

boolean GP_Locked (void)
{
    return demorecording || demoplayback || netgame || netdemo;
}

void GP_Reset (void)
{
    gp.movespeed = gp.gravity = gp.friction = 100;
    gp.monhealth = gp.monspeed = gp.mondamage = gp.plrdamage = 100;
    gp.infammo = gp.god = gp.respawn = gp.jump = 0;
    // alwaysrun is a control preference, left alone
}

void GP_Update (void)
{
    gameplay_t v = { 100, 100, 100, 100, 100, 100, 100, 0, 0, 0, 0, 0 };
    boolean base_respawn = (gameskill == sk_nightmare) || respawnparm;

    geff = GP_Locked () ? v : gp;
    geff.alwaysrun = gp.alwaysrun;

    // percent -> the form the game code uses
    geff.gravity  = (int)((long long)FRACUNIT * geff.gravity / 100);
    // friction% scales how much momentum is lost per tic
    {
	int drag = (int)((long long)(FRACUNIT - VANILLA_FRICTION) * geff.friction / 100);
	if (drag < 0)
	    drag = 0;
	if (drag > FRACUNIT-1)
	    drag = FRACUNIT-1;
	geff.friction = FRACUNIT - drag;
    }

    respawnmonsters = base_respawn || geff.respawn;
}

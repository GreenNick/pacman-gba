#ifndef MAIN_H
#define MAIN_H

#include "gba.h"
#include "gameboard.h"

#define ENTITY_WIDTH 9
#define ENTITY_HEIGHT 9
#define GHOSTS_SIZE 4

enum gba_state
{
    INITSTART,
    START,
    INITREADY,
    READY,
    PLAY
};

enum direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct entity
{
    int             row;
    int             col;
    int             deltaX;
    int             deltaY;
    enum direction  direction;
    int             vulnTime;
};

struct state
{
    enum gba_state      gba_state;
    struct entity       player;
    struct entity       ghosts[4];
    struct gameboard    board;
    int                 lives;
    u32                 score;
    u32                 highScore;
    u32                 level;
    u16                 dotsCollected;
};

void playerMovement(struct entity *e, u32 currButtons);
void playerDotCollision(struct state *s, struct entity *e);
void playerPDotCollision(struct state *s, struct entity *e);
void playerGhostCollision(struct state *s, struct entity *p, struct entity *g);
void playerEatsGhost(struct state *s, struct entity *p, struct entity *g);
void playerWallCollision(struct state *s, struct entity *ce, struct entity *pe);
void ghostWallCollision(struct state *s, struct entity *ce);
void entityTunnelCollision(struct entity *e);
void randDirection(struct entity *e);
void drawActiveDots(struct state *s);

#endif

#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "gba.h"
#include "gameboard.h"

#include "images/title.h"
#include "images/background.h"
#include "images/powerdot.h"
#include "images/pacman.h"
#include "images/pacman_up.h"
#include "images/pacman_down.h"
#include "images/pacman_left.h"
#include "images/pacman_right.h"
#include "images/inky.h"
#include "images/pinky.h"
#include "images/blinky.h"
#include "images/clyde.h"
#include "images/blueghost.h"

int main(void)
{
    REG_DISPCNT = MODE3 | BG2_ENABLE;

    // Save current and previous state of button input.
    u32 prevButtons = BUTTONS;
    u32 currButtons = BUTTONS;

    struct entity *currPlayer, *prevPlayer;
    struct entity *currGhosts, *prevGhosts;
    struct state currState, prevState;

    u32 counter = 0;
    char score[11];
    char highScore[11];
    char level[11];
    char lives[11];

    currState.gba_state = INITSTART;
    currState.score = 0;
    currState.highScore = 0;

    while (1)
    {
        prevState = currState;
        currButtons = BUTTONS; // Load the current state of the buttons

        // Reset to start
        if (KEY_JUST_PRESSED(BUTTON_SELECT, currButtons, prevButtons))
        {
            currState.gba_state = INITSTART;
            currState.highScore = 0;
        }

        switch (currState.gba_state)
        {
        case INITSTART:
            waitForVBlank();
            drawFullScreenImageDMA(title);
            drawCenteredString(90, 0, 240, 11, "PUSH START BUTTON", WHITE);
            drawCenteredString(145, 0, 240, 11, "(RE)CREATED BY NICHOLAS BOWERS", WHITE);

            currState.level = 1;
            currState.lives = 3;
            currState.board = board;
            currState.dotsCollected = 0;
            currState.score = 0;
            currState.gba_state = START;
            break;

        case START:
            if (KEY_JUST_PRESSED(BUTTON_START, currButtons, prevButtons))
                currState.gba_state = INITREADY;

            waitForVBlank();
            if (vBlankCounter % 50 == 0)
                drawCenteredString(90, 0, 240, 11, "PUSH START BUTTON", WHITE);
            else if (vBlankCounter % 25 == 0)
                drawRectDMA(90, 0, 240, 11, BLACK);
            break;

        case INITREADY:
            counter = vBlankCounter;

            currPlayer = &currState.player;
            currPlayer->row = 107;
            currPlayer->col = 71;
            currPlayer->deltaX = -1;
            currPlayer->deltaY = 0;
            currPlayer->direction = LEFT;

            currGhosts = &currState.ghosts[0];

            for (u32 i = 0; i < GHOSTS_SIZE; i++)
            {
                currGhosts[i].row = 75;
                currGhosts[i].col = 71;
                currGhosts[i].deltaX = 0;
                currGhosts[i].deltaY = 0;
                currGhosts[i].direction = UP;
                currGhosts[i].vulnTime = 0;
            }

            currGhosts[3].deltaY = -1;

            sprintf(score, "%02d", currState.score);
            sprintf(highScore, "%02d", currState.highScore);
            sprintf(level, "LEVEL %d", currState.level);
            sprintf(lives, "LIVES %d", currState.lives);

            waitForVBlank();
            drawFullScreenImageDMA(background);
            drawCenteredString(89, 52, 46, 11, "READY!", YELLOW);
            drawImageDMA(currPlayer->row, currPlayer->col, ENTITY_WIDTH, ENTITY_HEIGHT, pacman);
            
            drawImageDMA(currGhosts[0].row, currGhosts[0].col, ENTITY_WIDTH, ENTITY_HEIGHT, clyde);
            drawImageDMA(currGhosts[1].row, currGhosts[1].col, ENTITY_WIDTH, ENTITY_HEIGHT, pinky);
            drawImageDMA(currGhosts[2].row, currGhosts[2].col, ENTITY_WIDTH, ENTITY_HEIGHT, inky);
            drawImageDMA(currGhosts[3].row, currGhosts[3].col, ENTITY_WIDTH, ENTITY_HEIGHT, blinky);
            
            drawActiveDots(&currState);

            drawString(13, 160, "HIGH SCORE", WHITE);
            drawRectDMA(24, 156, 76, 11, BLACK);
            drawString(24, 160, highScore, WHITE);

            drawString(39, 160, "1UP", WHITE);
            drawRectDMA(50, 156, 76, 11, BLACK);
            drawString(50, 160, score, WHITE);

            drawRectDMA(65, 156, 76, 11, BLACK);
            drawString(65, 160, level, WHITE);

            drawRectDMA(80, 156, 76, 11, BLACK);
            drawString(80, 160, lives, WHITE);

            currState.gba_state = READY;
            break;

        case READY:
            if ((vBlankCounter - counter) >= 180)
            {
                drawRectDMA(88, 52, 46, 11, BLACK);
                currState.gba_state = PLAY;
            }

            waitForVBlank();
            break;

        case PLAY:
            currPlayer = &currState.player;
            prevPlayer = &prevState.player;

            currGhosts = &currState.ghosts[0];
            prevGhosts = &prevState.ghosts[0];

            if (currState.dotsCollected == 124)
            {
                currState.board = board;
                currState.gba_state = INITREADY;
                currState.dotsCollected = 0;
                currState.level++;
                break;
            }

            playerMovement(currPlayer, currButtons);
            playerWallCollision(&currState, currPlayer, prevPlayer);
            playerDotCollision(&currState, currPlayer);
            playerPDotCollision(&currState, currPlayer);
            entityTunnelCollision(currPlayer);

            // Update player position
            currPlayer->row += currPlayer->deltaY;
            currPlayer->col += currPlayer->deltaX;

            if ((vBlankCounter - counter) == 240)
                currGhosts[2].deltaY = -1;

            if ((vBlankCounter - counter) == 300)
                currGhosts[1].deltaY = -1;

            if ((vBlankCounter - counter) == 360)
                currGhosts[0].deltaY = -1;

            for (u32 i = 0; i < GHOSTS_SIZE; i++)
            {
                struct entity *currGhost = &currGhosts[i];

                ghostWallCollision(&currState, currGhost);
                entityTunnelCollision(currGhost);
                if (currGhost->vulnTime > 0)
                    playerEatsGhost(&currState, currPlayer, currGhost);
                else
                    playerGhostCollision(&currState, currPlayer, currGhost);

                // Update ghost position
                currGhost->row += currGhost->deltaY;
                currGhost->col += currGhost->deltaX;
            }

            

            // Restart game if player runs out of lives
            if (currState.lives < 0)
                currState.gba_state = INITSTART;

            // Update high score
            if (currState.score > currState.highScore)
                currState.highScore = currState.score;

            sprintf(score, "%02d", currState.score);
            sprintf(highScore, "%02d", currState.highScore);

            waitForVBlank();
            drawActiveDots(&currState);

            drawString(13, 160, "HIGH SCORE", WHITE);
            drawRectDMA(24, 156, 76, 11, BLACK);
            drawString(24, 160, highScore, WHITE);

            drawString(39, 160, "1UP", WHITE);
            drawRectDMA(50, 156, 76, 11, BLACK);
            drawString(50, 160, score, WHITE);
            
            drawRectDMA(prevPlayer->row, prevPlayer->col, 9, 9, BLACK);
            switch (currPlayer->direction)
            {
            case UP:
                drawImageDMA(currPlayer->row, currPlayer->col, ENTITY_WIDTH, ENTITY_HEIGHT, pacman_up);
                break;

            case DOWN:
                drawImageDMA(currPlayer->row, currPlayer->col, ENTITY_WIDTH, ENTITY_HEIGHT, pacman_down);
                break;

            case LEFT:
                drawImageDMA(currPlayer->row, currPlayer->col, ENTITY_WIDTH, ENTITY_HEIGHT, pacman_left);
                break;
                
            case RIGHT:
                drawImageDMA(currPlayer->row, currPlayer->col, ENTITY_WIDTH, ENTITY_HEIGHT, pacman_right);
                break;
            }

            drawRectDMA(73, 70, 11, 1, WHITE);

            for (u32 i = 0; i < GHOSTS_SIZE; i++)
                drawRectDMA(prevGhosts[i].row, prevGhosts[i].col, 9, 9, BLACK);

            drawImageDMA(currGhosts[0].row, currGhosts[0].col, ENTITY_WIDTH, ENTITY_HEIGHT, clyde);
            drawImageDMA(currGhosts[1].row, currGhosts[1].col, ENTITY_WIDTH, ENTITY_HEIGHT, pinky);
            drawImageDMA(currGhosts[2].row, currGhosts[2].col, ENTITY_WIDTH, ENTITY_HEIGHT, inky);
            drawImageDMA(currGhosts[3].row, currGhosts[3].col, ENTITY_WIDTH, ENTITY_HEIGHT, blinky);

            for (u32 i = 0; i < GHOSTS_SIZE; i++)
            {
                if (currGhosts[i].vulnTime > 0)
                {
                    currGhosts[i].vulnTime--;
                    drawImageDMA(currGhosts[i].row, currGhosts[i].col, ENTITY_WIDTH, ENTITY_HEIGHT, blueghost);
                }
            }

            break;
        }

        prevButtons = currButtons; // Store the current state of the buttons
    }

    return 0;
}

void playerMovement(struct entity *e, u32 currButtons)
{
    if (KEY_DOWN(BUTTON_UP, currButtons))
    {
        e->deltaX = 0;
        e->deltaY = -1;
        e->direction = UP;
    }
    if (KEY_DOWN(BUTTON_DOWN, currButtons))
    {
        e->deltaX = 0;
        e->deltaY = 1;
        e->direction = DOWN;
    }
    if (KEY_DOWN(BUTTON_LEFT, currButtons))
    {
        e->deltaX = -1;
        e->deltaY = 0;
        e->direction = LEFT;
    }
    if (KEY_DOWN(BUTTON_RIGHT, currButtons))
    {
        e->deltaX = 1;
        e->deltaY = 0;
        e->direction = RIGHT;
    }
}

void playerDotCollision(struct state *s, struct entity *e)
{
    for (u16 i = 0; i < DOTS_SIZE; i++)
    {
        struct dot *d = &s->board.dots[i];

        if (d->dot_state == ACTIVE
        && e->col + 4 == d->col
        && e->row + 4 == d->row)
        {
            d->dot_state = INACTIVE;
            s->score += 10;
            s->dotsCollected++;
        }
    }
}

void playerPDotCollision(struct state *s, struct entity *e)
{
    for (u16 i = 0; i < POWERDOTS_SIZE; i++)
    {
        struct dot *d = &s->board.powerdots[i];

        if (d->dot_state == ACTIVE
        && e->col + 4 == d->col + 3
        && e->row + 4 == d->row + 3)
        {
            d->dot_state = INACTIVE;
            s->score += 50;
            s->dotsCollected++;
            s->ghosts[0].vulnTime = 300;
            s->ghosts[1].vulnTime = 300;
            s->ghosts[2].vulnTime = 300;
            s->ghosts[3].vulnTime = 300;
        }
    }
}

void playerGhostCollision(struct state *s, struct entity *p, struct entity *g)
{
    if ((p->col + 4 > g->col && p->col + 4 < g->col + ENTITY_WIDTH && p->row == g->row)
    || (p->row + 4 > g->row && p->row + 4 < g->row + ENTITY_HEIGHT && p->col == g->col))
    {
        s->gba_state = INITREADY;
        s->lives--;
    }
}

void playerEatsGhost(struct state *s, struct entity *p, struct entity *g)
{
    if ((p->col + 4 > g->col && p->col + 4 < g->col + ENTITY_WIDTH && p->row == g->row)
    || (p->row + 4 > g->row && p->row + 4 < g->row + ENTITY_HEIGHT && p->col == g->col))
    {
        g->row = 75;
        g->col = 71;
        g->deltaX = 0;
        g->deltaY = -1;
        g->direction = UP;
        g->vulnTime = 0;
        s->score += 200;
    }
}

void playerWallCollision(struct state *s, struct entity *ce, struct entity *pe)
{
    for (u16 i = 0; i < WALLS_SIZE; i++)
    {
        struct wall *wall = &s->board.walls[i];

        // Collision bottom
        if (ce->deltaY == -1 || pe->deltaY == -1)
            if (ce->row == wall->row + wall->height 
            && ce->col + ENTITY_WIDTH > wall->col
            && ce->col < wall->col + wall->width)
            {
                ce->deltaX = pe->deltaX;
                ce->deltaY = 0;
                ce->direction = pe->direction;
            }

        // Collision top
        if (ce->deltaY == 1 || pe->deltaY == 1)
            if (ce->row + ENTITY_HEIGHT == wall->row 
            && ce->col + ENTITY_WIDTH > wall->col
            && ce->col < wall->col + wall->width)
            {
                ce->deltaX = pe->deltaX;
                ce->deltaY = 0;
                ce->direction = pe->direction;
            }

        // Collision right
        if (ce->deltaX == -1 || pe->deltaX == -1)
            if (ce->col == wall->col + wall->width 
            && ce->row + ENTITY_HEIGHT > wall->row
            && ce->row < wall->row + wall->height)
            {
                ce->deltaX = 0;
                ce->deltaY = pe->deltaY;
                ce->direction = pe->direction;
            }

        // Collision left
        if (ce->deltaX == 1 || pe->deltaX == 1)
            if (ce->col + ENTITY_WIDTH == wall->col
            && ce->row + ENTITY_HEIGHT > wall->row
            && ce->row < wall->row + wall->height)
            {
                ce->deltaX = 0;
                ce->deltaY = pe->deltaY;
                ce->direction = pe->direction;
            }
    }
}

void ghostWallCollision(struct state *s, struct entity *ce)
{
    for (u16 i = 0; i < WALLS_SIZE; i++)
    {
        struct wall *wall = &s->board.walls[i];

        // Collision bottom
        if (ce->deltaY == -1)
            if (ce->row == wall->row + wall->height 
            && ce->col + ENTITY_WIDTH > wall->col
            && ce->col < wall->col + wall->width)
            {
                randDirection(ce);
                ghostWallCollision(s, ce);
            }

        // Collision top
        if (ce->deltaY == 1)
            if (ce->row + ENTITY_HEIGHT == wall->row 
            && ce->col + ENTITY_WIDTH > wall->col
            && ce->col < wall->col + wall->width)
            {
                randDirection(ce);
                ghostWallCollision(s, ce);
            }

        // Collision right
        if (ce->deltaX == -1)
            if (ce->col == wall->col + wall->width 
            && ce->row + ENTITY_HEIGHT > wall->row
            && ce->row < wall->row + wall->height)
            {
                randDirection(ce);
                ghostWallCollision(s, ce);
            }

        // Collision left
        if (ce->deltaX == 1)
            if (ce->col + ENTITY_WIDTH == wall->col
            && ce->row + ENTITY_HEIGHT > wall->row
            && ce->row < wall->row + wall->height)
            {
                randDirection(ce);
                ghostWallCollision(s, ce);
            }

    }
}

void entityTunnelCollision(struct entity *e)
{
    if (e->col < 3)
        e->col = 139;

    if (e->col + ENTITY_WIDTH > 148)
        e->col = 3;
}

void randDirection(struct entity *e)
{
    enum direction dir = randint(0, 4);

    switch (dir)
    {
    case UP:
        e->deltaX = 0;
        e->deltaY = -1;
        break;

    case DOWN:
        e->deltaX = 0;
        e->deltaY = 1;
        break;

    case LEFT:
        e->deltaX = -1;
        e->deltaY = 0;
        break;
    
    case RIGHT:
        e->deltaX = 1;
        e->deltaY = 0;
        break;
    }
}

void drawActiveDots(struct state *s)
{
    for (u16 i = 0; i < DOTS_SIZE; i++)
    {
        struct dot *d = &s->board.dots[i];

        if (d->dot_state == ACTIVE)
            setPixel(d->row, d->col, WHITE);
    }

    for (u16 i = 0; i < POWERDOTS_SIZE; i++)
    {
        struct dot *d = &s->board.powerdots[i];

        if (d->dot_state == ACTIVE && vBlankCounter % 16 == 0)
            drawImageDMA(d->row, d->col, POWERDOT_WIDTH, POWERDOT_HEIGHT, powerdot);
        else if (d->dot_state == ACTIVE && vBlankCounter % 8 == 0)
            drawRectDMA(d->row, d->col, POWERDOT_WIDTH, POWERDOT_HEIGHT, BLACK);
    }
}
#ifndef GAMEBOARD_H
#define GAMEBOARD_H

enum dot_state
{
    ACTIVE,
    INACTIVE
};

struct wall
{
    int row;
    int col;
    int width;
    int height;
};

struct dot
{
    int             row;
    int             col;
    enum dot_state  dot_state;
};

struct gameboard
{
    struct wall walls[39];
    struct dot  dots[120];
    struct dot  powerdots[4];
};

extern struct gameboard board;
#define WALLS_SIZE 39
#define DOTS_SIZE 120
#define POWERDOTS_SIZE 4

#endif
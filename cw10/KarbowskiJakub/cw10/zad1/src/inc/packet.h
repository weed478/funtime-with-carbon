#ifndef JK_10_01_PACKET_H
#define JK_10_01_PACKET_H

#include "game.h"

#define PACKET_MAX_SIZE (256)
#define PLAYER_NAME_MAX (32)
#define STATUS_MESSAGE_MAX (64)

typedef enum packet_type_t
{
    PACKET_INIT,
    PACKET_MOVE,
    PACKET_GAME,
    PACKET_STATUS,
} packet_type_t;

typedef struct init_packet_t
{
    char name[PLAYER_NAME_MAX + 1];
} init_packet_t;

typedef struct move_packet_t
{
    char name[PLAYER_NAME_MAX + 1];
    pos_t pos;
} move_packet_t;

typedef struct game_packet_t
{
    game_t game;
} game_packet_t;

typedef struct status_packet_t
{
    int err;
    char msg[STATUS_MESSAGE_MAX + 1];
} status_packet_t;

typedef struct packet_t
{
    packet_type_t type;
    union
    {
        init_packet_t init;
        move_packet_t move;
        game_packet_t game;
        status_packet_t status;
    };
} packet_t;

void create_packet(void *buf, const packet_t *packet);

void parse_packet(const void *buf, packet_t *packet);

#endif

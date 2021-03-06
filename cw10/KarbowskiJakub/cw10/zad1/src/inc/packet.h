#ifndef JK_10_01_PACKET_H
#define JK_10_01_PACKET_H

#include "game.h"
#include "err.h"

#define PACKET_MAX_SIZE (256)
#define PLAYER_NAME_MAX (32)

typedef enum packet_type_t
{
    PACKET_INIT,
    PACKET_MOVE,
    PACKET_GAME,
    PACKET_STATUS,
    PACKET_PING,
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
    player_t player;
    game_t game;
    char opponent[PLAYER_NAME_MAX + 1];
} game_packet_t;

typedef struct status_packet_t
{
    err_t err;
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

void packet_create(void *buf, const packet_t *packet);

void packet_parse(const void *buf, packet_t *packet);

err_t packet_send(int fd, const packet_t *packet);

err_t packet_receive(int fd, packet_t *packet);

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>

#include "client.h"
#include "game.h"
#include "log.h"
#include "err.h"

static const char HELP[] =
        "SO Lab 10 - Jakub Karbowski\n"
        "Usage:\n"
        "%s NAME net|unix ADDRESS\n";

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, HELP, argv[0]);
        return -1;
    }

    const char *client_name = argv[1];

    connection_type_t connection_type;
    if (!strcmp("net", argv[2]))
    {
        connection_type = CONNECTION_NET;
    }
    else if (!strcmp("unix", argv[2]))
    {
        connection_type = CONNECTION_UNIX;
    }
    else
    {
        fprintf(stderr, HELP, argv[0]);
        return -1;
    }

    const char *server_address = argv[3];

    client_session_t session;
    if (client_connect(&session, connection_type, server_address))
    {
        LOGE("Could not connect to server");
        return -1;
    }

    LOGI("Connected");

    err_t res = client_log_in(&session, client_name);
    if (res)
    {
        LOGE("Error logging in: %s", err_msg(res));
        client_disconnect(&session);
        return -1;
    }

    LOGI("Logged in, waiting for opponent");

    while (session.connected)
    {
        game_t game;
        player_t player;
        char opponent[PLAYER_NAME_MAX];
        if (client_get_game(&session, &game, &player, opponent))
        {
            LOGE("Error getting game");
            break;
        }

        printf("Your opponent is [%s]\n", opponent);
        printf("You are %c\n", player == PLAYER_X ? 'X' : 'O');
        game_print(&game);

        if (game.is_over) break;

        if (game.next_player == player)
        {
            pos_t pos;
            for (;;)
            {
                printf("Enter your move (1-9):\n");
                int c = 0;
                do
                {
                    struct pollfd fds[2];
                    fds[0].fd = STDIN_FILENO;
                    fds[0].events = POLLIN;
                    fds[1].fd = session.sock;
                    fds[1].events = POLLIN;
                    if (poll(fds, 2, -1) > 0)
                    {
                        if (fds[0].revents & POLLIN)
                            c = fgetc(stdin);
                        else if (fds[1].revents & POLLIN)
                        {
                            packet_t ping;
                            packet_receive(session.sock, &ping);
                            if (ping.type == PACKET_PING)
                                packet_send(session.sock, &ping);
                        }
                    }
                } while (c != '1' && c != '2' && c != '3' &&
                         c != '4' && c != '5' && c != '6' &&
                         c != '7' && c != '8' && c != '9');
                while (fgetc(stdin) != '\n');
                pos = (pos_t) (c - '1');

                if (game_move(&game, pos))
                {
                    printf("Invalid move!\n");
                }
                else break;
            }

            res = client_send_move(&session, pos);
            if (res)
            {
                LOGE("Move error: %s", err_msg(res));
                break;
            }
        }
    }

    client_disconnect(&session);

    return 0;
}

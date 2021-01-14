#include "server.h"

char player_name[MAX_PLAYER][MAX_NAME_LENGTH] = {};
int player_game_joined[MAX_PLAYER] = {};
int game_started = 0;
uint8_t server_board[BOARD_SIZE][BOARD_SIZE] = {};


void connect6_packet_process(int player_idx, const int *player_sockets,
                             const unsigned char *recv_buf, size_t recv_buf_size)
{
    // Protocol header and data for parsing and sending
    unsigned char *sending = (unsigned char *) malloc(recv_buf_size);
    size_t sending_len;
    struct Connect6ProtocolHdr hdr;
    struct GameStartData rcv_gsd, snd_gsd;
    struct PutTurnData ptd;
    struct GameOverData god;
    int i, all_joined = 1;

    if (sending == NULL) return;

    hdr_parsing(recv_buf, recv_buf_size, &hdr);

    switch(hdr.type)
    {
        case GAME_START: //////////////////////////////////////////////////////
        printf("GAME_START packet received\n");

        game_start_data_parsing(
            recv_buf+PROTOCOL_HEADER_SIZE,
            recv_buf_size-PROTOCOL_HEADER_SIZE,
            &rcv_gsd
        );

        // Game is not started and the player is not joined
        if (!game_started && !player_game_joined[player_idx]) {
            // Register to server's variables
            player_game_joined[player_idx] = 1;
            strncpy(player_name[player_idx], rcv_gsd.name, rcv_gsd.name_length);

            // Check all players are joined
            for (i = 0; i < MAX_PLAYER; i++) {
                all_joined &= player_game_joined[i];
            }

            // if yes...
            if (all_joined) {
                // Send GAME_START to all players
                game_started = 1;

                for (i = 0; i < MAX_PLAYER; i++) {
                    // i; target player
                    // (i+1) % MAX_PLAYER; another player
                    snd_gsd.req_res_flag = 0x01;
                    snd_gsd.name_length = strlen(player_name[(i+1) % MAX_PLAYER]) + 1;
                    strncpy(snd_gsd.name, player_name[(i+1) % MAX_PLAYER], snd_gsd.name_length);

                    make_game_start_payload(sending, recv_buf_size, &sending_len, i+1, snd_gsd);

                    // Send packet
                    send(player_sockets[i], sending, sending_len, 0);
                }
            }
        }

        break;

        case PUT: /////////////////////////////////////////////////////////////
        printf("PUT packet received\n");

        break;

        case TURN:
        printf("TURN packet received\n");
        break;

        case GAME_OVER:
        printf("GAME_OVER packet received\n");
        break;

        case ERROR:
        printf("ERROR packet received\n");
        break;

        case TIMEOUT:
        printf("TIMEOUT packet received\n");
        break;

        case GAME_DISCARD:
        printf("GAME_DISCARD packet received\n");
        break;

        default:
        printf("Illegal packet received\n");
    }

    free((unsigned char *)sending);
}
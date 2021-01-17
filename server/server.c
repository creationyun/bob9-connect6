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
    struct PutTurnData rcv_ptd, snd_ptd;
    struct GameOverData god;
    int i, all_joined = 1;
    int other_player_idx = (player_idx + 1) % MAX_PLAYER;
    uint8_t game_result = 0;

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

        if (game_started) {
            // Game is already started: send ERROR
            make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_GAME_ALREADY_STARTED);
            send(player_sockets[player_idx], sending, sending_len, 0);
        } else if (player_game_joined[player_idx]) {
            // Player is already joined: send ERROR
            make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_EXCEED_CAPACITY);
            send(player_sockets[player_idx], sending, sending_len, 0);
        } else {
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

                // Send initial PUT, TURN packet - PUT(player1, 1, 9, 9)
                snd_ptd.coord_num = 1;
                snd_ptd.xy[0] = BOARD_SIZE/2;
                snd_ptd.xy[1] = BOARD_SIZE/2;
                make_turn_payload(sending, recv_buf_size, &sending_len, 1, snd_ptd);
                send(player_sockets[1], sending, sending_len, 0);  // send to player2
                make_put_payload(sending, recv_buf_size, &sending_len, 1, snd_ptd);
                send(player_sockets[0], sending, sending_len, 0);  // send to player1
            }
        }

        break;

        case PUT: /////////////////////////////////////////////////////////////
        printf("PUT packet received\n");
        
        put_turn_data_parsing(
            recv_buf+PROTOCOL_HEADER_SIZE,
            recv_buf_size-PROTOCOL_HEADER_SIZE,
            &rcv_ptd
        );

        if (game_started) {
            // apply to server's board
            for (i = 0; i < rcv_ptd.coord_num; i++) {
                server_board[rcv_ptd.xy[2*i+1]][rcv_ptd.xy[2*i]] = player_idx+1;

                // checking connect-6
                check_connect6(rcv_ptd.xy[2*i], rcv_ptd.xy[2*i+1], player_idx+1, &game_result);
                if (game_result == RESULT_WIN_OR_LOSE) break;
            }

            // copy data
            snd_ptd = rcv_ptd;

            // Make TURN payload and send
            make_turn_payload(sending, recv_buf_size, &sending_len, player_idx+1, snd_ptd);
            send(player_sockets[other_player_idx], sending, sending_len, 0);

            if (game_result == RESULT_WIN_OR_LOSE) {
                // GAME_OVER data field setting
                god.coord_num = 0;
                //god.xy[0] = x;
                //god.xy[1] = y;
                god.result = game_result;

                // Make GAME_OVER payload and send
                make_game_over_payload(sending, recv_buf_size, &sending_len, player_idx+1, god);
                send(player_sockets[player_idx], sending, sending_len, 0);
                send(player_sockets[other_player_idx], sending, sending_len, 0);
            }
        } else {
            // Game is not started: send ERROR
            make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_GAME_NOT_STARTED);
            send(player_sockets[player_idx], sending, sending_len, 0);
        }

        break;

        case TURN: /////////////////////////////////////////////////////////////
        printf("TURN packet received\n");

        // receiving TURN packet of server is not allowed.
        // so, send ERROR
        make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_PROTOCOL_NOT_VALID);
        send(player_sockets[player_idx], sending, sending_len, 0);
        break;

        case GAME_OVER: /////////////////////////////////////////////////////////////
        printf("GAME_OVER packet received\n");

        // receiving GAME_OVER packet of server is not allowed.
        // so, send ERROR
        make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_PROTOCOL_NOT_VALID);
        send(player_sockets[player_idx], sending, sending_len, 0);
        break;

        case ERROR: /////////////////////////////////////////////////////////////
        printf("ERROR packet received\n");
        break;

        case TIMEOUT: /////////////////////////////////////////////////////////////
        printf("TIMEOUT packet received\n");

        // receiving TIMEOUT packet of server is not allowed.
        // so, send ERROR
        make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_PROTOCOL_NOT_VALID);
        send(player_sockets[player_idx], sending, sending_len, 0);
        break;

        case GAME_DISCARD: /////////////////////////////////////////////////////////////
        printf("GAME_DISCARD packet received\n");

        if (game_started) {
            // GAME_OVER data field setting
            god.coord_num = 0;
            god.result = RESULT_GAME_DISCARDED;

            // Make GAME_OVER payload and send
            make_game_over_payload(sending, recv_buf_size, &sending_len, other_player_idx+1, god);
            send(player_sockets[other_player_idx], sending, sending_len, 0);
        } else {
            make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_GAME_NOT_STARTED);
            send(player_sockets[player_idx], sending, sending_len, 0);
        }
        break;

        default: /////////////////////////////////////////////////////////////
        printf("Illegal packet received\n");

        // send ERROR
        make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_PROTOCOL_NOT_VALID);
        send(player_sockets[player_idx], sending, sending_len, 0);
    }

    free((unsigned char *)sending);
}


void check_connect6(uint8_t x, uint8_t y, uint8_t target_player, uint8_t *game_result)
{
    if (game_result == NULL) return;

    int i = y, j = x;
    int count = 0;

    // x check (<- ->)
    while (j > 0 && server_board[i][j-1] == target_player)
        j--;
    
    while (j < BOARD_SIZE && server_board[i][j] == target_player) {
        count++;
        j++;
    }
    
    if (count >= 6) // Connect-6 complete!
    {
        *game_result = RESULT_WIN_OR_LOSE;
        return;
    }

    // y check (^ v)
    i = y;
    j = x;
    count = 0;

    while (i > 0 && server_board[i-1][j] == target_player)
        i--;
    
    while (i < BOARD_SIZE && server_board[i][j] == target_player) {
        count++;
        i++;
    }
    
    if (count >= 6) // Connect-6 complete!
    {
        *game_result = RESULT_WIN_OR_LOSE;
        return;
    }

    // cross check (right-bottom)
    i = y;
    j = x;
    count = 0;

    while (i > 0 && j > 0 && server_board[i-1][j-1] == target_player) {
        i--;
        j--;
    }
    
    while (i < BOARD_SIZE && j < BOARD_SIZE && server_board[i][j] == target_player) {
        count++;
        i++;
        j++;
    }
    
    if (count >= 6) // Connect-6 complete!
    {
        *game_result = RESULT_WIN_OR_LOSE;
        return;
    }

    // cross check (left-bottom)
    i = y;
    j = x;
    count = 0;

    while (i > 0 && j < BOARD_SIZE && server_board[i-1][j+1] == target_player) {
        i--;
        j++;
    }
    
    while (i < BOARD_SIZE && j >= 0 && server_board[i][j] == target_player) {
        count++;
        i++;
        j--;
    }
    
    if (count >= 6) // Connect-6 complete!
    {
        *game_result = RESULT_WIN_OR_LOSE;
        return;
    }
}

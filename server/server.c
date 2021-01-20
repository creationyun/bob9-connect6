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
    int i, j, all_joined = 1;
    int other_player_idx = (player_idx + 1) % MAX_PLAYER;
    uint8_t game_result = 0, errorcode;

    if (sending == NULL) {
        printf("Error: cannot allocate heap memory.\n");
        return;
    }

    // Header parsing
    errorcode = hdr_parsing(recv_buf, recv_buf_size, &hdr);

    if (errorcode != 0) {
        make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, errorcode);
        send(player_sockets[player_idx], sending, sending_len, 0);

        free((unsigned char *)sending);
        return;
    }

    // Disposed by each types
    switch(hdr.type)
    {
        case GAME_START: //////////////////////////////////////////////////////
        printf("GAME_START packet received\n");

        errorcode = game_start_data_parsing(
            recv_buf+PROTOCOL_HEADER_SIZE,
            recv_buf_size-PROTOCOL_HEADER_SIZE,
            &rcv_gsd
        );

        if (errorcode != 0) {
            make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, errorcode);
            send(player_sockets[player_idx], sending, sending_len, 0);
            break;
        }

        if (game_started) {
            printf("ERROR packet sent - ERROR_GAME_ALREADY_STARTED\n");
            // Game is already started: send ERROR
            make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_GAME_ALREADY_STARTED);
            send(player_sockets[player_idx], sending, sending_len, 0);
        } else if (player_game_joined[player_idx]) {
            printf("ERROR packet sent - ERROR_EXCEED_CAPACITY\n");
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

                    printf("GAME_START packet sent to player%d\n", i+1);
                    make_game_start_payload(sending, recv_buf_size, &sending_len, i+1, snd_gsd);

                    // Send packet
                    send(player_sockets[i], sending, sending_len, 0);
                }

                printf("Initial PUT & TURN packet sent\n");
                printf("(%u, %u)\n", BOARD_SIZE/2, BOARD_SIZE/2);
                // Send initial PUT, TURN packet - PUT(player1, 1, 9, 9)
                server_board[BOARD_SIZE/2][BOARD_SIZE/2] = 1;
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
        
        errorcode = put_turn_data_parsing(
            recv_buf+PROTOCOL_HEADER_SIZE,
            recv_buf_size-PROTOCOL_HEADER_SIZE,
            &rcv_ptd
        );

        if (errorcode != 0) {
            make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, errorcode);
            send(player_sockets[player_idx], sending, sending_len, 0);
            break;
        }

        if (game_started) {
            // apply to server's board
            for (i = 0; i < rcv_ptd.coord_num; i++) {
                printf("(%u, %u) ", rcv_ptd.xy[2*i], rcv_ptd.xy[2*i+1]);
                server_board[rcv_ptd.xy[2*i+1]][rcv_ptd.xy[2*i]] = player_idx+1;

                // checking connect-6
                check_connect6(rcv_ptd.xy[2*i], rcv_ptd.xy[2*i+1], player_idx+1, &game_result, god.xy);
                if (game_result == RESULT_WIN_OR_LOSE) break;
            }
            printf("\n");

            /*
            for (i = 0; i < BOARD_SIZE; i++) {
                for (j = 0; j < BOARD_SIZE; j++) {
                    printf("%u ", server_board[i][j]);
                }
                printf("\n");
            }
            */

            // copy data
            snd_ptd = rcv_ptd;

            printf("TURN packet sent\n");
            // Make TURN payload and send
            make_turn_payload(sending, recv_buf_size, &sending_len, player_idx+1, snd_ptd);
            send(player_sockets[other_player_idx], sending, sending_len, 0);

            if (game_result == RESULT_WIN_OR_LOSE) {
                // GAME_OVER data field setting
                god.coord_num = 6;
                god.result = game_result;

                printf("GAME_OVER packet sent - RESULT_WIN_OR_LOSE\n");
                // Make GAME_OVER payload and send
                make_game_over_payload(sending, recv_buf_size, &sending_len, player_idx+1, god);
                send(player_sockets[player_idx], sending, sending_len, 0);
                send(player_sockets[other_player_idx], sending, sending_len, 0);

                // game end.
                game_started = 0;
                init_game();
            }
        } else {
            printf("ERROR packet sent - ERROR_GAME_NOT_STARTED\n");
            // Game is not started: send ERROR
            make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_GAME_NOT_STARTED);
            send(player_sockets[player_idx], sending, sending_len, 0);
        }

        break;

        case TURN: /////////////////////////////////////////////////////////////
        printf("TURN packet received\n");

        printf("ERROR packet sent - ERROR_PROTOCOL_NOT_VALID\n");
        // receiving TURN packet of server is not allowed.
        // so, send ERROR
        make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_PROTOCOL_NOT_VALID);
        send(player_sockets[player_idx], sending, sending_len, 0);
        break;

        case GAME_OVER: /////////////////////////////////////////////////////////////
        printf("GAME_OVER packet received\n");

        printf("ERROR packet sent - ERROR_PROTOCOL_NOT_VALID\n");
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

        printf("ERROR packet sent - ERROR_PROTOCOL_NOT_VALID\n");
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

            printf("GAME_OVER packet sent - RESULT_GAME_DISCARDED\n");
            // Make GAME_OVER payload and send
            make_game_over_payload(sending, recv_buf_size, &sending_len, other_player_idx+1, god);
            send(player_sockets[other_player_idx], sending, sending_len, 0);
            
            // game end.
            game_started = 0;
            init_game();
        } else {
            printf("ERROR packet sent - ERROR_GAME_NOT_STARTED\n");
            make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_GAME_NOT_STARTED);
            send(player_sockets[player_idx], sending, sending_len, 0);
        }
        break;

        default: /////////////////////////////////////////////////////////////
        printf("Illegal packet received\n");

        printf("ERROR packet sent - ERROR_PROTOCOL_NOT_VALID\n");
        // send ERROR
        make_error_payload(sending, recv_buf_size, &sending_len, player_idx+1, ERROR_PROTOCOL_NOT_VALID);
        send(player_sockets[player_idx], sending, sending_len, 0);
    }

    free((unsigned char *)sending);
}


void check_connect6(uint8_t x, uint8_t y, uint8_t target_player, uint8_t *game_result_code, uint8_t *result_xy)
{
    if (game_result_code == NULL) return;

    int i = y, j = x;
    int count = 0;

    // x check (<- ->)
    while (j >= 0 && server_board[i][j] == target_player)
        j--;
    
    while (j < BOARD_SIZE-1 && server_board[i][j+1] == target_player) {
        j++;
        result_xy[2*count] = j;
        result_xy[2*count+1] = i;
        count++;
        if (count >= 6) break;
    }
    
    if (count >= 6) // Connect-6 complete!
    {
        *game_result_code = RESULT_WIN_OR_LOSE;
        return;
    }

    // y check (^ v)
    i = y;
    j = x;
    count = 0;

    while (i >= 0 && server_board[i][j] == target_player)
        i--;
    
    while (i < BOARD_SIZE-1 && server_board[i+1][j] == target_player) {
        i++;
        result_xy[2*count] = j;
        result_xy[2*count+1] = i;
        count++;
        if (count >= 6) break;
    }
    
    if (count >= 6) // Connect-6 complete!
    {
        *game_result_code = RESULT_WIN_OR_LOSE;
        return;
    }

    // cross check (right-bottom)
    i = y;
    j = x;
    count = 0;

    while (i >= 0 && j >= 0 && server_board[i][j] == target_player) {
        i--;
        j--;
    }
    
    while (i < BOARD_SIZE-1 && j < BOARD_SIZE-1 && server_board[i+1][j+1] == target_player) {
        i++;
        j++;
        result_xy[2*count] = j;
        result_xy[2*count+1] = i;
        count++;
        if (count >= 6) break;
    }
    
    if (count >= 6) // Connect-6 complete!
    {
        *game_result_code = RESULT_WIN_OR_LOSE;
        return;
    }

    // cross check (left-bottom)
    i = y;
    j = x;
    count = 0;

    while (i >= 0 && j < BOARD_SIZE && server_board[i][j] == target_player) {
        i--;
        j++;
    }
    
    while (i < BOARD_SIZE-1 && j > 0 && server_board[i+1][j-1] == target_player) {
        i++;
        j--;
        result_xy[2*count] = j;
        result_xy[2*count+1] = i;
        count++;
        if (count >= 6) break;
    }
    
    if (count >= 6) // Connect-6 complete!
    {
        *game_result_code = RESULT_WIN_OR_LOSE;
        return;
    }
}

void init_game()
{
    int i;

    for (i = 0; i < MAX_PLAYER; i++) {
        memset(player_name[i], 0, MAX_NAME_LENGTH);
    }

    for (i = 0; i < BOARD_SIZE; i++) {
        memset(server_board[i], 0, BOARD_SIZE);
    }
}

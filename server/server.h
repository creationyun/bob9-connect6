#ifndef __CONNECT6_SERVER_H__
#define __CONNECT6_SERVER_H__

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include "../connect6_protocol/connect6_protocol.h"

// Fixed Game Options
//#define PORT 8089
#define MAX_PLAYER 2
#define TIMEOUT_SECONDS 30

extern char player_name[MAX_PLAYER][MAX_NAME_LENGTH];
extern int player_game_joined[MAX_PLAYER];
extern int game_started;
extern uint8_t server_board[BOARD_SIZE][BOARD_SIZE];
extern int player_turn;

void connect6_packet_process(int player_idx, const int *player_sockets,
                             const unsigned char *recv_buf, size_t recv_buf_size);
void check_connect6(uint8_t x, uint8_t y, uint8_t target_player, uint8_t *game_result_code, uint8_t *result_xy);
void init_game();

#endif

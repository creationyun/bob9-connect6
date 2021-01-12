#pragma once

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
#define PORT 8089
#define MAX_PLAYER 2
#define BOARD_SIZE 19
#define TIMEOUT_SECONDS 30

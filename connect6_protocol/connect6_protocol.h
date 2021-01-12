#pragma once

#include <stdint.h>

#if defined _WIN32 || defined __CYGWIN__
	#ifdef WIN_EXPORT
		#ifdef __GNUC__
			#define EXPORTED __attribute__ ((dllexport))
		#else
			#define EXPORTED __declspec(dllexport)
		#endif
	#else
		#ifdef __GNUC__
			#define EXPORTED __attribute__ ((dllimport))
		#else
			#define EXPORTED __declspec(dllimport)
		#endif
	#endif
	#define NOT_EXPORTED
#else
	#if __GNUC__ >= 4
		#define EXPORTED __attribute__ ((visibility ("default")))
		#define NOT_EXPORTED __attribute__ ((visibility ("hidden")))
	#else
		#define EXPORTED
		#define NOT_EXPORTED
	#endif
#endif

#define PROTOCOL_VER 0
#define MAX_NAME_LENGTH 100

// Protocol types
#define GAME_START 0x00
#define PUT 0x01
#define TURN 0x02
#define GAME_OVER 0x03
#define ERROR 0x04
#define TIMEOUT 0x05
#define GAME_DISCARD 0x06

// Error types
#define ERROR_SERVER_INTERNAL_ERROR 0x00
#define ERROR_PROTOCOL_NOT_VALID 0x01
#define ERROR_EXCEED_COORDINATE_RANGE 0x02
#define ERROR_GAME_NOT_STARTED 0x03
#define ERROR_EXCEED_CAPACITY 0x04
#define ERROR_EXCEED_NAME_LENGTH 0x05

#ifdef __cplusplus
extern "C" {
#endif
	// Protocol header
	struct Connect6ProtocolHdr {
		uint8_t version;
		uint8_t type;
		uint8_t player_num;
		uint8_t data_length;
	};

	// GAME_START data field
	struct GameStartData {
		uint8_t req_res_flag;
		uint8_t name_length;
		char name[MAX_NAME_LENGTH];
	};

	// PUT or TURN data field
	struct PutTurnData {
		uint8_t coord_num;
		uint8_t xy[2*2];
	};

	// GAME_OVER data field
	struct GameOverData {
		uint8_t result;
		uint8_t coord_num;
		uint8_t xy[2*6];
	};

	void EXPORTED hdr_parsing(const unsigned char *payload, struct Connect6ProtocolHdr *header);
	uint8_t EXPORTED game_start_data_parsing(const unsigned char *data_payload, struct GameStartData *data);
	uint8_t EXPORTED put_turn_data_parsing(const unsigned char *data_payload, struct PutTurnData *data);
	uint8_t EXPORTED game_over_data_parsing(const unsigned char *data_payload, struct GameOverData *data);
	void EXPORTED error_data_parsing(const unsigned char *data_payload, uint8_t *error_type);

	void EXPORTED make_game_start_payload(unsigned char *payload, const uint8_t player_num, const struct GameStartData data);
	void EXPORTED make_put_payload(unsigned char *payload, const uint8_t player_num, const struct PutTurnData data);
	void EXPORTED make_turn_payload(unsigned char *payload, const uint8_t player_num, const struct PutTurnData data);
	void EXPORTED make_game_over_payload(unsigned char *payload, const uint8_t player_num, const struct GameOverData data);
	void EXPORTED make_error_payload(unsigned char *payload, const uint8_t player_num, const uint8_t error_type);
	void EXPORTED make_timeout_payload(unsigned char *payload, const uint8_t player_num);
	void EXPORTED make_game_discard_payload(unsigned char *payload, const uint8_t player_num);
#ifdef __cplusplus
}
#endif

#include "connect6_protocol.h"

// Header parsing
void EXPORTED hdr_parsing(const unsigned char *payload, size_t payload_size, struct Connect6ProtocolHdr *header)
{
    if (header == NULL) return;
    if (payload_size < PROTOCOL_HEADER_SIZE) return; 

    header->version = payload[0];
    header->type = payload[1];
    header->player_num = payload[2];
    header->data_length = payload[3];
}

// GAME_START data parsing
uint8_t EXPORTED game_start_data_parsing(const unsigned char *data_payload, size_t data_payload_size, struct GameStartData *data)
{
    int i;

    if (data == NULL) return ERROR_MISUSE_FUNCTION;
    if (data_payload_size < 2) return ERROR_PROTOCOL_NOT_VALID;

    data->req_res_flag = data_payload[0];
    data->name_length = data_payload[1];

    if (data_payload_size < (size_t)(data->name_length) + 2) return ERROR_PROTOCOL_NOT_VALID;

    if (data->name_length > MAX_NAME_LENGTH) {
        return ERROR_EXCEED_NAME_LENGTH;
    }

    for (i = 0; i < (data->name_length); i++) {
        data->name[i] = (char)data_payload[2+i];
    }

    return 0;
}

// PUT or TURN data parsing
uint8_t EXPORTED put_turn_data_parsing(const unsigned char *data_payload, size_t data_payload_size, struct PutTurnData *data)
{
    int i;

    if (data == NULL) return ERROR_MISUSE_FUNCTION;
    if (data_payload_size < 1) return ERROR_PROTOCOL_NOT_VALID;

    data->coord_num = data_payload[0];

    if (data_payload_size < (size_t)(data->coord_num)*2 + 1) return ERROR_PROTOCOL_NOT_VALID;

    if (data->coord_num > 2) {
        return ERROR_PROTOCOL_NOT_VALID;
    }

    for (i = 0; i < data->coord_num; i++) {
        data->xy[2*i] = data_payload[2*i+1];
        data->xy[2*i+1] = data_payload[2*i+2];
    }

    return 0;
}

// GAME_OVER data parsing
uint8_t EXPORTED game_over_data_parsing(const unsigned char *data_payload, size_t data_payload_size, struct GameOverData *data)
{
    int i;

    if (data == NULL) return ERROR_MISUSE_FUNCTION;
    if (data_payload_size < 2) return ERROR_PROTOCOL_NOT_VALID;

    data->result = data_payload[0];
    data->coord_num = data_payload[1];

    if (data_payload_size < (size_t)(data->coord_num)*2 + 2) return ERROR_PROTOCOL_NOT_VALID;

    if (data->coord_num > 6) {
        return ERROR_PROTOCOL_NOT_VALID;
    }

    for (i = 0; i < (data->coord_num); i++) {
        data->xy[2*i] = data_payload[2*i+2];
        data->xy[2*i+1] = data_payload[2*i+3];
    }

    return 0;
}

// ERROR data parsing
void EXPORTED error_data_parsing(const unsigned char *data_payload, size_t data_payload_size, uint8_t *error_type)
{
    if (error_type == NULL) return;
    if (data_payload_size < 1) return;

    *error_type = data_payload[0];
}

// Make GAME_START payload
void EXPORTED make_game_start_payload(unsigned char *payload, size_t payload_size, size_t *payload_len_written,
                                      const uint8_t player_num, const struct GameStartData data)
{
    int i;
    size_t desired_len = data.name_length + 2 + PROTOCOL_HEADER_SIZE;

    if (payload == NULL) return;
    if (payload_size < desired_len) return;

    // Header field
    payload[0] = PROTOCOL_VER;  // Version
    payload[1] = GAME_START;    // Type
    payload[2] = player_num;    // PlayerNum
    payload[3] = data.name_length + 2;  // DataLength
    
    // Data field
    payload[4] = data.req_res_flag;
    payload[5] = data.name_length;
    for (i = 0; i < data.name_length; i++) {
        payload[6+i] = (unsigned char)data.name[i];
    }

    if (payload_len_written != NULL) {
        *payload_len_written = desired_len;
    }
}

void EXPORTED make_put_payload(unsigned char *payload, size_t payload_size, size_t *payload_len_written,
                               const uint8_t player_num, const struct PutTurnData data)
{
    int i;
    size_t desired_len = data.coord_num * 2 + 1 + PROTOCOL_HEADER_SIZE;

    if (payload == NULL) return;
    if (payload_size < desired_len) return;

    // Header field
    payload[0] = PROTOCOL_VER;  // Version
    payload[1] = PUT;           // Type
    payload[2] = player_num;    // PlayerNum
    payload[3] = data.coord_num * 2 + 1;  // DataLength

    // Data field
    payload[4] = data.coord_num;
    for (i = 0; i < data.coord_num; i++) {
        payload[5+2*i] = data.xy[2*i];
        payload[6+2*i] = data.xy[2*i+1];
    }

    if (payload_len_written != NULL) {
        *payload_len_written = desired_len;
    }
}

void EXPORTED make_turn_payload(unsigned char *payload, size_t payload_size, size_t *payload_len_written,
                                const uint8_t player_num, const struct PutTurnData data)
{
    int i;
    size_t desired_len = data.coord_num * 2 + 1 + PROTOCOL_HEADER_SIZE;

    if (payload == NULL) return;
    if (payload_size < desired_len) return;

    // Header field
    payload[0] = PROTOCOL_VER;  // Version
    payload[1] = TURN;          // Type
    payload[2] = player_num;    // PlayerNum
    payload[3] = data.coord_num * 2 + 1;  // DataLength

    // Data field
    payload[4] = data.coord_num;
    for (i = 0; i < data.coord_num; i++) {
        payload[5+2*i] = data.xy[2*i];
        payload[6+2*i] = data.xy[2*i+1];
    }

    if (payload_len_written != NULL) {
        *payload_len_written = desired_len;
    }
}

void EXPORTED make_game_over_payload(unsigned char *payload, size_t payload_size, size_t *payload_len_written,
                                     const uint8_t player_num, const struct GameOverData data)
{
    int i;
    size_t desired_len = data.coord_num * 2 + 2 + PROTOCOL_HEADER_SIZE;

    if (payload == NULL) return;
    if (payload_size < desired_len) return;

    // Header field
    payload[0] = PROTOCOL_VER;  // Version
    payload[1] = GAME_OVER;     // Type
    payload[2] = player_num;    // PlayerNum
    payload[3] = data.coord_num * 2 + 2;  // DataLength

    // Data field
    payload[4] = data.result;
    payload[5] = data.coord_num;
    for (i = 0; i < data.coord_num; i++) {
        payload[6+2*i] = data.xy[2*i];
        payload[7+2*i] = data.xy[2*i+1];
    }

    if (payload_len_written != NULL) {
        *payload_len_written = desired_len;
    }
}

void EXPORTED make_error_payload(unsigned char *payload, size_t payload_size, size_t *payload_len_written,
                                 const uint8_t player_num, const uint8_t error_type)
{
    size_t desired_len = PROTOCOL_HEADER_SIZE + 1;

    if (payload == NULL) return;
    if (payload_size < desired_len) return;

    // Header field
    payload[0] = PROTOCOL_VER;  // Version
    payload[1] = ERROR;         // Type
    payload[2] = player_num;    // PlayerNum
    payload[3] = 1;             // DataLength

    // Data field
    payload[4] = error_type;

    if (payload_len_written != NULL) {
        *payload_len_written = desired_len;
    }
}

void EXPORTED make_timeout_payload(unsigned char *payload, size_t payload_size, size_t *payload_len_written,
                                   const uint8_t player_num)
{
    size_t desired_len = PROTOCOL_HEADER_SIZE;

    if (payload == NULL) return;
    if (payload_size < desired_len) return;

    // Header field
    payload[0] = PROTOCOL_VER;  // Version
    payload[1] = TIMEOUT;       // Type
    payload[2] = player_num;    // PlayerNum
    payload[3] = 0;             // DataLength

    if (payload_len_written != NULL) {
        *payload_len_written = desired_len;
    }
}

void EXPORTED make_game_discard_payload(unsigned char *payload, size_t payload_size, size_t *payload_len_written,
                                        const uint8_t player_num)
{
    size_t desired_len = PROTOCOL_HEADER_SIZE;

    if (payload == NULL) return;
    if (payload_size < desired_len) return;

    // Header field
    payload[0] = PROTOCOL_VER;  // Version
    payload[1] = GAME_DISCARD;  // Type
    payload[2] = player_num;    // PlayerNum
    payload[3] = 0;             // DataLength

    if (payload_len_written != NULL) {
        *payload_len_written = desired_len;
    }
}

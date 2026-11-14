#ifndef COCOLINX_SOM_V2_HELPER_H___
#define COCOLINX_SOM_V2_HELPER_H___

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define RESPONSE_DATA_SIZE_MAX 512
#define AT_TX_DATA_SIZE_MAX    512
#define AT_TX_BYTES_SIZE_MAX   256
#define TOKEN_SIZE_MAX         32
#define CR '\r'
#define LF '\n'
#define COMMA ','
#define COLON ':'
#define QUOTE '"'
#define SPACE ' '
#define NULL_TERMINATOR '\0'

typedef enum {
    AT_OK = 0,
    AT_ERROR,
    AT_CME_ERROR,
    AT_CMS_ERROR
} AtRespCode;

typedef enum {
    URC_NONE = 0,
    URC_CEREG,
    URC_MQTTEVT,
    URC_XMQTTMSG,
    URC_XGNSS,
    URC_XGNSSPOS,
    URC_UNKNOWN
} UrcType;

typedef struct 
{
    const char *buf;
    int32_t len;
} AtToken;

typedef struct 
{
    const char *at;
    int32_t count;
    int32_t resp_code;
    int32_t at_error_code;
    AtToken tokens[TOKEN_SIZE_MAX];
} AtParser;

void trim_cr_lf(const char **str);
void trim_left_space(const char **str);
void trim_quote(const char **str, int32_t *len);
bool is_resp(const char *str);
bool is_end_response(const char *str);
bool is_end_line(const char *str);
bool is_colon_or_comma(const char *str);
bool is_quote(const char *str);
int32_t get_urc_type(const char *str, const char *end);
int32_t find_prefix_token(const char *str, int32_t start_idx);
int32_t get_at_error_ack(void);
bool is_empty_token(uint8_t token_index);

int32_t char_to_hex(char c, uint8_t *x);
int32_t hex_to_bin(const char *hex, size_t hexlen, uint8_t *buf, size_t buflen);
int32_t bin_to_hex(const uint8_t *buf, int32_t buflen, char *hex, int32_t hexlen);

bool char_to_int32_raw(const char *str, int32_t len, int32_t *resp);
bool char_to_int32(const AtToken *tok, int32_t *resp);
int32_t transfer_pkt_raw(const uint8_t *cmd, int32_t cmd_size, int32_t max_tokens, uint32_t timeout_ms);
int32_t at_pkt_parser(int32_t max_tokens, const uint8_t *cmd, int32_t cmd_size);

int32_t urc_pkt_parser(UrcPacket *urc_packet, const char *end);
int32_t parse_urc_cereg(int32_t *cereg, const char **cursor, const char *end);
int32_t parse_urc_mqtt_evt(int32_t *evt_type, int32_t *result, const char **cursor, const char *end);
int32_t parse_urc_mqtt_msg(char *mqtt_topic, int32_t mqtt_topic_size, 
                        char *mqtt_msg, int32_t mqtt_msg_size, const char **cursor, const char *end);
int32_t parse_urc_gnss(GnssData *gnss_data, const char **cursor, const char *end);
int32_t parse_urc_gnss_pos(GnssData *gnss_data, const char **cursor, const char *end);

#endif /* COCOLINX_SOM_V2_HELPER_H_ */
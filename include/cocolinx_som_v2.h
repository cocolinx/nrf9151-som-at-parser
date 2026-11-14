#ifndef CSHIELD_V2_H
#define CSHIELD_V2_H

#include "_device.h"

/* plmn number */
typedef enum {
    PLMN_NOT_SET = -1,
    PLMN_AUTO = 0,
    PLMN_SKT = 45005,
    PLMN_KT = 45008,
    PLMN_LGU = 45006
} LtePlmn;

typedef enum {
    ACK_OKAY = 0,

    // Common errors
    ACK_ERR_ARG,
    ACK_ERR_TIMEOUT,
    ACK_ERR_PARSE,
    ACK_ERR_RESP_OVERFLOW,

    // CMNG return value
    ACK_NO_MATCH,

    // AT command response
    ACK_AT_ERROR = 101,
    ACK_AT_CME_ERROR,
    ACK_AT_CMS_ERROR,

    // URC results
    ACK_URC_NONE =  0x0000,
    ACK_URC_CEREG = 0x0100,
    ACK_URC_MQTT_EVT =  0x0200,
    ACK_URC_MQTT_MSG = 0x0400,
    ACK_URC_GNSS_EVT =  0x0800,
    ACK_URC_GNSS_DATA = 0x1000
} AckCode;

// * 10^6 before save
typedef struct 
{
    // #XGNSS
    int32_t gnss_service;
    int32_t gnss_status;

    // #XGNSSPOS
    int32_t latitude;
    int32_t longitude;
    int32_t altitude;
    int32_t accuracy;
    int32_t speed;
    int32_t heading;
    char datetime[32];
} GnssData;

typedef struct
{
    int32_t cereg;

    char *mqtt_topic;
    int32_t mqtt_topic_size;

    char *mqtt_msg;
    int32_t mqtt_msg_size;

    int32_t mqtt_evt_type;
    int32_t mqtt_result;

    GnssData gnss_data;
} UrcPacket;


bool begin();

int32_t get_at_error_code();
int32_t read_urc_pkt(UrcPacket *urc_packet);

/**************************************** AT set command ****************************************/
/* Mobile termination control and status commands */
int32_t set_cfun(int32_t fun);
int32_t set_cclk(char *time);

/* Packet domain commands */
int32_t set_cgdcont(int32_t cid, char *pdp_type, char *apn);
int32_t set_cgatt(int32_t state);

/* Network service related commands */
int32_t set_cops(int32_t mode, int32_t format, int32_t oper);
int32_t set_cereg(int32_t n);

/* Mobile termination errors */
int32_t set_cmee(int32_t n);

/* GNSS AT commands */
int32_t cx_gnss_control(int32_t cloud_assistance, int32_t interval, int32_t timeout);
int32_t cx_gnss_stop();
int32_t cx_gnss_del(int32_t mask);

/* MQTT client AT commands */
int32_t cx_mqtt_cfg(const char *client_id, int32_t keep_alive, int32_t clean_session);
int32_t cx_mqtt_con(int32_t op, const char *username, const char *password, const char *url, int32_t port);
int32_t cx_mqtt_disconnect();
int32_t cx_mqtt_con_secure(int32_t op, const char *username, const char *password, const char *url, int32_t port, int32_t sec_tag);
int32_t cx_mqtt_sub(const char *topic, int32_t qos);
int32_t cx_mqtt_unsub(const char *topic);
int32_t cx_mqtt_pub(const char *topic, const uint8_t *msg, int32_t qos, int32_t retain);
int32_t cx_mqtt_pub_str(const char *topic, const char *msg, int32_t qos, int32_t retain);

/* Socket AT commands */
int32_t cx_socket(int32_t family, int32_t type, int32_t role, int32_t *handle, int32_t cid);
int32_t cx_close(int32_t handle, int32_t *result);
int32_t cx_close_all();
int32_t cx_bind(int32_t handle, int32_t port);
int32_t cx_connect(int32_t handle, const char *url, int32_t port, int32_t *status);
int32_t cx_send(int32_t handle, int32_t flags, uint8_t *data, int32_t size);
int32_t cx_send_str(int32_t handle, int32_t flags, char *data, int32_t size);
int32_t cx_send_to(int32_t handle, int32_t flags, const char *url, int32_t port, uint8_t *data, int32_t size);
int32_t cx_send_to_str(int32_t handle, int32_t flags, const char *url, int32_t port, char *data, int32_t size);

/**************************************** AT read command ****************************************/
/* General */
int32_t get_cgmi(char *manufacturer, int32_t max_size);
int32_t get_cgmm(char *model, int32_t max_size);
int32_t get_cgmr(char *revision, int32_t max_size);
int32_t get_imei(char *imei, int32_t max_size);
int32_t get_uuid(char *uuid, int32_t max_size);

/* Mobile termination control and status commands */
int32_t get_cfun(int32_t *fun);
int32_t get_cesq(int32_t *rsrq, int32_t *rsrp);
int32_t get_band(int32_t *band); 
int32_t get_cclk(char *time, uint8_t max_size);

/* Packet domain commands */
int32_t get_cgdcont(int32_t *cid, char *pdp_type, char *apn);
int32_t get_cgatt(int32_t *state);

/* Network service related commands */
int32_t get_cops(int32_t *oper);
int32_t get_cereg(int32_t *stat);

/* Mobile termination errors */
int32_t get_cmee(int32_t *n);

/* UICC access commands */
int32_t get_imsi(char *imsi, int32_t max_size);
int32_t get_iccid(char *iccid, int32_t max_size);

/* GNSS AT commands */
int32_t cx_gnss_read(int32_t *gnss_service, int32_t *gnss_status);

/* MQTT client AT commands */
int32_t cx_get_mqtt_cfg(char *client_id, int32_t max_size, int32_t *keep_alive, int32_t *clean_session);
int32_t cx_get_mqtt_con(int32_t *status, char *url, int32_t max_size, int32_t *port, int32_t *sec_tag);

/* Socket AT commands */
int32_t cx_get_socket(int32_t target_handle, int32_t *family, int32_t *role, int32_t *type);
int32_t cx_recv(int32_t handle, int32_t flags, int32_t timeout, uint8_t *data, int32_t max_size);
int32_t cx_recv_str(int32_t handle, int32_t flags, int32_t timeout, char *data, int32_t max_size);
int32_t cx_recv_from(int32_t handle, int32_t flags, int32_t timeout, uint8_t *data, int32_t max_size);
int32_t cx_recv_from_str(int32_t handle, int32_t flags, int32_t timeout, char *data, int32_t max_size);
int32_t cx_get_addr_info(const char *hostname, char *ip_addresses, int32_t max_size, int32_t address_family);

#endif // CSHIELD_V2_H

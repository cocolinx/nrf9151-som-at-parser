/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) 2026 CocoLinx Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This file contains code derived from the Zephyr Project.
 * Modified by CocoLinx Co., Ltd. in 2026.
 */

#include "../include/cocolinx_som_v2.h"
#include "../include/cocolinx_som_v2_helper.h"

static char _pktbuf[RESPONSE_DATA_SIZE_MAX+8];
static char _urcbuf[RESPONSE_DATA_SIZE_MAX+8];
static AtParser _parser;

static void cx_at_parser_init(void);
int32_t transfer_pkt(const char *cmd, int32_t max_tokens, uint32_t timeout_ms);

/** 
 * @breif: Customize this part according to your MCU or OS.
 * @note uart_read() should be non-blocking
 */ 
static void uart_write(const uint8_t *data, size_t size)
{
    /* Send AT commad data to the nRF9151-SOM via UART */
}

static int32_t uart_read(uint8_t *buf, size_t bufsize)
{
    /* Read received response data from nRF9151-SOM via UART */
}

static void uart_flush(void)
{
    /* Clear pending data from the UART RX buffer */
}

static uint32_t get_ms(void)
{
    /* Return system uptime in milliseconds */
}

static void sleep_ms(uint32_t ms)
{
    /* Sleep for the specified time in milliseconds */
}

bool begin(void)
{
    /* Add UART initialization here if required */
    
    sleep_ms(500);

    cx_at_parser_init();

    memset(_pktbuf, 0, sizeof(_pktbuf));
  
    uint32_t start_ms = get_ms();

    while((get_ms() - start_ms) <= 5000)
    {
        transfer_pkt("AT", 16, 1000);
        if(_parser.resp_code == AT_OK) return true;
        sleep_ms(500);
    }

    return false;
}

int32_t get_at_error_code()
{
    return _parser.at_error_code;
}

int32_t read_urc_pkt(UrcPacket *urc_packet)
{
    if(urc_packet == NULL) return -(ACK_ERR_ARG);   

    int32_t rxcnt = 0;
    int32_t last_rxcnt = 0;
    uint32_t last_rx_ms = get_ms();

    while((get_ms() - last_rx_ms) <= 10)
    {     
        rxcnt += uart_read((uint8_t *)&_urcbuf[rxcnt], RESPONSE_DATA_SIZE_MAX - rxcnt);
        if(last_rxcnt < rxcnt) last_rx_ms = get_ms();
        last_rxcnt = rxcnt;
        if(rxcnt > RESPONSE_DATA_SIZE_MAX) return -(ACK_ERR_RESP_OVERFLOW);
    }

    if(rxcnt == 0) return ACK_URC_NONE;

    _urcbuf[rxcnt] = '\0';

    const char *end = _urcbuf + rxcnt;

    return urc_pkt_parser(urc_packet, end);
}
/**************************************** AT set command ****************************************/

/* Mobile termination control and status commands */
int32_t set_cfun(int32_t fun)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+CFUN=%d", fun);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t set_cclk(char *time)
{
    if(time == NULL) return -(ACK_ERR_ARG);

    char cmd[64];

    snprintf(cmd, 64, "AT+CCLK=\"%s\"", time);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* Packet domain commands */
int32_t set_cgdcont(int32_t cid, char *pdp_type, char *apn)
{
    if(pdp_type == NULL || apn == NULL) return -(ACK_ERR_ARG);

    char cmd[64];

    snprintf(cmd, 64, "AT+CGDCONT=%d,\"%s\",\"%s\"", cid, pdp_type, apn);

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}


int32_t set_cgatt(int32_t state)
{
    char cmd[32];

    snprintf(cmd, 32, "AT+CGATT=%d", state);

    int32_t ack = transfer_pkt(cmd, 16, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* Network service related commands */
int32_t set_cops(int32_t mode, int32_t format, int32_t oper)
{
    char cmd[32];

    if(mode == 0) snprintf(cmd, 32, "AT+COPS=%d", mode);
    else snprintf(cmd, 32, "AT+COPS=%d,%d,\"%d\"", mode, format, oper);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t set_cereg(int32_t n)
{
    char cmd[32];

    snprintf(cmd, 32, "AT+CEREG=%d", n);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* Mobile termination errors */
int32_t set_cmee(int32_t n)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+CMEE=%d", n);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* GNSS AT commands */
int32_t cx_gnss_control(int32_t cloud_assistance, int32_t interval, int32_t timeout)
{
    char cmd[64];

    if(interval == 1) snprintf(cmd, 64, "AT#XGNSS=1,%d,%d", cloud_assistance, interval);
    else snprintf(cmd, 64, "AT#XGNSS=1,%d,%d,%d", cloud_assistance, interval, timeout);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t cx_gnss_stop()
{
    char cmd[16];

    strcpy(cmd, "AT#XGNSS=0");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t cx_gnss_del(int32_t mask)
{
    char cmd[32];

    snprintf(cmd, 32, "AT#XGNSSDEL=%d", mask);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}


/* MQTT client AT commands */
int32_t cx_mqtt_cfg(const char *client_id, int32_t keep_alive, int32_t clean_session)
{
    char cmd[128];

    snprintf(cmd, 128, "AT#XMQTTCFG=\"%s\",%d,%d", 
        (client_id == NULL) ? "" : client_id, keep_alive, clean_session);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;    
}

int32_t cx_mqtt_con(int32_t op, const char *username, const char *password, const char *url, int32_t port)
{
    char cmd[128];

    if(url == NULL) return -(ACK_ERR_ARG);

    snprintf(cmd, 128, "AT#XMQTTCON=%d,\"%s\",\"%s\",\"%s\",%d",
        op, (username == NULL) ? "" : username, (password == NULL) ? "" : password, url, port);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t cx_mqtt_disconnect()
{
    char cmd[16];

    strcpy(cmd, "AT#XMQTTCON=0");

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t cx_mqtt_con_secure(int32_t op, const char *username, const char *password, const char *url, int32_t port, int32_t sec_tag)
{
    char cmd[128];

    if(url == NULL) return -(ACK_ERR_ARG);

    snprintf(cmd, 128, "AT#XMQTTCON=%d,\"%s\",\"%s\",\"%s\",%d,%d",
        op, (username == NULL) ? "" : username, (password == NULL) ? "" : password, url, port, sec_tag);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t cx_mqtt_sub(const char *topic, int32_t qos)
{
    char cmd[128];

    snprintf(cmd, 128, "AT#XMQTTSUB=\"%s\",%d", (topic == NULL) ? "" : topic, qos);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    if(ack != 0) return ack;
    
    return ACK_OKAY;
}

int32_t cx_mqtt_unsub(const char *topic)
{
    char cmd[128];

    if(topic == NULL) return -(ACK_ERR_ARG);
    
    snprintf(cmd, 128, "AT#XMQTTUNSUB=\"%s\"", topic);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    if(ack != 0) return ack;
    
    return ACK_OKAY;
}

int32_t cx_mqtt_pub(const char *topic, const uint8_t *msg, int32_t qos, int32_t retain)
{
    char cmd[512];

    snprintf(cmd, 512, "AT#XMQTTPUB=\"%s\",\"%s\",%d,%d", topic, msg, qos, retain);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    if(ack != 0) return ack;
    
    return ACK_OKAY;
}

int32_t cx_mqtt_pub_str(const char *topic, const char *msg, int32_t qos, int32_t retain)
{
    return cx_mqtt_pub(topic, (const uint8_t *)msg, qos, retain);
}


/* Socket AT commands */
int32_t cx_socket(int32_t family, int32_t type, int32_t role, int32_t *handle, int32_t cid)
{
    int32_t val;
    char cmd[32];
    bool ret;

    snprintf(cmd, 32, "AT#XSOCKET=%d,%d,%d,%d", family, type, role, cid);

    int32_t ack = transfer_pkt(cmd, 16, 20000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 4) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("#XSOCKET", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(handle != NULL) *handle = val;

    return ACK_OKAY;
}

int32_t cx_close(int32_t handle, int32_t *result)
{
    int32_t val;
    char cmd[32];
    bool ret;

    snprintf(cmd, 32, "AT#XCLOSE=%d", handle);

    int32_t ack = transfer_pkt(cmd, 16, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("#XCLOSE", 0);
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(result != NULL) *result = val;

    return ACK_OKAY;
}

int32_t cx_close_all()
{
    char cmd[32];

    snprintf(cmd, 32, "AT#XCLOSE");

    int32_t ack = transfer_pkt(cmd, 16, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t cx_bind(int32_t handle, int32_t port)
{
    char cmd[32];

    snprintf(cmd, 32, "AT#XBIND=%d,%d", handle, port);

    int32_t ack = transfer_pkt(cmd, 16, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t cx_connect(int32_t handle, const char *url, int32_t port, int32_t *status)
{
    int32_t val;
    char cmd[256];
    bool ret;

    snprintf(cmd, 256, "AT#XCONNECT=%d,\"%s\",%d", handle, url, port);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("#XCONNECT", 0);
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(status != NULL) *status = val;

    return ACK_OKAY;
}

int32_t cx_send(int32_t handle, int32_t flags, uint8_t *data, int32_t size)
{
    char cmd[AT_TX_DATA_SIZE_MAX + 64];
    char data_c[AT_TX_DATA_SIZE_MAX];
    bool ret;

    if(size > AT_TX_BYTES_SIZE_MAX) return -(ACK_ERR_ARG);
    if(data == NULL) return -(ACK_ERR_ARG);

    ret = bin_to_hex(data, size, data_c, AT_TX_DATA_SIZE_MAX);
    if(ret == 0) return -(ACK_ERR_PARSE);

    snprintf(cmd, AT_TX_DATA_SIZE_MAX + 64, "AT#XSEND=%d,1,%d,\"%s\"", handle, flags, data_c);

    int32_t ack = transfer_pkt(cmd, 16, 20000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t cx_send_str(int32_t handle, int32_t flags, char *data, int32_t size)
{
    return cx_send(handle, flags, (uint8_t *)data, size);
}

int32_t cx_send_to(int32_t handle, int32_t flags, const char *url, int32_t port, uint8_t *data, int32_t size)
{
    char cmd[AT_TX_DATA_SIZE_MAX + 64];
    char data_c[AT_TX_DATA_SIZE_MAX];
    bool ret;

    if(size > AT_TX_BYTES_SIZE_MAX) return -(ACK_ERR_ARG);
    if(data == NULL) return -(ACK_ERR_ARG);

    ret = bin_to_hex(data, size, data_c, AT_TX_DATA_SIZE_MAX);
    if(ret == 0) return -(ACK_ERR_PARSE);

    snprintf(cmd, AT_TX_DATA_SIZE_MAX + 64, "AT#XSENDTO=%d,1,%d,\"%s\",%d,\"%s\"", handle, flags, url, port, data_c);

    int32_t ack = transfer_pkt(cmd, 16, 20000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    
    return ACK_OKAY;
}

int32_t cx_send_to_str(int32_t handle, int32_t flags, const char *url, int32_t port, char *data, int32_t size)
{
    return cx_send_to(handle, flags, url, port, (uint8_t *)data, size);
}

/**************************************** AT read command ****************************************/

/* General */
int32_t get_cgmi(char *manufacturer, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CGMI");

    int32_t ack = transfer_pkt(cmd, 1, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 1) return -(ACK_ERR_PARSE);

    if(manufacturer != NULL)
    {
        int32_t len = _parser.tokens[0].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(manufacturer, _parser.tokens[0].buf, len);
        manufacturer[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t get_cgmm(char *model, int32_t max_size)
{
    char cmd[16];
    
    strcpy(cmd, "AT+CGMM");

    int32_t ack = transfer_pkt(cmd, 1, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 1) return -(ACK_ERR_PARSE);

    if(model != NULL)
    {
        int32_t len = _parser.tokens[0].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(model, _parser.tokens[0].buf, len);
        model[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t get_cgmr(char *revision, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CGMR");

    int32_t ack = transfer_pkt(cmd, 1, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 1) return -(ACK_ERR_PARSE);

    if(revision != NULL)
    {
        int32_t len = _parser.tokens[0].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(revision, _parser.tokens[0].buf, len);
        revision[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t get_imei(char *imei, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CGSN=1");

    int32_t ack = transfer_pkt(cmd, 2, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CGSN", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    if(imei != NULL)
    {
        int32_t len = _parser.tokens[index + 1].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        trim_quote(&_parser.tokens[index + 1].buf, &len);
        memcpy(imei, _parser.tokens[index + 1].buf, len);
        imei[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t get_uuid(char *uuid, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT\%XMODEMUUID");

    int32_t ack = transfer_pkt(cmd, 2, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("\%XMODEMUUID", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    if(uuid != NULL)
    {
        int32_t len = _parser.tokens[index + 1].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        trim_quote(&_parser.tokens[index + 1].buf, &len);
        memcpy(uuid, _parser.tokens[index + 1].buf, len);
        uuid[len] = '\0';
    }

    return ACK_OKAY;
}

/* Mobile termination control and status commands */
int32_t get_cfun(int32_t *fun)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CFUN?");

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CFUN", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);    

    if(fun != NULL) *fun = val;

    return ACK_OKAY;
}

int32_t get_cesq(int32_t *rsrq, int32_t *rsrp)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CESQ");

    int32_t ack = transfer_pkt(cmd, 16, 2000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 7) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CESQ", 0);
    if(index < 0 || index + 6 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 5], &val);
    if(!ret) return -(ACK_ERR_PARSE);   

    if(rsrq != NULL) *rsrq = (val - 40) / 2;

    ret = char_to_int32(&_parser.tokens[index + 6], &val);
    if(!ret) return -(ACK_ERR_PARSE);       

    if(rsrp != NULL) *rsrp = val - 141;

    return ACK_OKAY;
}

int32_t get_band(int32_t *band)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT\%XCBAND");

    int32_t ack = transfer_pkt(cmd, 16, 2000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("\%XCBAND", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE); 

    if(band != NULL) *band = val;

    return ACK_OKAY;
}

int32_t get_cclk(char *time, uint8_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CCLK?");

    int32_t ack = transfer_pkt(cmd, 2, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CCLK", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    if(time != NULL)
    {
        int32_t len = _parser.tokens[index + 1].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        trim_quote(&_parser.tokens[index + 1].buf, &len);
        memcpy(time, _parser.tokens[index + 1].buf, len);
        time[len] = '\0';
    }

    return ACK_OKAY;
}

/* Packet domain commands */
int32_t get_cgdcont(int32_t *cid, char *pdp_type, char *apn)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CGDCONT?");

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 7) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CGDCONT", 0);
    if(index < 0 || index + 3 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(cid != NULL) *cid = val;

    if(pdp_type != NULL)
    {
        int32_t len = _parser.tokens[index + 2].len;
        trim_quote(&_parser.tokens[index + 2].buf, &len);
        memcpy(pdp_type, _parser.tokens[index + 2].buf, len);
        pdp_type[len] = '\0';
    }

    if(apn != NULL)
    {
        int32_t len = _parser.tokens[index + 3].len;
        trim_quote(&_parser.tokens[index + 3].buf, &len);
        memcpy(apn, _parser.tokens[index + 3].buf, len);
        apn[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t get_cgatt(int32_t *state)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CGATT?");

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CGATT", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);   

    if(state != NULL) *state = val;

    return ACK_OKAY;
}

/* Network service related commands */
int32_t get_cops(int32_t *oper)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd,"AT+COPS?");

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 5) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+COPS", 0);
    if(index < 0 || index + 3 >= _parser.count) return -(ACK_ERR_PARSE);

    int32_t len = _parser.tokens[index + 3].len;
    trim_quote(&_parser.tokens[index + 3].buf, &len);

    ret = char_to_int32(&_parser.tokens[index + 3], &val);
    if(!ret) return -(ACK_ERR_PARSE);   

    if(oper != NULL) *oper = val;

    return ACK_OKAY;
}

int32_t get_cereg(int32_t *stat)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CEREG?");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CEREG", 0);
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(stat != NULL) *stat = val;
    
    return ACK_OKAY;
}

/* Mobile termination errors */
int32_t get_cmee(int32_t *n)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CMEE?");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CMEE", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(n != NULL) *n = val;

    return ACK_OKAY;
}

/* UICC access commands */
int32_t get_imsi(char *imsi, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CIMI");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 1) return -(ACK_ERR_PARSE);

    if(imsi != NULL)
    {
        int32_t len = _parser.tokens[0].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(imsi, _parser.tokens[0].buf, len);
        imsi[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t get_iccid(char *iccid, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT\%XICCID");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("\%XICCID", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    if(iccid != NULL)
    {
        int32_t len = _parser.tokens[index + 1].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(iccid, _parser.tokens[index + 1].buf, len);
        iccid[len] = '\0';
    }

    return ACK_OKAY;
}

/* GNSS AT commands */
int32_t cx_gnss_read(int32_t *gnss_service, int32_t *gnss_status)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT#XGNSS?");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("#XGNSS", 0);
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(gnss_service != NULL) *gnss_service = val;

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(gnss_status != NULL) *gnss_status = val;

    return ACK_OKAY;
}

/* MQTT client AT commands */
int32_t cx_get_mqtt_cfg(char *client_id, int32_t max_size, int32_t *keep_alive, int32_t *clean_session)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT#XMQTTCFG?");

    int32_t ack = transfer_pkt(cmd, 16, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 4) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("#XMQTTCFG", 0);
    if(index < 0 || index + 3 >= _parser.count) return -(ACK_ERR_PARSE);

    if(client_id != NULL)
    {
        int32_t len = _parser.tokens[index + 1].len;
        trim_quote(&_parser.tokens[index + 1].buf, &len);
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(client_id, _parser.tokens[index + 1].buf, len);
        client_id[len] = '\0';
    }

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(keep_alive != NULL) *keep_alive = val;

    ret = char_to_int32(&_parser.tokens[index + 3], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(clean_session != NULL) *clean_session = val;
    
    return ACK_OKAY;
}

int32_t cx_get_mqtt_con(int32_t *status, char *url, int32_t max_size, int32_t *port, int32_t *sec_tag)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT#XMQTTCON?");

    int32_t ack = transfer_pkt(cmd, 16, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);
    
    int32_t index = find_prefix_token("#XMQTTCON", 0);
    if(index < 0) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(val == 0)
    {
        if(status != NULL) *status = val;
        return ACK_OKAY;
    }
    if(_parser.count == 5 && index + 4 < _parser.count) 
    {
        if(status != NULL) *status = val;

        if(url != NULL)
        {
            int32_t len = _parser.tokens[index + 3].len;
            trim_quote(&_parser.tokens[index + 3].buf, &len);
            if(max_size <= len) return -(ACK_ERR_ARG);
            memcpy(url, _parser.tokens[index + 3].buf, len);
            url[len] = '\0';
        }

        ret = char_to_int32(&_parser.tokens[index + 4], &val);
        if(!ret) return -(ACK_ERR_PARSE);

        if(port != NULL) *port = val;

        return ACK_OKAY;
    }

    if(_parser.count == 6 && index + 5 < _parser.count) 
    {
        if(status != NULL) *status = val;

        if(url != NULL)
        {
            int32_t len = _parser.tokens[index + 3].len;
            trim_quote(&_parser.tokens[index + 3].buf, &len);
            if(max_size <= len) return -(ACK_ERR_ARG);
            memcpy(url, _parser.tokens[index + 3].buf, len);
            url[len] = '\0';
        }

        ret = char_to_int32(&_parser.tokens[index + 4], &val);
        if(!ret) return -(ACK_ERR_PARSE);

        if(port != NULL) *port = val;

        ret = char_to_int32(&_parser.tokens[index + 5], &val);
        if(!ret) return -(ACK_ERR_PARSE);

        if(sec_tag != NULL) *sec_tag = val;
    }
    return ACK_OKAY;
}

/* Socket AT commands */
int32_t cx_get_socket(int32_t target_handle, int32_t *family, int32_t *role, int32_t *type)
{
    int32_t val;
    int32_t index = 0;
    char cmd[16];
    bool ret;
    bool is_found = false;

    strcpy(cmd, "AT#XSOCKET?");

    int32_t ack = transfer_pkt(cmd, 32, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 6) return -(ACK_ERR_PARSE);

    while(index < _parser.count - 4)
    {
        index = find_prefix_token("#XSOCKET", index);
        if(index < 0 || index + 4 >= _parser.count) return -(ACK_ERR_PARSE);

        ret = char_to_int32(&_parser.tokens[index + 1], &val);
        if(!ret) return -(ACK_ERR_PARSE);

        if(val == target_handle) 
        {
            is_found = true;
            break;
        }
        index++; 
    }
    if(!is_found) return -(ACK_ERR_PARSE); 

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);
    if(family != NULL) *family = val;

    ret = char_to_int32(&_parser.tokens[index + 3], &val);
    if(!ret) return -(ACK_ERR_PARSE);
    if(role != NULL) *role = val;

    ret = char_to_int32(&_parser.tokens[index + 4], &val);
    if(!ret) return -(ACK_ERR_PARSE);
    if(type != NULL) *type = val;

    return ACK_OKAY;
}

int32_t cx_recv(int32_t handle, int32_t flags, int32_t timeout, uint8_t *data, int32_t max_size)
{
    char cmd[32];

    snprintf(cmd, 32, "AT#XRECV=%d,0,%d,%d", handle, flags, timeout);

    int32_t ack = transfer_pkt(cmd, 5, timeout + 100);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 5) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("#XRECV", 0);
    if(index < 0 || index + 4 >= _parser.count) return -(ACK_ERR_PARSE);

    if(data != NULL)
    {
        uint16_t len = _parser.tokens[index + 4].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy((char *)data, _parser.tokens[index + 4].buf, len);
        return len;
    }

    return ACK_OKAY;
}

int32_t cx_recv_str(int32_t handle, int32_t flags, int32_t timeout, char *data, int32_t max_size)
{
    return cx_recv(handle, flags, timeout, (uint8_t *)data, max_size);
}

int32_t cx_recv_from(int32_t handle, int32_t flags, int32_t timeout, uint8_t *data, int32_t max_size)
{
    char cmd[32];

    snprintf(cmd, 32, "AT#XRECVFROM=%d,0,%d,%d", handle, flags, timeout);

    int32_t ack = transfer_pkt(cmd, 7, timeout + 100);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 7) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("#XRECVFROM", 0);
    if(index < 0 || index + 6 >= _parser.count) return -(ACK_ERR_PARSE);

    if(data != NULL)
    {
        uint16_t len = _parser.tokens[index + 6].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy((char *)data, _parser.tokens[index + 6].buf, len);
        return len;
    }

    return ACK_OKAY;
}

int32_t cx_recv_from_str(int32_t handle, int32_t flags, int32_t timeout, char *data, int32_t max_size)
{
    return cx_recv_from(handle, flags, timeout, (uint8_t *)data, max_size);
}

int32_t cx_get_addr_info(const char *hostname, char *ip_addresses, int32_t max_size, int32_t address_family)
{
    char cmd[64];

    snprintf(cmd, 64, "AT#XGETADDRINFO=\"%s\",%d", hostname, address_family);

    int32_t ack = transfer_pkt(cmd, 2, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("#XGETADDRINFO", 0);
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    if(ip_addresses != NULL)
    {
        int32_t len = _parser.tokens[index + 1].len;
        trim_quote(&_parser.tokens[index + 1].buf, &len);
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(ip_addresses, _parser.tokens[index + 1].buf, len);
        ip_addresses[len] = '\0';
    }

    return ACK_OKAY;
}

/**************************************** Helper Functions ****************************************/

void cx_at_parser_init(void)
{
	memset(&_parser, 0, sizeof(AtParser));
	_parser.at = _pktbuf;
    _parser.resp_code = AT_ERROR;
}

void trim_cr_lf(const char **str)
{
	while ((*str)[0] == CR || (*str)[0] == LF) {
		(*str)++;
	}
}

void trim_left_space(const char **str)
{
    while ((*str)[0] == SPACE) {
        (*str)++;
    }
}

void trim_quote(const char **str, int32_t *len)
{
    if (str == NULL || len == NULL) return;
    if (*str == NULL) return;
    if (*len < 2) return;

    if ((*str)[0] == QUOTE) {
        (*str)++;
        (*len)--;
    }

    if ((*str)[*len - 1] == QUOTE) {
        (*len)--;
    }
}

bool is_resp(const char *str)
{
    trim_cr_lf(&str);

    if(str[0] == NULL_TERMINATOR) return false;

    if (strncmp(str, "OK\r\n", 4) == 0) {
        return true;
    }

    if (strncmp(str, "ERROR\r\n", 7) == 0) {
        return true;
    }

    if (strncmp(str, "+CME ERROR:", 11) == 0) {
        return true;
    }

    if (strncmp(str, "+CMS ERROR:", 11) == 0) {
        return true;
    }

    return false;
}


bool is_end_response(const char *str)
{
    return str[0] == CR && str[1] == LF && str[2] == NULL_TERMINATOR;
}

bool is_end_line(const char *str)
{
	return str[0] == CR && str[1] == LF;
}

bool is_colon_or_comma(const char *str)
{
    return str[0] == COLON || str[0] == COMMA;
}

bool is_quote(const char *str)
{
    return str[0] == QUOTE;
}

int32_t get_urc_type(const char *str, const char *end)
{
    trim_cr_lf(&str);

    if(str[0] == NULL_TERMINATOR) return URC_NONE;

    int32_t remain = (int32_t)(end - str);

    if(remain >= 7 && strncmp(str, "+CEREG:", 7) == 0) return URC_CEREG;
    if(remain >= 10 && strncmp(str, "#XMQTTEVT:", 10) == 0) return URC_MQTTEVT;
    if(remain >= 10 && strncmp(str, "#XMQTTMSG:", 10) == 0) return URC_XMQTTMSG;
    if(remain >= 7 && strncmp(str, "#XGNSS:", 7) == 0) return URC_XGNSS;
    if(remain >= 10 && strncmp(str, "#XGNSSPOS:", 10) == 0) return URC_XGNSSPOS;

    return URC_UNKNOWN;
}

int32_t find_prefix_token(const char *str, int32_t start_idx)
{
    if(str == NULL) return -1;

    int32_t count = _parser.count;

    for(int32_t idx = start_idx; idx < count; idx++)
    {
        if(strncmp(_parser.tokens[idx].buf, str, _parser.tokens[idx].len) == 0) return idx;
    }

    return -1;
}

int32_t get_at_error_ack()
{
    if(_parser.resp_code == AT_ERROR) return ACK_AT_ERROR;
    if(_parser.resp_code == AT_CME_ERROR) return ACK_AT_CME_ERROR;
    if(_parser.resp_code == AT_CMS_ERROR) return ACK_AT_CMS_ERROR;
    return ACK_ERR_PARSE;
}

bool is_empty_token(uint8_t token_index)
{
    return (_parser.tokens[token_index].len == 0) ? true : false;
}

int32_t char_to_hex(char c, uint8_t *x)
{
	if ((c >= '0') && (c <= '9')) {
		*x = c - '0';
	} else if ((c >= 'a') && (c <= 'f')) {
		*x = c - 'a' + 10;
	} else if ((c >= 'A') && (c <= 'F')) {
		*x = c - 'A' + 10;
	} else {
		return -(ACK_ERR_PARSE);
	}

	return 0;
}

// return The length of the binary array, or 0 if an error occurred.
int32_t hex_to_bin(const char *hex, size_t hexlen, uint8_t *buf, size_t buflen)
{
	uint8_t dec;

	if (buflen < (hexlen / 2 + hexlen % 2)) {
		return 0;
	}

	/* if hexlen is uneven, insert leading zero nibble */
	if ((hexlen % 2) != 0) {
		if (char_to_hex(hex[0], &dec) < 0) {
			return 0;
		}
		buf[0] = dec;
		hex++;
		buf++;
	}

	/* regular hex conversion */
	for (size_t i = 0; i < (hexlen / 2); i++) {
		if (char_to_hex(hex[2 * i], &dec) < 0) {
			return 0;
		}
		buf[i] = dec << 4;

		if (char_to_hex(hex[2 * i + 1], &dec) < 0) {
			return 0;
		}
		buf[i] += dec;
	}

	return hexlen / 2 + hexlen % 2;
}

// return The length of the converted string, or 0 if an error occurred.
int32_t bin_to_hex(const uint8_t *buf, int32_t buflen, char *hex, int32_t hexlen)
{
    static const char hex_table[] = "0123456789ABCDEF";

    if(buf == NULL) return 0;
    if(hex == NULL) return 0;
    if(buflen < 0) return 0;
    if(hexlen <= 0) return 0;

	if (hexlen < ((buflen * 2) + 1)) {
		return 0;
	}

    uint8_t value;

    for(int32_t i = 0; i < buflen; i++) 
    {
        value = buf[i];

        hex[i * 2]     = hex_table[(value >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_table[value & 0x0F];
    }

	hex[2 * buflen] = '\0';
	return 2 * buflen;
}

bool char_to_int32_raw(const char *str, int32_t len, int32_t *resp)
{
    if (str == NULL || resp == NULL) return false;
    if (len <= 0) return false;

    int32_t result = 0;
    bool is_negative = false;

    if (*str == '-')
    {
        is_negative = true;
        str++;
        len--;

        if (len <= 0) return false;
    }

    while (len--)
    {
        if (*str >= '0' && *str <= '9')
        {
            result = result * 10 + (*str - '0');
            str++;
        }
        else if(*str == '.') str++;
        else return false;
    }

    *resp = is_negative ? -result : result;

    return true;
}


bool char_to_int32(const AtToken *tok, int32_t *resp)
{
    if(tok == NULL) return false;
    return char_to_int32_raw(tok->buf, tok->len, resp);
}

// send -> recv
// return ack code(negative sign)
int32_t transfer_pkt_raw(const uint8_t *cmd, int32_t cmd_size, int32_t max_tokens, uint32_t timeout_ms)
{
    if(cmd == NULL) return -(ACK_ERR_ARG);
    if(max_tokens > TOKEN_SIZE_MAX) return -(ACK_ERR_ARG);

    if(timeout_ms < 1000) timeout_ms = 1000;

    sleep_ms(10);

    // _parser init
    cx_at_parser_init();

    // flush rx buffer if exist
    uart_flush();

    // send at command
    uart_write(cmd, cmd_size);
    uart_write((const uint8_t *)"\r\n", 2);

    // read at response
    uint32_t rxcnt = 0;    
    bool rxdone = false;
    uint32_t start_ms = get_ms();
    uint32_t idx;

    while((get_ms() - start_ms) <= timeout_ms)
    {
        if(rxdone == true) break;

        rxcnt += uart_read((uint8_t *)&_pktbuf[rxcnt], RESPONSE_DATA_SIZE_MAX - rxcnt);
        if(rxcnt == 0) continue;

        if(rxcnt >= RESPONSE_DATA_SIZE_MAX) return -(ACK_ERR_RESP_OVERFLOW);

        if(_pktbuf[rxcnt-1] == LF)
        {
            if(rxcnt >= 6)
                if(strncmp(&_pktbuf[rxcnt-6], "\r\nOK\r\n", 6) == 0) 
                {
                    _pktbuf[rxcnt] = '\0';
                    _parser.resp_code = AT_OK;
                    rxdone = true;
                    break;
                }
            if(rxcnt >= 9)
                if(strncmp(&_pktbuf[rxcnt-9], "\r\nERROR\r\n", 9) == 0) 
                {
                    _pktbuf[rxcnt] = '\0';
                    _parser.resp_code = AT_ERROR;
                    rxdone = true;
                    break;
                }
            if(rxcnt >= 13) 
            {
                idx = 3;
                while(idx <= rxcnt)
                {
                    if(_pktbuf[rxcnt-idx] == LF) break;
                    if(_pktbuf[rxcnt-idx] == '+') 
                    {
                        if(strncmp(&_pktbuf[rxcnt-idx], "\r\n+CME ERROR:", 13) == 0) 
                        {
                           _pktbuf[rxcnt] = '\0';
                            _parser.resp_code = AT_CME_ERROR;
                            rxdone = true;
                            break;
                        }
                        if(strncmp(&_pktbuf[rxcnt-idx], "\r\n+CMS ERROR:", 13) == 0)
                        {
                            _pktbuf[rxcnt] = '\0';
                            _parser.resp_code = AT_CMS_ERROR;
                            rxdone = true;
                            break;
                        }
                    }
                    idx++;
                }
            }
        }
    }

    if(rxdone == false) return -(ACK_ERR_TIMEOUT);

    if(rxcnt > cmd_size) return at_pkt_parser(max_tokens, cmd, cmd_size);
    return at_pkt_parser(max_tokens, NULL, cmd_size);
}

int32_t transfer_pkt(const char *cmd, int32_t max_tokens, uint32_t timeout_ms)
{
    transfer_pkt_raw((uint8_t *)cmd, strlen(cmd), max_tokens, timeout_ms);
}

// ',' ':' 로 파싱
int32_t at_pkt_parser(int32_t max_tokens, const uint8_t *cmd, int32_t cmd_size)
{
    if(max_tokens <= 0) return -(ACK_ERR_ARG);
    if(max_tokens > TOKEN_SIZE_MAX) return -(ACK_ERR_ARG);

    bool ret;
    uint8_t token_idx = 0;
    int32_t token_len = 0;
    const char *cursor = _parser.at;
    
    trim_cr_lf(&cursor);
    
    // echo trim
    if(cmd != NULL)
    {
        if(memcmp(cursor, cmd, cmd_size) == 0) // memcmp 로 교체
        {
            cursor += cmd_size;
            trim_cr_lf(&cursor);
        }
    }

    while(*cursor != NULL_TERMINATOR)
    {
        token_len = 0;

        // end line
        if(is_resp(cursor))
        {
            if(_parser.resp_code == AT_CME_ERROR || _parser.resp_code == AT_CMS_ERROR)
            {
                cursor += 11;
                trim_left_space(&cursor);
                _parser.tokens[token_idx].buf = cursor;
                while(!is_end_response(cursor)) 
                {
                    cursor++;
                    token_len++;
                }
                _parser.tokens[token_idx].len = token_len;
                _parser.count++;

                // put at error code
                ret = char_to_int32(&_parser.tokens[token_idx], &_parser.at_error_code);
                if(!ret) return -(ACK_ERR_PARSE);
            }
            break;
        }

        if(token_idx >= max_tokens) return -(ACK_ERR_PARSE);
        // token parsing
        _parser.tokens[token_idx].buf = cursor;

        if(token_idx >= max_tokens - 1) // put all
        {
            while(!is_end_line(cursor))
            {
                cursor++;
                token_len++;
            }
        }
        else
        {
            while(!is_colon_or_comma(cursor) && !is_end_line(cursor))
            {
                cursor++;
                token_len++;
            }
        }

        _parser.tokens[token_idx].len = token_len;
        token_idx++;

        _parser.count++;    
        cursor++;

        trim_left_space(&cursor);
        trim_cr_lf(&cursor);
    }

    return ACK_OKAY;
}


int32_t urc_pkt_parser(UrcPacket *urc_packet, const char *end)
{
    int32_t ret = ACK_URC_NONE;
    int32_t ack = ACK_URC_NONE;
    const char *cursor = _urcbuf;

    trim_cr_lf(&cursor);

    while(*cursor != NULL_TERMINATOR)
    {
        ack = ACK_URC_NONE;
        switch(get_urc_type(cursor, end))
        {
            case URC_UNKNOWN:
                while(!is_end_line(cursor)) cursor++;
                break;
            case URC_CEREG: 
                ack = parse_urc_cereg(&urc_packet->cereg, &cursor, end);
                break;
            case URC_MQTTEVT:
                ack = parse_urc_mqtt_evt(&urc_packet->mqtt_evt_type, &urc_packet->mqtt_result, &cursor, end);
                break;
            case URC_XMQTTMSG:
                ack = parse_urc_mqtt_msg(urc_packet->mqtt_topic, urc_packet->mqtt_topic_size, 
                                        urc_packet->mqtt_msg, urc_packet->mqtt_msg_size, &cursor, end);
                break;
            case URC_XGNSS:
                ack = parse_urc_gnss(&urc_packet->gnss_data, &cursor, end);
                break;
            case URC_XGNSSPOS:
                ack = parse_urc_gnss_pos(&urc_packet->gnss_data, &cursor, end);
                break;
            case URC_NONE: return ret;
        }
        if(ack < 0) return -(ACK_ERR_PARSE);
        ret |= ack;
        trim_cr_lf(&cursor);
    }

    return ret;
}

int32_t parse_urc_cereg(int32_t *cereg, const char **cursor, const char *end)
{
    int32_t val, remain;
    bool ret;

    (*cursor) += 7;
    trim_left_space(cursor);

    remain = (int32_t)(end - *cursor);
    if(remain < 1) return -(ACK_ERR_PARSE);

    ret = char_to_int32_raw(*cursor, 1, &val);
    if(!ret) return -(ACK_ERR_PARSE);

    *cereg = val;
    (*cursor)++;

    return ACK_URC_CEREG;
}

int32_t parse_urc_mqtt_evt(int32_t *evt_type, int32_t *result, const char **cursor, const char *end)
{
    int32_t val;
    bool ret;
    int32_t length = 0;

    (*cursor) += 10;
    trim_left_space(cursor);

    while(!is_end_line(*cursor) && (*cursor)[0] != NULL_TERMINATOR)
    {
        if((*cursor)[0] == COMMA)
        {
            ret = char_to_int32_raw((*cursor) - length, length, &val);
            if(!ret) return -(ACK_ERR_PARSE);

            *evt_type = val;
            (*cursor)++;
            length = 0;

            continue;
        }
        (*cursor)++;
        length++;
    }

    ret = char_to_int32_raw((*cursor) - length, length, &val);
    if(!ret) return -(ACK_ERR_PARSE);

    *result = val;

    return ACK_URC_MQTT_EVT;
}

int32_t parse_urc_mqtt_msg(char *mqtt_topic, int32_t mqtt_topic_size, char *mqtt_msg, int32_t mqtt_msg_size, const char **cursor, const char *end)
{
    bool ret;
    int32_t topic_size, msg_size, remain;
    int32_t length = 0;

    (*cursor) += 10;
    trim_left_space(cursor);

    while(!is_end_line(*cursor) && (*cursor)[0] != NULL_TERMINATOR)
    {
        if((*cursor)[0] == COMMA)
        {
            ret = char_to_int32_raw((*cursor) - length, length, &topic_size);
            if(!ret) return -(ACK_ERR_PARSE);
            (*cursor)++;
            length = 0;

            continue;
        }
        (*cursor)++;
        length++;
    }

    ret = char_to_int32_raw((*cursor) - length, length, &msg_size);
    if(!ret) return -(ACK_ERR_PARSE);

    trim_cr_lf(cursor);
    remain = (int32_t)(end - *cursor);
    if(remain < topic_size) return -(ACK_ERR_PARSE);

    if(mqtt_topic != NULL) 
    {
        if(mqtt_topic_size <= topic_size) return -(ACK_ERR_ARG);
        memcpy(mqtt_topic, *cursor, topic_size);
        mqtt_topic[topic_size] = '\0';
    }

    (*cursor) += topic_size;

    trim_cr_lf(cursor);
    remain = (int32_t)(end - *cursor);
    if(remain < msg_size) return -(ACK_ERR_PARSE);

    if(mqtt_msg != NULL) 
    {
        if(mqtt_msg_size <= msg_size) return -(ACK_ERR_ARG);
        memcpy(mqtt_msg, *cursor, msg_size);
        mqtt_msg[msg_size] = '\0';
    }

    (*cursor) += msg_size;

    return ACK_URC_MQTT_MSG;
}

int32_t parse_urc_gnss(GnssData *gnss_data, const char **cursor, const char *end)
{
    bool ret;
    int32_t length = 0;

    (*cursor) += 7;
    trim_left_space(cursor);

    while(!is_end_line(*cursor) && (*cursor)[0] != NULL_TERMINATOR)
    {
        if((*cursor)[0] == COMMA)
        {
            ret = char_to_int32_raw((*cursor) - length, length, &gnss_data->gnss_service);
            if(!ret) return -(ACK_ERR_PARSE);
            (*cursor)++;
            length = 0;

            continue;
        }
        (*cursor)++;
        length++;
    }

    ret = char_to_int32_raw((*cursor) - length, length, &gnss_data->gnss_status);
    if(!ret) return -(ACK_ERR_PARSE);

    return ACK_URC_GNSS_EVT;
}

int32_t parse_urc_gnss_pos(GnssData *gnss_data, const char **cursor, const char *end)
{
    bool ret;
    int32_t length, field_idx;
    int32_t *fields[] = {
        &gnss_data->latitude,
        &gnss_data->longitude,
        &gnss_data->altitude,
        &gnss_data->accuracy,
        &gnss_data->speed,
        &gnss_data->heading
    };

    length = 0;
    field_idx = 0;

    (*cursor) += 10;
    trim_left_space(cursor);

    while(!is_end_line(*cursor) && (*cursor)[0] != NULL_TERMINATOR)
    {
        if((*cursor)[0] == COMMA)
        {
            ret = char_to_int32_raw((*cursor) - length, length, fields[field_idx]);
            if(!ret) return -(ACK_ERR_PARSE);

            (*cursor)++;
            length = 0;
            field_idx++;
            continue;
        }
        (*cursor)++;
        length++;
    }

    const char *datetime_ptr = (*cursor) - length;

    if(field_idx == 6 && length > 0)
    {
        trim_quote(&datetime_ptr, &length);
        memcpy(gnss_data->datetime, datetime_ptr, length);
        gnss_data->datetime[length] = '\0';
    }
    else return -(ACK_ERR_PARSE);

    return ACK_URC_GNSS_DATA;
}
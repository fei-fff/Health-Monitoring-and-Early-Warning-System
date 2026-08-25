#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "app_health.h"


typedef enum
{
    CMD_UNKNOWN = 0,
    CMD_PAGE,
    CMD_TH_TEMP,
    CMD_TH_VDD,
    CMD_LED
} protocol_cmd_t;

typedef struct
{
    protocol_cmd_t cmd;
    int32_t arg;
} protocol_command_t;

int protocol_build_sample(const health_sample_t *sample, uint32_t uptime_ms, char *buf, size_t cap);
int protocol_build_alarm(uint16_t code, uint32_t uptime_ms, int32_t value_x10, char *buf, size_t cap);
int protocol_parse_line(const char *line, protocol_command_t *cmd);
int protocol_build_ack(protocol_cmd_t cmd, uint8_t code, char *buf, size_t cap);

#endif



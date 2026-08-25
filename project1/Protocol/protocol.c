#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int protocol_build_sample(const health_sample_t *sample, uint32_t uptime_ms, char *buf, size_t cap)
{
    int temp_x10 = (int)(sample->mcu_temp_c * 10.0f);

    (void)cap;

    return sprintf(buf, "S,%lu,%d,%u,%lu,%u\r\n",
                   (unsigned long)uptime_ms,
                   temp_x10,
                   (unsigned int)(sample->vdd_mv + 0.5f),
                   (unsigned long)sample->event_count,
                   (unsigned int)sample->alarm_flags);
}

int protocol_build_alarm(uint16_t code, uint32_t uptime_ms, int32_t value_x10, char *buf, size_t cap)
{
    (void)cap;

    return sprintf(buf, "A,%u,%lu,%d\r\n",
                   (unsigned int)code,
                   (unsigned long)uptime_ms,
                   value_x10);
}

int protocol_parse_line(const char *line, protocol_command_t *cmd)
{
    if (cmd == NULL || line == NULL)
    {
        return 0;
    }

    if (strncmp(line, "CMD,PAGE,", 9) == 0)
    {
        cmd->cmd = CMD_PAGE;
        cmd->arg = strtol(line + 9, NULL, 10);
        return 1;
    }

    if (strncmp(line, "CMD,TH,TEMP,", 12) == 0)
    {
        cmd->cmd = CMD_TH_TEMP;
        cmd->arg = strtol(line + 12, NULL, 10);
        return 1;
    }

    if (strncmp(line, "CMD,TH,VDD,", 11) == 0)
    {
        cmd->cmd = CMD_TH_VDD;
        cmd->arg = strtol(line + 11, NULL, 10);
        return 1;
    }

    if (strncmp(line, "CMD,LED,", 8) == 0)
    {
        cmd->cmd = CMD_LED;
        cmd->arg = strtol(line + 8, NULL, 10);
        return 1;
    }

    cmd->cmd = CMD_UNKNOWN;
    return 0;
}

int protocol_build_ack(protocol_cmd_t cmd, uint8_t code, char *buf, size_t cap)
{
    const char *name = "UNKNOWN";

    if (cmd == CMD_PAGE)      name = "PAGE";
    else if (cmd == CMD_TH_TEMP) name = "TH_TEMP";
    else if (cmd == CMD_TH_VDD)  name = "TH_VDD";
    else if (cmd == CMD_LED)     name = "LED";

    (void)cap;

    return sprintf(buf, "ACK,%s,%u\r\n", name, (unsigned int)code);
}






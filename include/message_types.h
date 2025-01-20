#pragma once

typedef enum message_types_t {
    MSG_TYPE_POSITION,
    MSG_TYPE_VELOCITY,
    MSG_TYPE_PRESSURE,
    MSG_TYPE_VERTICAL_ACCEL,
    MSG_TYPE_MAG_FIELD, 
    MSG_TYPE_TEMPERATURE,
    MSG_TYPE_HUMIDITY
} message_types_t;

typedef struct radio_message_t {
    message_types_t type;
    String data;
} radio_message_t;
#ifndef SWIT3_MODBUS_H
#define SWIT3_MODBUS_H

#include <stdint.h>
#include "types.h"

RequestParser modbus_parse;
int16_t modbus_parse(struct Interpreter* ci, struct Request req, char* resp);

#endif
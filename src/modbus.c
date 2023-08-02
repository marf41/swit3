#include "modbus.h"

int16_t modbus_parse(struct Interpreter* ci, struct Request req, char* resp) {
	// printf("\nModbus: "); for (uint16_t i = 0; i < reqlen; i++) { printf("%X ", req[i]); } printf("\n");
	uint16_t first = (req.req[8] << 8) | req.req[9];
	uint16_t value = (req.req[10] << 8) | req.req[11];
	switch (req.req[7]) {
		case 3:
		case 4:
			resp[0] = req.req[0]; resp[1] = req.req[1];
			resp[2] = req.req[2]; resp[3] = req.req[3];
			resp[6] = req.req[6]; resp[7] = req.req[7];
			resp[4] = 0; resp[5] = 3 + 2 * value;
			resp[8] = 2 * value;
			for (uint8_t i = 0; i < value; i++) {
				uint8_t n = i * 2;
				uint16_t v = ci->mem[first + i];
				resp[n + 9] = v >> 8;
				resp[n + 10] = v & 0xFF;
			}
			return 9 + resp[8];
			break;
		case 6:
			ci->mem[first % ci->mem_size] = value;
			for (uint8_t i = 0; i < 12; i++) { resp[i] = req.req[i]; }
			return 12;
			break;
	}
    return 0;
}
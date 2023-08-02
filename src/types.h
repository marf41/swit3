#ifndef SWIT3_TYPES_H
#define SWIT3_TYPES_H

#define UNUSED(x) (void)(x)

#include <stdint.h>
#include <stdbool.h>

typedef int16_t stack_int;

enum func_mode { OUT, NAME, BODY, OVR };
typedef enum func_mode func_mode_t;

enum int_error { ERR_OK, ERR_SKIP_MISS, ERR_MAX };
enum int_warning { WARN_OK, WARN_SCOL_MISS, WARN_MAX };

struct Func {
    uint32_t mem_address;
    uint32_t id;
};

struct Interpreter;

struct Request {
    char* req; // request body
    uint16_t len; // request length
    uint16_t max; // max response length
};

typedef int16_t (RequestParser)(struct Interpreter* ci, struct Request, char* resp);

struct Interpreter {
	stack_int* stack;
	int16_t* mem;
	uint16_t mem_size;
    char* pc;
    uint16_t pcn;
    uint16_t pcn_for;
	char skip;
	char cmd[4];
	uint32_t funcmem[1000];
    struct Func funcs[100];
    uint16_t funcn; // funcmem pointer
    uint16_t funci; // funcs pointer
    bool in_if;
    bool in_case;
    bool in_case_done;
    bool in_for;
    bool echo;
    bool in_var;
    func_mode_t in_func;
    int16_t loops;
    uint32_t errs[ERR_MAX];
    uint32_t warns[WARN_MAX];
    char miss;
    RequestParser* web_parser;
    RequestParser* modbus_parser;
    uint16_t max_depth;
};

#endif
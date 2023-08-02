// #define DBG 1
// Numbers: !, ", #, $, %, &, ', (, ), *, +, ,, -, ., /, 0-9, :, ;, <, =, >, ?
// @ = `, [ = {, [ = }, _ = DEL , ^ = ~, 

#include "int.h"

stack_int* stack;

#ifdef MODBUS
uint8_t mbstart = 0;
#include "modbus.h"
#endif

#ifdef WEB
uint8_t webstart = 0;
uint32_t webreqs = 0;
RequestParser basic_page;
int16_t basic_page(struct Interpreter* ci, struct Request req, char* resp) {
	UNUSED(ci);
	char page[1024] = { 0 };
	uint8_t len = snprintf(page, 1024, "OK\n> %s\n", ci->pc); UNUSED(len);
	web_header(resp, req.max, page, strlen(page));
    return -strlen(resp);
}
#endif

uint16_t f16convert(float v) {
	uint16_t man; int16_t exp;
	uint8_t sign = (v < 0) ? 1 : 0;
	v = fabs(v);
	if (v == 0.0) { exp = 0; man = 0; return F16PACK(sign, exp, man); }
	if (isinf(v) || isnan(v)) { exp = F16EXPMAX; man = 0; return F16PACK(sign, exp, man); }
	int fexp;
	float norm = frexpf(v, &(fexp)); exp = fexp + ((F16EXPMAX - 1) / 2);
	man = (uint16_t)(norm * F16MANMAX);
	// printf("%.3f: %d * %d * 2 ^ %d (%d * %d * 2 ^ %d)\n", v, sign, man, exp, sign, norm, fexp);
	return F16PACK(sign, exp, man);
}
float fconvert(uint16_t v) {
	float f = ((F16SIGN(v) == 0) ? 1 : -1) * powf(2, (F16EXP(v) - 15)) * (F16MAN(v) / 1024.0f);
	return f;
}

void push(int16_t v) { stack[++stack[0]] = v; }
int16_t pop(void) { return (stack[0] > 0) ? stack[stack[0]--] : -1; }
void fpush(float v) { push(f16convert(v)); }
float fpop(void) { return fconvert(pop()); }
void swap(void) { if (stack[0] >= 2) { int16_t a = pop(); int16_t b = pop(); push(a); push(b); } }
void duplicate(void) { push(stack[stack[0]]); }
void over(void) { if (stack[0] >= 2) { push(stack[stack[0]-1]); } }
void drop(void) { if (stack[0] > 0) { stack[0]--; } }
void rotate(void) { // c b a -> b a c
	if (stack[0] >= 3) { 
		int16_t a = pop(); int16_t b = pop(); int16_t c = pop();
		push(b); push(a); push(c);
	}
}
int16_t get(struct Interpreter* ci, uint16_t addr) { return ci->mem[addr % ci->mem_size]; }
void set(struct Interpreter* ci, uint16_t addr, int16_t val) { ci->mem[addr % ci->mem_size] = val; }
void binary(int16_t b) {
	if ((b > 255) || (b < -128)) {
		printf("0b" PRINTF_BINARY_PATTERN_INT16 " ", PRINTF_BYTE_TO_BINARY_INT16(b));
	} else {
		if ((b > 15) || (b < -8)) {
			printf("0b" PRINTF_BINARY_PATTERN_INT8 " ", PRINTF_BYTE_TO_BINARY_INT8(b));
		} else {
			printf("0b" PRINTF_BINARY_PATTERN_INT4 " ", PRINTF_BYTE_TO_BINARY_INT4(b));
		}
	}
}

void parse(struct Interpreter* ci, uint32_t p, int16_t num, uint16_t inmem) {
#ifdef DEBUG
	printf("\nParsing: %s - %d [%d][%d][%d]. Number: %d\n", ci->cmd, p, ci->cmd[0], ci->cmd[1], ci->cmd[2], num);
#endif
	// if ((p == 0) && (num == 0) && (ci->cmd[0] == 0)) { return; }
	if ((p == 0) && (num == 0)) { return; }
	if (p == 0) {
		push(num);
#ifdef DEBUG
		printf("Pushing %d - %d %d %d\n", num, p == 0, num == 0, ci->cmd[0]);
#endif
		return;
}
	int16_t a; int16_t b;
	float fa; float fb;
	int32_t base;
	switch(p) {
		case H3('f', 'o', 'r'): ci->in_for = 1; ci->pcn_for = ci->pcn; break;
		case H3('v', 'a', 'r'): ci->in_var = 1; break;
		case H2('n', 'l'): putchar('\n'); break;
		// ECHO:
		case H2('.', '"'): 	ci->echo = 1; break;
		case H('"'): 		ci->echo = 0; break;
		// CASE:
		// 3 ?: 1 : SKIP ; 2 : SKIP ; 3 : THIS ;:
		case H2('?', ':'): ci->in_case = 1; ci->in_case_done = 0; break; // CASE
		case H2(';', ':'): ci->in_case = 0; pop(); break; // END CASE
		case H2('d', ':'): if (ci->in_case && ci->in_case_done) { ci->skip = ';'; } break; // DEFAULT CASE
		// IF: 1 ? THIS : SKIP ; 0 ? SKIP : THIS ;
		case H('?'): if (pop()) { ci->in_if = 1; } else { ci->skip = ':'; }	break;
		case H(':'):
			if (ci->in_if) { ci->skip = ';'; ci->in_if = 0; return; }
			if (ci->in_case) {
				int16_t v = pop(); duplicate();
				if (v == pop()) { ci->in_case_done = 1; } else { ci->skip = ';'; }
				return;
			}
			ci->in_func = NAME;
			break;
		case H(';'):
#ifdef DEBUG
			putchar(';');
#endif
			if (ci->in_if) { ci->in_if = 0; return; }
			if (ci->in_case && ci->in_case_done) { ci->skip = ';'; return; }
			if (ci->in_for) { int16_t v = pop(); if (v > 0) { push(v-1); ci->pcn = ci->pcn_for; } else { ci->in_for = 0; } return; }
			if (ci->in_func) { ci->in_func = OUT; }
#ifdef DEBUG
			putchar(';');
#endif
			break;
		case H('.'): printf("%d ", pop()); break;
		case H('#'): push(num); break;
		case H2('u', '.'): printf("%u ", (uint16_t)pop()); break;
		case H2('h', '.'): printf("0x%X ", pop()); break;
		case H2('b', '.'): binary(pop()); break;
		case H2('f', '.'): printf("%.3f ", fpop()); break;
		case H2('s', '.'): for(uint8_t i = 1; i <= ci->stack[0]; i++) { printf("%d ", ci->stack[i]); } break;
		case H3('s', 'f', '.'): for(uint8_t i = 1; i <= ci->stack[0]; i++) { printf("%.3f ", fconvert(ci->stack[i])); } break;
		case H('+'): push(pop() + pop()); break;
		case H2('f', '+'): fpush(fpop() + fpop()); break;
		case H('-'): if (num != 0) { push(-num); } else { a = pop(); b = pop(); push(b - a); } break;
		case H2('f', '-'): a = fpop(); b = fpop(); fpush(b - a); break;
		case H('/'): a = pop(); b = pop(); push(b / a); break;
		case H2('f', '/'): fa = fpop(); fb = fpop(); fpush(fb / fa); break;
		case H('*'): push(pop() * pop()); break;
		case H2('f', '*'): fpush(fpop() * fpop()); break;
		case H('~'): push(~pop()); break;
		case H2('f', '>'): push(f16convert((float)pop())); break; // convert from int to F16
		case H2('f', '<'): push(round(fconvert(pop()))); break; // convert from F16 to int
		case H('>'): a = pop(); b = pop(); push((b > a) ? 1 : 0); break;
		case H('<'): a = pop(); b = pop(); push((b < a) ? 1 : 0); break;
		case H2('>', '='): a = pop(); b = pop(); push((b >= a) ? 1 : 0); break;
		case H2('<', '='): a = pop(); b = pop(); push((b <= a) ? 1 : 0); break;
		case H2('<', '>'): a = pop(); b = pop(); push((b != a) ? 1 : 0); break;
		case H2('=', '='): a = pop(); b = pop(); push((b == a) ? 1 : 0); break;
		case H('='): a = pop(); push((a == num)? 1 : 0); break; // =0
		case H('D'): push(get(ci, num)); break;
		case H('b'): b = num; a = 0; base = 1; while(b) { a += (b % 10) * base; base *= 2; b /= 10; } push(a); break;
		case H('h'): b = num; a = 0; base = 1; while(b) { a += (b % 10) * base; base *= 16; b /= 10; } push(a); break;
		case H('('): ci->skip = ')'; break;
		case H(')'): break;
		case H('|'): push(pop() | pop()); break;
		case H('&'): push(pop() & pop()); break;
		case H2('|', '|'): push(pop() || pop()); break;
		case H2('&', '&'): push(pop() && pop()); break;
		case H2('<', '<'): a = pop(); b = pop(); push(b << a); break;
		case H2('>', '>'): a = pop(); b = pop(); push(b >> a); break;
		case H3('s','w','p'): swap(); break;
		case H3('d', 'u', 'p'): duplicate(); break;
		case H3('o', 'v', 'r'): over(); break;
		case H3('r', 'o', 't'): rotate(); break;
		case H3('d', 'r', 'p'): drop(); break;
		case H3('s', 'e', 't'): a = pop(); b = pop(); set(ci, a, b); break;
		case H3('g', 'e', 't'): push(get(ci, pop())); break;
		case H3('m', 'e', 'm'): putchar('|'); putchar(' '); for (uint8_t i = 0; i < 10; i++) { printf("%d: %d | ", i, ci->mem[i]); } putchar('\n'); break;
		case H3('n', 'e', 'g'): push(-pop()); break; // negate value
		case H3('a', 'b', 's'): push(abs(pop())); break;
		case H2('m', 's'): base = pop() * 1000; usleep(base > 0 ? base : 0); fflush(stdout); break;
		case H3('i', 'n', 'f'): ci->pcn = 0; break;
		case H2('g', 'o'): a = pop(); if (a >= 0) { ci->pcn = a; } fflush(stdout); break;
		case H2('p', 'c'): push(ci->pcn); break;
		case H2(':', ';'): ci->in_func = OUT; break;
#ifdef WEB
		case H3('w', 'e', 'b'): if (web_setup(1, pop()) == 0) { webstart = 1; } break;
#endif
#ifdef MODBUS
		case H3('m', 'b', 's'): if(web_setup(2, pop()) == 0) { mbstart = 1; } break;
#endif
		case H(0): puts(" NOP "); break;
		default:
			if (ci->in_func == NAME) {
				ci->funcs[++ci->funci].id = p;
				ci->funcs[ci->funci].mem_address = ci->funcn + 1;
				ci->in_func = BODY;
				return;
			}
			for (uint8_t i = 0; i <= ci->funci; i++) {
				if (ci->funcs[i].id == p) {
					if (!inmem) {
						printf("[function '%s'] ", ci->cmd);
					} else {
						printf("[function depth %d] ", inmem);
					}
					uint8_t fn = ci->funcs[i].mem_address;
					if (ci->max_depth == 0) { ci->max_depth = 1000; }
					if (inmem > ci->max_depth) { return; }
					while ((ci->funcmem[fn] != H2(':', ';')) && (fn <= ci->funcn)) { parse(ci, ci->funcmem[fn], ci->funcmem[fn+1], inmem + 1); fn += 2; }
					return;
				}
			}
			printf("Unknown command '%s' (%d) @ %d [%d][%d][%d].\n", ci->cmd, num, p, ci->cmd[0], ci->cmd[1], ci->cmd[2]); break;
	}
#ifdef DBG
	if (skip > 0) { printf("Skipping to: %c\n", skip); }
#endif
}

int8_t interpret(struct Interpreter *ci) {
#ifdef WEB
	if (ci->web_parser == NULL) { ci->web_parser = basic_page; }
#endif
#ifdef MODBUS
	if (ci->modbus_parser == NULL) { ci->modbus_parser = modbus_parse; }
#endif
	uint32_t num = 0;
	uint32_t p = 0;
	char c = 0;
	uint8_t hn = 0;
	bool had_num = 0;
	int8_t cn = 0;
	stack = ci->stack;
	ci->pcn = 0;
	while ((c = ci->pc[ci->pcn++])) {
		// putchar(c); // DEBUG
		if (WSP(c)) {
			if (ci->in_func == BODY) {
				// if (ci->in_func == BODY) {
				if (had_num && (p == 0)) { p = H('#'); }
				ci->funcmem[++ci->funcn] = p; ci->funcmem[++ci->funcn] = num; if (p == H2(':', ';')) { ci->in_func = OUT; }
				// }
			} else {
				if (ci->in_var) {
					push(p);
					ci->in_var = 0;
				} else {
					if (had_num && (p == 0)) { // p = H('#'); }
						push(num);
					} else {
						parse(ci, p, num, 0);
					}
				}
			}
			c = ci->pc[ci->pcn-1];
			p = 0; num = 0; cn = 0; had_num = 0;
			ci->cmd[0] = 0; ci->cmd[1] = 0; ci->cmd[2] = 0; ci->cmd[3] = 0;
			while (WSP(c)) { c = ci->pc[ci->pcn++]; }
		}
		if (ci->skip) { while (c && (c != ci->skip)) { c = ci->pc[ci->pcn++]; } ci->skip = 0; }
		if (ci->echo) { while (c && (c != '"')) { putchar(c); c = ci->pc[ci->pcn++]; } }
		hn = 0;
		if (NUM(c)) {
			int8_t a = PRS(c) - 13;
			if ((a >= 3) && (a <= 12)) {
				num = (num * 10) + (a - 3);
				hn = 1;
				had_num = 1;
			}
		}
		if ((hn == 0) && (cn < 3)) {
			p = (p << 6) | PRS(c);
			ci->cmd[cn++] = c;
		}
#ifdef WEB
		if (webstart) { int8_t reqs = web_loop(ci, 1, ci->web_parser); webreqs += reqs; }
#endif
#ifdef MODBUS
		if (mbstart) { web_loop(ci, 2, ci->modbus_parser); }
#endif
	}
	parse(ci, p, num, 0);
	// putchar('\t');
	if (ci->skip != 0) { ci->errs[ERR_SKIP_MISS]++; ci->miss = ci->skip; ci->skip = 0; }
	if (ci->echo) { ci->errs[ERR_SKIP_MISS]++; ci->miss = '"'; ci->echo = 0; }
	if (ci->in_if) { ci->errs[ERR_SKIP_MISS]++; ci->miss = ':'; ci->in_if = 0; }
	if (ci->in_case) { ci->errs[ERR_SKIP_MISS]++; ci->miss = ';'; ci->in_case = 0; } //;:
	if (ci->in_for) { ci->errs[ERR_SKIP_MISS]++; ci->miss = ';'; ci->in_for = 0; }
	if (ci->in_var) {}

	if (p != H(';')) { ci->warns[WARN_SCOL_MISS]++; }
	if (ci->loops > 0) { ci->loops--; }

	return 0;
}

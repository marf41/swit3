// #define DBG 1
// Numbers: !, ", #, $, %, &, ', (, ), *, +, ,, -, ., /, 0-9, :, ;, <, =, >, ?
// @ = `, [ = {, [ = }, _ = DEL , ^ = ~, 

#include "main.h"
#ifdef WEB
#include "web.c"
#endif

char skip = 0;
int16_t stack[16] = { 0 };
uint8_t sn = 0;
int16_t mem[MEM_SIZE] = { 0 };
char cmd[4] = { 0, 0, 0, 0 };

uint32_t funcmem[1000] = { 0 }; // array of hashes, functions ends with ;
uint32_t funcids[100] = { 0 }; // hash of function name
uint16_t funcpts[100] = { 0 }; // position in array
uint16_t funcn = 0; // funcmem pointer
uint8_t funci = 0; // funcids & funcpts pointer

#ifdef MODBUS
uint8_t mbstart = 0;
RequestParser modbus_parse;
int16_t modbus_parse(char* req, uint16_t reqlen, char* resp, uint16_t maxlen) {
	// printf("\nModbus: "); for (uint16_t i = 0; i < reqlen; i++) { printf("%X ", req[i]); } printf("\n");
	uint16_t first = (req[8] << 8) | req[9];
	uint16_t value = (req[10] << 8) | req[11];
	switch (req[7]) {
		case 3:
		case 4:
			resp[0] = req[0]; resp[1] = req[1];
			resp[2] = req[2]; resp[3] = req[3];
			resp[6] = req[6]; resp[7] = req[7];
			resp[4] = 0; resp[5] = 3 + 2 * value;
			resp[8] = 2 * value;
			for (uint8_t i = 0; i < value; i++) {
				uint8_t n = i * 2;
				uint16_t v = mem[first + i];
				resp[n + 9] = v >> 8;
				resp[n + 10] = v & 0xFF;
			}
			return 9 + resp[8];
			break;
		case 6:
			mem[first % MEM_SIZE] = value;
			for (uint8_t i = 0; i < 12; i++) { resp[i] = req[i]; }
			return 12;
			break;
	}
}
#endif

#ifdef WEB
uint8_t webstart = 0;
uint32_t webreqs = 0;
RequestParser web_page;
int16_t web_page(char* req, uint16_t reqlen, char* resp, uint16_t maxlen) {
	char page[1024] = { 0 };
	uint8_t len = snprintf(page, 1024, "Are you OK?\n");
	for (uint8_t i = 0; i < 100; i++) {
		char buf[255] = { 0 };
		uint8_t rlen = snprintf(buf, 255, "%d: %d\n", i, mem[i]);
		strcat(page, buf);
		len += rlen;
	}
	web_header(resp, maxlen, page, strlen(page));
	// printf("%s %d\n", resp, strlen(resp));
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

void push(int16_t v) { stack[++sn] = v; }
int16_t pop(void) { return (sn > 0) ? stack[sn--] : -1; }
void fpush(float v) { push(f16convert(v)); }
float fpop(void) { return fconvert(pop()); }
void swap(void) { if (sn >= 2) { int16_t a = pop(); int16_t b = pop(); push(a); push(b); } }
void duplicate(void) { push(stack[sn]); }
void over(void) { if (sn >= 2) { push(stack[sn-1]); } }
void drop(void) { if (sn > 0) { sn--; } }
void rotate(void) { // c b a -> b a c
	if (sn >= 3) { 
		int16_t a = pop(); int16_t b = pop(); int16_t c = pop();
		push(b); push(a); push(c);
	}
}
int16_t get(uint16_t addr) { return mem[addr % 1000]; }
void set(uint16_t addr, int16_t val) { mem[addr % 1000] = val; }
void binary(uint16_t b) {
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

uint8_t inif = 0;
uint8_t incase = 0;
uint8_t incasedone = 0;
uint8_t infor = 0;
uint8_t echo = 0;
uint8_t invar = 0;
uint8_t infunc = 0;

char* pc;
uint8_t pcn = 0;
uint8_t pcnfor = 0;

void parse(uint32_t p, int16_t num, uint8_t inmem) {
#ifdef DBG
	printf("\nParsing: %d. Number: %d\n", p, num);
#endif
	if (p == 0) { push(num); return; }
	int16_t a; int16_t b;
	float fa; float fb;
	uint32_t base;
	switch(p) {
		case H3('f', 'o', 'r'): infor = 1; pcnfor = pcn; break;
		case H3('v', 'a', 'r'): invar = 1; break;
		case H2('n', 'l'): putchar('\n'); break;
		// ECHO:
		case H2('.', '"'): echo = 1; break;
		case H('"'): echo = 0; break;
		// CASE:
		// 3 ?: 1 : SKIP ; 2 : SKIP ; 3 : THIS ;:
		case H2('?', ':'): incase = 1; incasedone = 0; break; // CASE
		case H2(';', ':'): incase = 0; pop(); break; // END CASE
		case H2('d', ':'): if (incasedone) { skip = ';'; } break; // DEFAULT CASE
		// IF: 1 ? THIS : SKIP ; 0 ? SKIP : THIS ;
		case H('?'): if (pop()) { inif = 1; } else { skip = ':'; }	break;
		case H(':'):
			if (inif) { skip = ';'; inif = 0; return; }
			if (incase) {
				int16_t v = pop(); duplicate();
				if (v == pop()) { incasedone = 1; } else { skip = ';'; }
				return;
			}
			infunc = 1;
			break;
		case H(';'):
			if (incase && incasedone) { skip = ';'; return; }
			if (infor) { int16_t v = pop(); if (v > 0) { push(v-1); pcn = pcnfor; } else { infor = 0; } return; }
			if (infunc) { infunc = 0; }
			break;
		case H('.'): printf("%d ", pop()); break;
		case H('#'): push(num); break;
		case H2('u', '.'): printf("%u ", (uint16_t)pop()); break;
		case H2('h', '.'): printf("0x%X ", pop()); break;
		case H2('b', '.'): binary(pop()); break;
		case H2('f', '.'): printf("%.3f ", fpop()); break;
		case H2('s', '.'): for(uint8_t i = 1; i <= sn; i++) { printf("%d ", stack[i]); } break;
		case H3('s', 'f', '.'): for(uint8_t i = 1; i <= sn; i++) { printf("%.3f ", fconvert(stack[i])); } break;
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
		case H('D'): push(get(num)); break;
		case H('b'): b = num; a = 0; base = 1; while(b) { a += (b % 10) * base; base *= 2; b /= 10; } push(a); break;
		case H('h'): b = num; a = 0; base = 1; while(b) { a += (b % 10) * base; base *= 16; b /= 10; } push(a); break;
		case H('('): skip = ')'; break;
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
		case H3('s', 'e', 't'): a = pop(); b = pop(); set(a, b); break;
		case H3('g', 'e', 't'): push(get(pop())); break;
		case H3('m', 'e', 'm'): putchar('|'); putchar(' '); for (uint8_t i = 0; i < 10; i++) { printf("%d: %d | ", i, mem[i]); } putchar('\n'); break;
		case H3('n', 'e', 'g'): push(-pop()); break; // negate value
		case H3('a', 'b', 's'): push(abs(pop())); break;
		case H2('m', 's'): base = pop() * 1000; usleep(base); fflush(stdout); break;
		case H3('i', 'n', 'f'): pcn = 0; break;
		case H2('g', 'o'): a = pop(); if (a >= 0) { pcn = a; } fflush(stdout); break;
		case H2('p', 'c'): push(pcn); break;
#ifdef WEB
		case H3('w', 'e', 'b'): if (web_setup(1, pop()) == 0) { webstart = 1; } break;
#endif
#ifdef MODBUS
		case H3('m', 'b', 's'): if(web_setup(2, pop()) == 0) { mbstart = 1; } break;
#endif
		case H(0): puts(" NOP "); break;
		default:
			if (!inmem) {
				for (uint8_t i = 0; i <= funci; i++) {
					if (funcids[i] == p) {
						printf("[function '%s'] ", cmd);
						uint8_t fn = funcpts[i];
						while ((funcmem[fn] != H(';')) && (fn <= funcn)) { parse(funcmem[fn], funcmem[fn+1], 1); fn += 2; }
						return;
					}
				}
				printf("Unknown command '%s' (%d) @ %d.\n", cmd, num, p); break;
			}
	}
#ifdef DBG
	if (skip > 0) { printf("Skipping to: %c\n", skip); }
#endif
}

int main(int argc, char** argv) {
	if (argc > 1) {
		SAVE_START_TIME();
		printf("> %s\n", argv[1]);
		// uint8_t n = 0;
		char c = 0;
		uint8_t cn = 0;
		uint32_t p = 0;
		uint16_t num = 0;
		// int8_t neg = 1;
		pc = argv[1];
		while((c = pc[pcn++])) {
			if (WSP(c)) {
				if (infunc) {
					if (infunc == 2) { funcmem[++funcn] = p; funcmem[++funcn] = num; if (p == H(';')) { infunc = 0; } }
					if (infunc == 1) { funcids[++funci] = p; funcpts[funci] = funcn + 1; infunc++; }
				} else {
					if (invar) { push(p); invar = 0; } else { parse(p, num, 0); }
				}
				c = pc[pcn-1]; // printf("PC%d ", pcn);
				p = 0;
				cn = 0;
				num = 0;
				// neg = 1;
				cmd[0] = 0;
				cmd[1] = 0;
				cmd[2] = 0;
				cmd[3] = 0;
				while(WSP(c)) { c = pc[pcn++]; }
			}
			if (skip) { while(c != skip) { c = pc[pcn++]; } skip = 0; }
			if (echo) {	while(c != '"') { putchar(c); c = pc[pcn++]; } }
			// printf(">%c< ", c);
			uint8_t hn = 0;
			if (NUM(c)) {
				int8_t a = PRS(c) - 13;
				if ((a >= 3) && (a <= 12)) {
#ifdef DBG
					printf("Number: %d. Adding: %d\n", num * neg, a - 3);
#endif
					num = (num * 10) + (a - 3);
					hn = 1;
				}
				// if (a == 0) { neg = neg * -1; }
			}
			if ((hn == 0) && (cn < 3)) {
				p = (p << 6) | PRS(c);
				cmd[cn++] = c;
#ifdef DBG
				printf("Adding %c, step %d, now %d.\n", c, cn, p);
#endif
			}
#ifdef WEB
			if (webstart) { int8_t reqs = web_loop(1, web_page); webreqs += reqs; }
#endif
#ifdef MODBUS
			if (mbstart) { web_loop(2, modbus_parse); }
#endif
		}
		parse(p, num, 0); // parse last elements
		putchar('\t');
		if (skip != 0) { printf("[ERR] Missing terminating '%c'.\n", skip); }
		if (p != H(';')) { printf("[WARN] Missing ';' at program end (%c).\n", cmd[0]); }
		PRINT_TIME_FROM_START();
#ifdef DBG
		printf("Stack: ");
		for (uint8_t i = 0; i <= sn; i++) {
			printf("%d @ %d, ", stack[i], i);
		}
		putchar('\n');
#endif
		// printf("%d %d %d %d\n", Hs("This"), H2s("is"), H1s("a"), Hs("test"));
	} else {
		for (uint8_t i = 0; i < 127; i++) {
			printf("%3d %c: lower %c, hash %2d, midhash %2d, white %d, number %d\n", i, i, i | 32, PRS(i), PRS2(i), WSP(i), NUM(i));
		}
	}
	return 0;
}

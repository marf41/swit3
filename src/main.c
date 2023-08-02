#include <string.h>

#include "int.h"
#include "web.h"

int16_t stacks[16][16];

int16_t mem[1000];

RequestParser web_page;
int16_t web_page(struct Interpreter* ci, struct Request req, char* resp) {
	char page[1024] = { 0 };
	uint8_t len = snprintf(page, 1024, "Are you OK?\n");
	for (uint8_t i = 0; i < 100; i++) {
		char buf[255] = { 0 };
		uint8_t rlen = snprintf(buf, 255, "%d: %d\n", i, ci->mem[i]);
		strncat(page, buf, 255);
		len += rlen;
	}
	web_header(resp, req.max, page, strlen(page));
	// printf("%s %d\n", resp, strlen(resp));
    return -strlen(resp);
}

int main(int argc, char** argv) {
    struct Interpreter ci = { 0 };
    ci.stack = stacks[0];
    ci.mem = mem;
    ci.mem_size = 1000;
    ci.web_parser = web_page;
	if (argc > 1) {
		SAVE_START_TIME();
		printf("> %s\n", argv[1]);
        ci.pc = argv[1];
		interpret(&ci);
		if (ci.errs[ERR_SKIP_MISS]) { printf("\t[ERR] Missing terminating '%c'.\n", ci.miss); }
		if (ci.warns[WARN_SCOL_MISS]) { printf("\t[WARN] Missing ';' at program end (%c).\n", ci.cmd[0]); }
		putchar('\t'); PRINT_TIME_FROM_START();
#ifdef DBG
		printf("Stack: ");
		for (uint8_t i = 0; i <= sn; i++) {
			printf("%d @ %d, ", stack[i], i);
		}
		putchar('\n');
#endif
		// printf("%d %d %d %d\n", Hs("This"), H2s("is"), H1s("a"), Hs("test"));
	} else {
        bool repl = 1;
        char buf[250];
        char pc[255];
        while (repl) {
            putchar('>'); putchar(' ');
            if (fgets(buf, 240, stdin)) {
                snprintf(pc, 255, "%s ;", buf);
                ci.pc = pc;
                interpret(&ci);
                printf("Stack: ");
                for (uint8_t i = 1; i <= stacks[0][0]; i++) { printf("%d ", stacks[0][i]); }
                printf(". Function memory: %d %d\n", ci.funcn, ci.funci);
            }
        }
        /*
		for (uint8_t i = 0; i < 127; i++) {
			printf("%3d %c: lower %c, hash %2d, midhash %2d, white %d, number %d\n", i, i, i | 32, PRS(i), PRS2(i), WSP(i), NUM(i));
		}
        */
	}
	return 0;
}

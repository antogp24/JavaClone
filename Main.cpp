#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string>
#include <memory>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

#include "FolderReader.h"
#include "AstPrinter.h"
#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Color.h"

namespace JavaError {
	bool had_error;
	bool had_runtime_error;
};

bool REPL = false;

static void run_dir(char* name);
static void run_file(char *name);
static void run_repl();

int main(int argc, char** argv) {
	JavaError::had_error = false;
	JavaError::had_runtime_error = false;

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	if (argc - 1 > 1) {
		printf("Usage: javaclone <directory>");
		return 1;
	}
	else if (argc - 1 == 1) {
		run_dir(argv[1]);
	}
	else {
		// REPL = true;
		run_repl();
	}

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtDumpMemoryLeaks();
	return 0;
}

static void run_dir(char* name) {
	FolderReader folder(name);

	for (char *str : folder.get_listings()) {
		if (str == NULL || str[0] == '.') {
			continue;
		}

		std::string path(name);
		path.append("/").append(str);

		if (JavaError::had_error) {
			break;
		}

		run_file((char *)path.c_str());
		printf("\n");
	}
}

static void run_file(char *name) {
	printf("Running file: %s\n", name);

	FILE* file = fopen(name, "r");
	assert(file != NULL);

	fseek(file, 0, SEEK_END);
	uint64_t len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* src = (char*)malloc(len * sizeof(char));
	assert(src != NULL);
	memset(src, 0, len);
	fread(src, sizeof(char), len, file);

	Lexer lexer(src, len);
	const std::vector<Token> &tokens = lexer.scan();

	if (!JavaError::had_error && REPL) lexer.print_tokens();

	free(src);
	fclose(file);
}

static void run_repl() {
	Interpreter interpreter = {};

	while (true) {
		JavaError::had_error = false;
		JavaError::had_runtime_error = false;

		printf(COLOR_RED"java> ");
		printf(COLOR_END);

		char prompt[1024] = {0};
		fgets(prompt, sizeof(prompt), stdin);
		size_t len = strlen(prompt);
		if (len == 1) goto done;

		std::string src(prompt);

		Lexer lexer((char*)src.c_str(), len);
		const std::vector<Token> &tokens = lexer.scan();
		if (!JavaError::had_error && REPL) lexer.print_tokens();

		if (JavaError::had_error) { continue; }

		Parser parser(tokens);
		std::vector<Stmt*>* statements = parser.parse_statements();

		if (JavaError::had_error) {
			parser.statements_free(statements);
			continue;
		}

		interpreter.interpret(statements);
		parser.statements_free(statements);
	}
done:;
}
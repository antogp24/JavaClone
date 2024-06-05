#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string>

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
Keywords *keywords = nullptr;

static void run_dir(char* name);
static void run_file(char *name);
static void run_repl();

int main(int argc, char** argv) {
	JavaError::had_error = false;
	JavaError::had_runtime_error = false;

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	{
		// This bullshit is to avoid a false positive
		// from crtdbg that a global std::unordered_map
		// causes a memory leak, when in reality it calls
		// the destructor after dumping the memory leaks.
		// This scope forces it to call the destructor before.
		Keywords local_keywords;
		keywords = &local_keywords;
		lexer_fill_keywords();

		if (argc - 1 > 1) {
			printf("Usage: javaclone <directory>");
			return 1;
		}
		else if (argc - 1 == 1) {
			run_dir(argv[1]);
		}
		else {
			run_repl();
		}

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

	if (!JavaError::had_error) lexer.print_tokens();

	free(src);
	fclose(file);
}

static void run_repl() {
	while (true) {
		JavaError::had_error = false;
		JavaError::had_runtime_error = false;

		printf(COLOR_RED"java> ");
		printf(COLOR_END);

		char prompt[1024] = {0};
		fgets(prompt, sizeof(prompt), stdin);
		size_t len = strlen(prompt);
		if (len == 1) goto done;

		char* src = (char*)malloc((len + 1) * sizeof(char));
		assert(src != NULL);
		strncpy(src, prompt, len + 1);

		Lexer lexer(src, len);
		const std::vector<Token> &tokens = lexer.scan();
		if (!JavaError::had_error) lexer.print_tokens();

		if (JavaError::had_error) { free(src); continue; }

		Parser parser(tokens);
		Expr *expression = parser.parse_expression();

		if (JavaError::had_error) { free(src); continue; }

		AstPrinter::println("Ast Expr: ", expression);
		Interpreter interpreter = {};
		interpreter.interpret(expression);
		expression_free(expression);

		free(src);
	}
done:;
}
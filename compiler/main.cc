#include "lexer.hh"
#include "logger.hh"
#include "source.hh"
#include "syntax.hh"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *g_ProgramName;

static void PreprocessFile(const char* filePath) {
    SourceFile sourceFile{ filePath };
    if (!sourceFile.IsOpen) {
        Log(LL_FATAL, "cannot open %s", filePath);
        return;
    }

    Lexer lexer{ &sourceFile };

    SyntaxToken t{ };
    do {
        t = lexer.ReadTokenDirect();
    }
    while (t.GetKind() != SK_EOF_TOKEN);
}

int main(int argc, char** argv) {
    g_ProgramName = argv[0];

    if (argc == 1) {
        printf("compiler 0.1-snapshot\n");
        printf("\
Usage: %s [options] file...\n\
Options:\n\
", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i{ 1 }; i < argc; ++i) {
        PreprocessFile(argv[i]);
    }

    return g_ErrorsLogged ? EXIT_FAILURE : EXIT_SUCCESS;
}

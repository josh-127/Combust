#include "common.h"
#include "lexer.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

char *g_ProgramName;

static void init_globals(int argc, char **argv)
{
    g_ProgramName = argv[0];
}

#define PROCESS_COMPILE_ASSEMBLE_LINK 0
#define PROCESS_COMPILE_ASSEMBLE      1
#define PROCESS_COMPILE               2
#define PROCESS_PREPROCESS            3

static int g_fileProcess = PROCESS_COMPILE_ASSEMBLE_LINK;

static void preprocess_files(int optind, int argc, char **argv)
{
    for (; optind < argc; ++optind) {
        SOURCE_FILE sourceFile = OpenSourceFile(argv[optind]);
        LEXER *lexer = CreateLexer(&sourceFile);
        TOKEN t = ReadTokenDirect(lexer);
        while (t.Kind != TK_EOF)
            t = ReadTokenDirect(lexer);
        DeleteLexer(lexer);
        CloseSourceFile(&sourceFile);
    }
}

static void process_files(int optind, int argc, char **argv)
{
    if (g_fileProcess == PROCESS_PREPROCESS)
        preprocess_files(optind, argc, argv);
    else
        LogFatal("not implemented yet");
}

#define OPTION_VERSION 0
#define OPTION_HELP    1

int main(int argc, char** argv)
{
    static struct option longOptions[] = {
        { "version", no_argument, 0, OPTION_VERSION },
        { "help",    no_argument, 0, OPTION_HELP },
        { 0 }
    };

    int c;

    init_globals(argc, argv);

    while ((c = getopt_long(argc, argv, "ESc", longOptions, 0)) != -1) {
        switch (c) {
        case 'E':
            g_fileProcess = PROCESS_PREPROCESS;
            break;

        case 'S':
            g_fileProcess = PROCESS_COMPILE;
            break;

        case 'c':
            g_fileProcess = PROCESS_COMPILE_ASSEMBLE;
            break;

        case OPTION_VERSION:
            printf("mycc 0.1-snapshot\n");
            return EXIT_FAILURE;

        case OPTION_HELP:
            printf("\
Usage: %s [options] file...\n\
Options:\n\
    --help          Display this information.\n\
    --version       Display compiler version information.\n\
    -E              Preprocess only; do not compile, assemble, or link.\n\
    -S              Compile only; do not assemble or link.\n\
    -c              Compile and assemble, but do not link.\n\
", argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (optind == argc) {
        LogFatal("no input files");
        fprintf(stderr, "compilation terminated\n");
    }
    else {
        process_files(optind, argc, argv);
    }

    return g_ErrorsLogged ? EXIT_FAILURE : EXIT_SUCCESS;
}

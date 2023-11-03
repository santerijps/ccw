#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ccw.h"
#include "util.h"

#define GCC_STRIPPED "-s -fno-ident -fno-asynchronous-unwind-tables "
#define GCC_WARNINGS "-Wall -Wextra "
#define GCC_ERRORS "-Wall -Wextra -Werror "
#define GCC_STD "-std=c2x "

#define DEFAULT_COMPILER_OPTIONS GCC_STD GCC_STRIPPED GCC_WARNINGS "-Oz "
#define RELEASE_COMPILER_OPTIONS GCC_STD GCC_STRIPPED GCC_ERRORS "-Ofast "

int main(int argc, char **argv) {
  char compiler_options[1024] = {0};
  char source_files[512] = {0};
  char out_file_name[256] = {0};
  char out_file_path[256] = "./out.exe";
  char format_command[512] = {0};
  char run_file_path[256] = {0};
  char run_args[512] = {0};

  int format = 0;
  int run = 0;

  for (int i = 1; i < argc; i++) {

    if (!strcmp(argv[i], "-o") && i + 1 < argc) {
      memset(out_file_path, 0, sizeof(char) * 256);
      strcpy(out_file_path, argv[i + 1]);
    }

    else if (!strcmp(argv[i], "-run")) {
      run = 1;
      for (int j = i + 1; j < argc; j++) {
        strcat(run_args, argv[j]);
        if (j < argc - 1) {
          strcat(run_args, " ");
        }
      }
      break;
    }

    else if (!strcmp(argv[i], "-defaults")) {
      strcat(compiler_options, DEFAULT_COMPILER_OPTIONS);
      continue;
    }

    else if (!strcmp(argv[i], "-release")) {
      strcat(compiler_options, RELEASE_COMPILER_OPTIONS);
      continue;
    }

    else if (!strcmp(argv[i], "-format")) {
      format = 1;
      continue;
    }

    else if (StringEndsWith(argv[i], ".c")) {
      strcat(source_files, argv[i]);
      if (i < argc - 1) {
        strcat(source_files, " ");
      }
    }

    strcat(compiler_options, argv[i]);
    if (i < argc - 1) {
      strcat(compiler_options, " ");
    }

  }

  strcpy(out_file_name, basename(out_file_path));

  if (format) {
    CreateClangFormatCommand(format_command, source_files);
  }

  if (run) {
    if (!StringContainsSlash(out_file_path)) {
      char buffer[256] = {0};
      sprintf(buffer, "./%s", out_file_path);
      strcpy(run_file_path, buffer);
    }
    else {
      strcpy(run_file_path, out_file_path);
    }
    char run_command[512] = {0};
    sprintf(run_command, "%s %s", run_file_path, run_args);
    CompileAndRunLoop(out_file_name, compiler_options, format_command, run_command);
  }

  else {
    CompileLoop(out_file_name, compiler_options, format_command);
  }

  return 0;
}

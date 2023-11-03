#include "util.h"

#ifndef _LIBGEN_H_
#include <libgen.h>
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

inline int StringContainsSlash(const char *s) {
  for (size_t i = 0; s[i] != '\0' ; i++) {
    if (s[i] == '/' || s[i] == '\\') {
      return 1;
    }
  }
  return 0;
}

inline int StringEndsWith(const char *s1, const char *s2) {
  const size_t s1_length = strlen(s1);
  const size_t s2_length = strlen(s2);
  if (s1_length < s2_length) {
    return 0;
  }
  return !strcmp(s1 + (s1_length - s2_length), s2);
}

inline void SubString(char *buffer, const char *s, int start, int stop) {
  for (int i = start; i < stop; i++) {
    *buffer++ = s[i];
  }
}

inline void CreateClangFormatCommand(char *buffer, const char *s) {
  char dir_paths[8][64] = {0};
  int dir_count = 0;
  for (size_t i = 0, j = 0; s[i] != '\0' ; i++) {
    if (s[i] == ' ') {
      char file_name[128] = {0};
      SubString(file_name, s, j, i);
      char *dir_path = dirname(file_name);
      j = i + 1;
      for (int k = 0; k < dir_count; k++) {
        if (!strcmp(dir_paths[k], dir_path)) {
          goto DirPathAlreadyExists;
        }
      }
      strcpy(dir_paths[dir_count], dir_path);
      dir_count += 1;
      DirPathAlreadyExists:
    }
  }
  strcat(buffer, "clang-format -i --style=Google");
  for (int i = 0; i < dir_count; i++) {
    char format[256] = {0};
    sprintf(format, " %s/*.c", dir_paths[i]);
    strcat(buffer, format);
  }
}

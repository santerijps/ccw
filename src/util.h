#ifndef UTIL_H
#define UTIL_H

int StringContainsSlash(const char *s);
int StringEndsWith(const char *s1, const char *s2);
void SubString(char *buffer, const char *s, int start, int stop);
void CreateClangFormatCommand(char *buffer, const char *s);

#endif
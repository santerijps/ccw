#ifndef CCW_H
#define CCW_H

int FileWasWrittenTo(const char *dir_path, const char *out_file_name);
void WaitFor(unsigned long ms);
void CompileLoop(const char *out_file_name, const char *compiler_options, const char *format_command);
void CompileAndRunLoop(const char *out_file_name, const char *compiler_options, const char *format_command, const char *run_command);

#endif

// stdlib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// windows
#include <windows.h>
#include <tlhelp32.h>

#define CHANGE_POLL_INTERVAL_MS 200
#define SEARCH_SIZE 512
#define FILES_COUNT 1024
#define COMMAND_SIZE 1024
#define SHELL "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe"

DWORD FILE_WRITES[FILES_COUNT] = {0};
size_t FILE_INDEX = 0;

STARTUPINFO SI;
PROCESS_INFORMATION PI;

int CheckDirectory(const char *dir_path, const char *out_file_name);
void FillWriteTimes(const char *dir_path, const char *out_file_name);
int RunCompiler(const char *options);
void RunProgram(const char *run_command);
void KillProgram(void);
void RunClangFormat(const char *format_command);

int FileWasWrittenTo(const char *dir_path, const char *out_file_name) {
  static int initial_run = 1;
  if (initial_run) {
    FillWriteTimes(dir_path, out_file_name);
    initial_run = 0;
  }
  FILE_INDEX = 0;
  return CheckDirectory(dir_path, out_file_name);
}

void FillWriteTimes(const char *dir_path, const char *out_file_name) {
  char search[SEARCH_SIZE] = {0};
  HANDLE fh = NULL;
  WIN32_FIND_DATA fd;
  sprintf(search, "%s\\*.*", dir_path);
  fh = FindFirstFile(search, &fd);
  if (fh == INVALID_HANDLE_VALUE) {
    return;
  }
  while (1) {
    if (!FindNextFile(fh, &fd)) {
      break;
    }
    if (
      !strcmp(fd.cFileName, ".") ||
      !strcmp(fd.cFileName, "..") ||
      !strcmp(fd.cFileName, out_file_name)
    ) {
      continue;
    }
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      sprintf(search, "%s\\%s", dir_path, fd.cFileName);
      FillWriteTimes(search, out_file_name);
    }
    else {
      FILE_WRITES[FILE_INDEX] = fd.ftLastWriteTime.dwLowDateTime;
      FILE_INDEX += 1;
    }
  }
  FindClose(fh);
}

int CheckDirectory(const char *dir_path, const char *out_file_name) {
  char search[SEARCH_SIZE] = {0};
  HANDLE fh = NULL;
  WIN32_FIND_DATA fd;

  sprintf(search, "%s\\*.*", dir_path);
  fh = FindFirstFile(search, &fd);

  if (fh == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Invalid handle value!");
    return 0;
  }

  while (1) {

    if (!FindNextFile(fh, &fd)) {
      break;
    }

    if (
      !strcmp(fd.cFileName, ".") ||
      !strcmp(fd.cFileName, "..") ||
      !strcmp(fd.cFileName, out_file_name)
    ) {
      continue;
    }

    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      sprintf(search, "%s\\%s", dir_path, fd.cFileName);
      if (CheckDirectory(search, out_file_name)) {
        return 1;
      }
    }

    else {
      if (FILE_WRITES[FILE_INDEX] != fd.ftLastWriteTime.dwLowDateTime) {
        FILE_WRITES[FILE_INDEX] = fd.ftLastWriteTime.dwLowDateTime;
        return 1;
      }
      FILE_INDEX += 1;
    }
  }

  FindClose(fh);
  return 0;
}

int RunCompiler(const char *options) {
  char command[COMMAND_SIZE] = {0};
  sprintf(command, "gcc %s", options);
  fprintf(stderr, "%s\n", command);
  return system(command);
}

void RunProgram(const char *run_command) {
  ZeroMemory(&SI, sizeof(SI));
  ZeroMemory(&PI, sizeof(PI));
  SI.cb = sizeof(SI);
  char command[COMMAND_SIZE] = {0};
  sprintf(command, "%s /c %s", SHELL, run_command);
  DWORD flags = NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT;
  if (!CreateProcess(SHELL, command, NULL, NULL, FALSE, flags, NULL, NULL, &SI, &PI)) {
    fprintf(stderr, "Error creating process: %ld\n", GetLastError());
  }
}

void KillProgram(void) {
  PROCESSENTRY32 pe;
  HANDLE hSnap, hChildProc;
  ZeroMemory(&pe, sizeof(PROCESSENTRY32));
  pe.dwSize = sizeof(PROCESSENTRY32);
  hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (Process32First(hSnap, &pe)) while (TRUE) {
    if (pe.th32ParentProcessID == PI.dwProcessId) {
      hChildProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
      if (hChildProc) {
        TerminateProcess(hChildProc, 1);
        CloseHandle(hChildProc);
      }
    }
    if (!Process32Next(hSnap, &pe)) {
      break;
    }
  }
  TerminateThread(PI.hThread, 1);
  TerminateProcess(PI.hProcess, 1);
  CloseHandle(PI.hThread);
  CloseHandle(PI.hProcess);
}

void WaitFor(unsigned long ms) {
  Sleep(ms);
}

void CompileLoop(
  const char *out_file_name,
  const char *compiler_options,
  const char *format_command
) {
  system("cls");
  RunClangFormat(format_command);
  RunCompiler(compiler_options);
  while (1) {
    if (FileWasWrittenTo(".", out_file_name)) {
      system("cls");
      RunClangFormat(format_command);
      // FillWriteTimes(".", out_file_name);
      RunCompiler(compiler_options);
    }
    WaitFor(CHANGE_POLL_INTERVAL_MS);
  }
}

void CompileAndRunLoop(
  const char *out_file_name,
  const char *compiler_options,
  const char *format_command,
  const char *run_command
) {
  system("cls");
  RunClangFormat(format_command);
  if (RunCompiler(compiler_options) == 0) {
    RunProgram(run_command);
  }
  while (1) {
    if (FileWasWrittenTo(".", out_file_name)) {
      KillProgram();
      system("cls");
      RunClangFormat(format_command);
      // FillWriteTimes(".", out_file_name);
      if (RunCompiler(compiler_options) == 0) {
        RunProgram(run_command);
      }
    }
    WaitFor(CHANGE_POLL_INTERVAL_MS);
  }
}

inline void RunClangFormat(const char *format_command) {
  if (format_command[0] != '\0') {
    fprintf(stderr, "%s\n", format_command);
    system(format_command);
  }
}

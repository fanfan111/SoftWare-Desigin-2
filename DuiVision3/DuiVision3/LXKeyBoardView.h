#pragma once

#define VT_RECORD_DIR  _T("C:\\KeyboardSecurityTool\\VTRecord")
#define R0_RECORD_DIR  _T("C:\\KeyboardSecurityTool\\R0Record")
#define R3_RECORD_DIR  _T("C:\\KeyboardSecurityTool\\R3Record")

#define VT_RECORD_FILE  "C:\\KeyboardSecurityTool\\VTRecord\\Record.txt"
#define R0_RECORD_FILE  "C:\\KeyboardSecurityTool\\R0Record\\Record.txt"
#define R3_RECORD_FILE  "C:\\KeyboardSecurityTool\\R3Record\\Record.txt"

typedef void (WINAPI* KEY_CALLBACK)(BYTE, LPCWSTR);

VOID startVTRecord();
VOID stopVTRecord();

VOID startR3Record();
VOID stopR3Record();

VOID startR0Record();
VOID stopR0Record();
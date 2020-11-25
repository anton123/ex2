#pragma once
typedef struct
{
	char* path_to_input;
	char* path_to_output;
	int start_i;
	int finish_i;
	int key;
} THREAD_params_t;

DWORD WINAPI CaesarThread(LPVOID lpParam);

#define BUFFER_SIZE  200
#define MAX_LINES 200
#define ERROR_CODE ((int)(-1))
#define SUCCESS_CODE ((int)(0))
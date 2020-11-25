// Math_Thread.c

// Includes --------------------------------------------------------------------

#include <windows.h>
#include "Caesar.h"

// Function Definitions --------------------------------------------------------

DWORD WINAPI CaesarThread(LPVOID lpParam)
{
	THREAD_params_t* p_params;

	/* Check if lpParam is NULL */
	if (NULL == lpParam)
	{
		return -1;
	}

	/*
	* Convert (void *) to parameters type.
	* In this example, MATH_THREAD_params_t is a simple struct.
	* In general, it could be any type:
	* integer, array, array of struct, string etc.
	*/
	p_params = (THREAD_params_t*)lpParam;

    errno_t retval;
    HANDLE hFile;
    HANDLE hFile_out;
    DWORD  dwBytesRead = 0;
    DWORD  dwBytesWrite = 0;
    WCHAR ReadBuffer[BUFFER_SIZE] = { 0 };
    OVERLAPPED ol = { 0 };



    hFile = CreateFileA(p_params->path_to_input,               // file to open
        GENERIC_READ,          // open for reading
        FILE_SHARE_READ,       // share for reading
        NULL,                  // default security
        OPEN_EXISTING,         // existing file only
        FILE_ATTRIBUTE_NORMAL, // normal file
        NULL);                 // no attr. template

    if (hFile == ERROR_CODE)
    {
        printf("Terminal failure: unable to open file \n");
        return;
    }

    SetFilePointer(
        hFile,
        p_params->start_i,
        NULL,
        FILE_BEGIN
    );

    if (FALSE == ReadFile(hFile, ReadBuffer, (p_params->finish_i - p_params->start_i), &dwBytesRead, NULL))
    {
        printf("Terminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
        free(ReadBuffer);
        CloseHandle(hFile);
        return;
    }


    int buffer_length = strlen(ReadBuffer);
    decrypt(ReadBuffer, p_params->key, buffer_length);

    char* fileout = p_params->path_to_output;
    hFile_out = CreateFileA(fileout,               // file to write
        GENERIC_WRITE,                             // open for writing
        0,                                         // share for writing
        NULL,                                      // default security
        CREATE_ALWAYS,                               // existing file only
        FILE_ATTRIBUTE_NORMAL,                     // normal file
        NULL);                                     // no attr. template

    if (hFile_out == ERROR_CODE)
    {
        printf("Terminal failure: unable to open file \n");
        return;
    }



    if (FALSE == WriteFile(hFile_out, ReadBuffer, buffer_length, &dwBytesWrite, NULL))
    {
        printf("Terminal failure: Unable to write to file.\n GetLastError=%08x\n", GetLastError());
        free(ReadBuffer);
        CloseHandle(hFile_out);
        return;
    }

    CloseHandle(hFile_out);
    CloseHandle(hFile);
}


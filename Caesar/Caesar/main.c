#include <stdbool.h>
#include <stdio.h>
#include <windows.h>




static const int STATUS_CODE_FAILURE = -1;

void decrypt(char* Buffer, int key)
{
    int length;
    length = strlen(Buffer);
    for (int i = 0; i < length; i++)
    {
        if ((Buffer[i] >= 48) && (Buffer[i] <= 57))
        {
            Buffer[i] = '0' + (Buffer[i] - '0' - key) % 10;
        }
        else if ((Buffer[i] >= 65) && (Buffer[i] <= 90))
            Buffer[i] = 'A' + (Buffer[i] - 'A' - key) % 26;
        else if ((Buffer[i] >= 97) && (Buffer[i] <= 122))
            Buffer[i] = 'a' + (Buffer[i] - 'a' - key) % 26;
        else
            continue;
    }
}

int main(int argc, char* argv[])
{
    errno_t retval;
    HANDLE hFile;
    HANDLE hFile_out;
    DWORD  dwBytesRead = 0;
    DWORD  dwBytesWrite = 0;
    OVERLAPPED ol = { 0 };
    FILE* p_stream = NULL;
    int key = argv[2];
    if (argc != 3)
    {
        printf("Usage Error: Incorrect number of arguments\n\n");
        return;
    }

    // Open file to get length
    retval = fopen_s(&p_stream, argv[1], "r");
    if (0 != retval)
    {
        printf("Failed to open file.\n");
        return STATUS_CODE_FAILURE;
    }
    
    fseek(p_stream, 0, SEEK_END);
    size_t length = ftell(p_stream);
    retval = fclose(p_stream);
    if (0 != retval)
    {
        printf("Failed to close file.\n");
        return STATUS_CODE_FAILURE;
    }

    char ReadBuffer = (char*)malloc(length*sizeof(char));
    if(NULL == ReadBuffer)
    {
        return STATUS_CODE_FAILURE;
    }

    hFile = CreateFileA(argv[1],               // file to open
        GENERIC_READ,          // open for reading
        FILE_SHARE_READ,       // share for reading
        NULL,                  // default security
        OPEN_EXISTING,         // existing file only
        FILE_ATTRIBUTE_NORMAL, // normal file
        NULL);                 // no attr. template

    if (hFile == STATUS_CODE_FAILURE)
    {
        printf("Terminal failure: unable to open file \n");
        return;
    }

    // Read one character less than the buffer size to save room for
    // the terminating NULL character. 

    if (FALSE == ReadFile(hFile, ReadBuffer, length, &dwBytesRead,NULL))
    {
        printf("Terminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
        free(ReadBuffer);
        CloseHandle(hFile);
        return;
    }

    decrypt(ReadBuffer, key);

    char fileout = "decrypted.txt";
    hFile_out = CreateFileA(fileout,               // file to open
        GENERIC_WRITE,          // open for reading
        FILE_SHARE_WRITE,       // share for reading
        NULL,                  // default security
        OPEN_ALWAYS,         // existing file only
        FILE_ATTRIBUTE_NORMAL, // normal file
        NULL);                 // no attr. template

    if (hFile_out == STATUS_CODE_FAILURE)
    {
        printf("Terminal failure: unable to open file \n");
        return;
    }

    if (FALSE == WriteFile(hFile_out, ReadBuffer, length, &dwBytesWrite, NULL))
    {
        printf("Terminal failure: Unable to write to file.\n GetLastError=%08x\n", GetLastError());
        free(ReadBuffer);
        CloseHandle(hFile_out);
        return;
    }

    CloseHandle(hFile_out);
    CloseHandle(hFile);
}

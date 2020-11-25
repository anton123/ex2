#include <stdbool.h>
#include <stdio.h>
#include <windows.h>
#include "Caesar.h"

#define BUFFER_SIZE  200
#define MAX_LINES 200
#define ERROR_CODE ((int)(-1))
#define SUCCESS_CODE ((int)(0))


static const int STATUS_CODE_FAILURE = -1;
 

static HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine,
    LPVOID p_thread_parameters,
    LPDWORD p_thread_id)
{
    HANDLE thread_handle;

    if (NULL == p_start_routine)
    {
        printf("Error when creating a thread");
        printf("Received null pointer");
        exit(ERROR_CODE);
    }

    if (NULL == p_thread_id)
    {
        printf("Error when creating a thread");
        printf("Received null pointer");
        exit(ERROR_CODE);
    }

    thread_handle = CreateThread(
        NULL,                /*  default security attributes */
        0,                   /*  use default stack size */
        p_start_routine,     /*  thread function */
        p_thread_parameters, /*  argument to thread function */
        0,                   /*  use default creation flags */
        p_thread_id);        /*  returns the thread identifier */

    return thread_handle;
}




void decrypt(char* Buffer, int key, int length)
{
    int move_index = 0;
    for (int i = 0; i < length; i++)
    {
        if ((Buffer[i] >= 48) && (Buffer[i] <= 57)) // number 0-9
        {
            move_index = (Buffer[i] - '0' - key) % 10;
            if (move_index > 0)
                Buffer[i] = '0' + move_index;
            else if (move_index < 0)
                Buffer[i] = '9' + 1 + move_index;
            else
                Buffer[i] = '0';
        }
        else if ((Buffer[i] >= 65) && (Buffer[i] <= 90)) // A-Z
        {
            move_index = (Buffer[i] - 'A' - key) % 26;
            if (move_index > 0)
                Buffer[i] = 'A' + move_index;
            else if (move_index < 0)
                Buffer[i] = 'Z' + 1 + move_index;
            else
                Buffer[i] = 'A';
        }
        else if ((Buffer[i] >= 97) && (Buffer[i] <= 122)) // a-z
        {
            move_index = (Buffer[i] - 'a' - key) % 26;
            if (move_index > 0)
                Buffer[i] = 'a' + move_index;
            else if (move_index < 0)
                Buffer[i] = 'z' + 1 + move_index;
            else
                Buffer[i] = 'a';
        }
        else
            continue;
    }
    return length;
}



int main(int argc, char* argv[])
{

    errno_t retval;
    FILE* p_stream = NULL;
    int lines_i_arr[MAX_LINES] = { 0 };
    int curr_i=0,curr_c=0,lines_i=1, num_of_threads, lines_per_thread, leftover_lines;
    DWORD thread_id;
    THREAD_params_t* p_thread_params;
    DWORD p_thread_ids;
    HANDLE* thread_handle;
    char* ptr;

    int key = strtol(argv[2], &ptr, 10);

    if (argc != 4)
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
    while (p_stream != NULL)
    {
        curr_c = fgetc(p_stream);
        if (curr_c == '\n') {
            curr_i++;
            lines_i_arr[lines_i] = curr_i;
            lines_i++;
        }
        else if (curr_c == EOF)
            break;
        else 
        {
            curr_i++;
            continue;
        }

    }
    
    retval = fclose(p_stream);
    if (0 != retval)
    {
        printf("Failed to close file.\n");
        return STATUS_CODE_FAILURE;
    }
    num_of_threads = atoi(argv[3]);
    lines_per_thread = lines_i / num_of_threads;
    leftover_lines = lines_i % num_of_threads;
    

    p_thread_params = (THREAD_params_t *)malloc(sizeof(THREAD_params_t)*num_of_threads);
    if (NULL == p_thread_params)
    {
        printf("Error when allocating memory");
        return ERROR_CODE;
    }
    p_thread_ids = (DWORD*)malloc(sizeof(DWORD) * num_of_threads);
    if (NULL == p_thread_ids)
    {
        printf("Error when allocating memory");
        return ERROR_CODE;
    }

    thread_handle = (HANDLE*)malloc(sizeof(HANDLE) * num_of_threads);
    if (NULL == thread_handle)
    {
        printf("Error when allocating memory");
        return ERROR_CODE;
    }

    for (int i = 0; i < num_of_threads; i++) //The loop calls to threads
    {
        if (lines_per_thread == 0) // In this case, we have more threads than lines
        {
            if (leftover_lines > 0) { // is there still a line left for the current thread, if yes- take it and update the number of leftover lines
                p_thread_params[i].path_to_input = argv[1];
                p_thread_params[i].path_to_output = "decrypted.txt";
                p_thread_params[i].start_i = lines_i_arr[i];
                p_thread_params[i].finish_i = lines_i_arr[i + 1];
                p_thread_params[i].key = key;
                thread_handle[i] = CreateThreadSimple(CaesarThread, p_thread_params[i], &thread_id[i]);
                
                leftover_lines--;
            }
            else // if there no more lines, we call for a thread and pass the start and end as '0', its is handaled in the call_to_thread func.
                p_thread_params[i].path_to_input = argv[1];
            p_thread_params[i].path_to_output = "decrypted.txt";
            p_thread_params[i].start_i = 0;
            p_thread_params[i].finish_i = 0;
            p_thread_params[i].key = key;
            thread_handle[i] = CreateThreadSimple(CaesarThread, p_thread_params[i], &thread_id[i]);

            leftover_lines--;
        }
        else // In this case, we have enough lines for the number of threads
        {
            if (leftover_lines > 0) { // if we have lefotver lines, we give an extra line to first threads
                p_thread_params[i].path_to_input = argv[1];
                p_thread_params[i].path_to_output = "decrypted.txt";
                p_thread_params[i].start_i = lines_i_arr[i * lines_per_thread];
                p_thread_params[i].finish_i = lines_i_arr[(i + 1) * lines_per_thread + 1];
                p_thread_params[i].key = key;
                thread_handle[i] = CreateThreadSimple(CaesarThread, p_thread_params[i], &thread_id[i]);
                leftover_lines--;
            }
            else
            {
                p_thread_params[i].path_to_input = argv[1];
                p_thread_params[i].path_to_output = "decrypted.txt";
                p_thread_params[i].start_i = lines_i_arr[i * lines_per_thread];
                p_thread_params[i].finish_i = lines_i_arr[(i + 1) * lines_per_thread];
                p_thread_params[i].key = key;
                thread_handle[i] = CreateThreadSimple(CaesarThread, p_thread_params[i], &thread_id[i]);
            }
        }

    }
    
    
    
    /*
    *  NEED TO ADD - Thread termination, WaitForMultipleObjects
    */
}

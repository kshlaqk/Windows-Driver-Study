#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>

#define CDO_SYB_NAME L"\\\\.\\my_test_sybname123"

int main()
{
    HANDLE device = NULL;
    DWORD moudle = 0;
    char buffer[0x20] = { 0 };
    DWORD retLen = 0;

    printf("you want read or write?\r\n");
    printf("1: Write\r\n");
    printf("2: Read\r\n");
    
    scanf("%d", &moudle);

    if (moudle == 1)
    {
        system("cls");
        printf("plz input a string:\r\n");
        getchar();

        // ʹ�� fgets ��ȫ�ض�ȡ����
        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            // �Ƴ����ܴ��ڵĻ��з�
            buffer[strcspn(buffer, "\n")] = 0;
        }

        //���ļ�һ��������豸
        device = CreateFile(CDO_SYB_NAME, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
        if (device == INVALID_HANDLE_VALUE)
        {
            printf("Open device failed.\r\n");
            return -1;
        }
        else
        {
            printf("Open device successfully.\r\n");

            WriteFile(device, buffer, strlen(buffer) + 1, &retLen, NULL);

            system("pause");

            CloseHandle(device);
            return 0;
        }
    }
    else if (moudle == 2)
    {
        system("cls");
        //���ļ�һ��������豸
        device = CreateFile(CDO_SYB_NAME, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
        if (device == INVALID_HANDLE_VALUE)
        {
            printf("Open device failed.\r\n");
            return -1;
        }
        else
        {
            printf("Open device successfully.\r\n");

            ReadFile(device, buffer, strlen(buffer) + 1, &retLen, NULL);

            printf("get str: %s\r\n", buffer);

            system("pause");

            CloseHandle(device);
            return 0;
        }
    }
    
    printf("parm error\r\n");

    return 0;
}
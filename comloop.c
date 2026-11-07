#define INITGUID
#include "comloop.h"
#include "./WorkLib/StrLib.h"
#include "./WorkLib/cJson.h"
#include <ctype.h>
#include <signal.h>

#define stdin_clean                                 \
    do {                                            \
        int c;                                      \
        while ((c = getchar()) != '\n' && c != EOF) \
            continue;                               \
    } while (0)

// 串口信息
HANDLE hSerial = INVALID_HANDLE_VALUE;
DWORD baudRate = 115200;
BYTE byteSize = 8;
BYTE stopBits = ONESTOPBIT;
BYTE parity = NOPARITY;
char portName[30] = "COM3";

// 异步操作所需的重叠结构
OVERLAPPED osRead = {0};
OVERLAPPED osWrite = {0};
// 增加 volatile 保证多线程下的可见性
volatile BOOL g_KeepRunning = TRUE;

// 切换到 UTF-8 模式（用于对接 Linux）
void SetTerminalUTF8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

// 切换到 GBK 模式（用于显示程序自身的中文提示）
void SetTerminalGBK() {
    SetConsoleOutputCP(936); // 936 是 GBK
    SetConsoleCP(936);
}

// 信号处理函数
void SignalHandler(int signum) {
    g_KeepRunning = FALSE;
    return;
}

/**
 * 检查字符串是否表示一个合法的整数（支持可选的正负号）
 */
int is_valid_integer(const char *str) {
    if (str == NULL || *str == '\0')
        return 0;

    if (*str == '+' || *str == '-')
        str++;

    if (*str == '\0')
        return 0;

    while (*str) {
        if (!isdigit((unsigned char)*str))
            return 0;
        str++;
    }
    return 1;
}

int getUserInt(long int default_value) {
    char input[100];
    int value;

    while (1) {
        memset(&input, 0, ARR_SIZE(input));
        value = 0;
        printf("please int number ( %ld ) : ", default_value);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("read error\n");
            return 1;
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n')
            input[len - 1] = '\0';

        if (strlen(input) == 0) {
            value = default_value;
        } else if (is_valid_integer(input)) {
            value = atoi(input);
        } else {
            printf("input error：'%s' \n", input);
            continue;
        }
        break;
    }
    return value;
}

void ListAvailablePorts() {
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return;

    SP_DEVINFO_DATA DeviceInfoData;
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
        TCHAR deviceName[256];
        if (SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)deviceName, sizeof(deviceName), NULL)) {
            _tprintf(_T("devices: %s\n"), deviceName);
        }
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
}

void ConfigureSerialPort() {
    printf("\n--- tty set menu ---\n");
    printf("please tty number (key COM3): ");
    char userInput[10];
    scanf("%s", userInput);
    stdin_clean;
    snprintf(portName, sizeof(portName), "\\\\.\\%s", userInput);

    printf("Input bandRate (key 115200)\n");
    baudRate = (DWORD)getUserInt(115200);

    int choice = 8;
    printf("byte Size (5-8)\n");
    choice = getUserInt(8);
    byteSize = (choice >= 5 && choice <= 8) ? (BYTE)choice : 8;

    printf("1:1\n2:1.5\n3:2\nstop bit\n");
    choice = getUserInt(1);
    stopBits = (choice == 2) ? ONE5STOPBITS : (choice == 3 ? TWOSTOPBITS : ONESTOPBIT);

    printf("1:none\n2:odd\n3:even\ncheck bit\n");
    choice = getUserInt(1);
    parity = (choice == 2) ? ODDPARITY : (choice == 3 ? EVENPARITY : NOPARITY);

    printf("\nprot:%s\n", userInput);
    printf("band Rate:%ld\n", baudRate);
    printf("byte Size:%d\n", byteSize);
    printf("stop Bit:%d\n", stopBits + 1);
    printf("check bit:%d\n", choice);
    printf("continue the Enter any key:");
    getch();
}

BOOL OpenSerialPort() {
    hSerial = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error: An error occurred while opening this serial port %s\n, number of error: %lu\n", portName, GetLastError());
        return FALSE;
    }

    // 初始化异步事件句柄
    osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    SetCommTimeouts(hSerial, &timeouts);

    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    GetCommState(hSerial, &dcb);
    dcb.BaudRate = baudRate;
    dcb.ByteSize = byteSize;
    dcb.StopBits = stopBits;
    dcb.Parity = parity;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(hSerial, &dcb)) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    printf("\nserial %s ok\n", portName);
    return TRUE;
}

bool isRecvJson = false;

// 线程函数：接收串口数据
DWORD WINAPI ReadSerialThread(LPVOID lpParam) {
    char buffer[1024];
    DWORD bytesRead;
    BOOL fWaitingOnRead = FALSE;

    while (g_KeepRunning) {
        if (!fWaitingOnRead) {
            if (!ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, &osRead)) {
                if (GetLastError() != ERROR_IO_PENDING) {
                    printf("\n[Error] read eeror for comx\n");
                    break;
                }
                fWaitingOnRead = TRUE;
            } else if (bytesRead > 0) {
                fwrite(buffer, 1, bytesRead, stdout);
                fflush(stdout);
                // 收到数据
                isRecvJson = true;
            }
        } else {
            DWORD wait = WaitForSingleObject(osRead.hEvent, 100);
            if (wait == WAIT_OBJECT_0) {
                if (GetOverlappedResult(hSerial, &osRead, &bytesRead, FALSE)) {
                    if (bytesRead > 0) {
                        fwrite(buffer, 1, bytesRead, stdout);
                        fflush(stdout);
                        // 收到数据
                        isRecvJson = true;
                    }
                } else if (GetLastError() != ERROR_IO_PENDING) {
                    break;
                }
                fWaitingOnRead = FALSE;
                ResetEvent(osRead.hEvent);
            }
        }
    }
    return 0;
}

void InteractiveMode() {
    printf("\n--- Press Ctrl+A then Ctrl+C to exit ---\n\n");

    HANDLE hThread = CreateThread(NULL, 0, ReadSerialThread, NULL, 0, NULL);
    SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);

    DWORD dwWritten;
    char buffer[1024] = {0};
    DWORD bytesRead;

    strnew SendBuf = NEW_NAME(buffer);
    newRootJsonObject(SendBuf);
    addJsonItemData(SendBuf, "DevId:\"992701\"");
    addJsonItemData(SendBuf, "Cmd:\"%s\"", "Read");
    newSubArrayJsonItem(SendBuf, "DataArr", {
        addJsonItemData(SendBuf, "DegSpeed");
        addJsonItemData(SendBuf, "CarLen");
        addJsonItemData(SendBuf, "NowHinder");
    });
    addCsToJsonAndPushJsonStr(newJsonObjectByString(SendBuf));
    catString(SendBuf.Name._char, "\n", SendBuf.MaxLen, 1);
    bytesRead = strlen(buffer);

    while (g_KeepRunning) {
        if (!isRecvJson) {
            continue;
        }
        isRecvJson = false;
        // 核心修复：正确处理异步写入
        if (!WriteFile(hSerial, buffer, bytesRead, &dwWritten, &osWrite)) {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                // 异步挂起，需等待写入完成
                WaitForSingleObject(osWrite.hEvent, INFINITE);
                GetOverlappedResult(hSerial, &osWrite, &dwWritten, FALSE);
            } else {
                printf("\n[Error] write error for comx, code: %lu\n", err);
                break;
            }
        }
        // 重置事件状态，为下次写入做准备
        ResetEvent(osWrite.hEvent);
    }

    g_KeepRunning = FALSE; // 确保读线程也能退出
    if (hThread != NULL) {
        if (WaitForSingleObject(hThread, 500) == WAIT_TIMEOUT) {
            TerminateThread(hThread, 0);
        }
        CloseHandle(hThread);
    }
    if (osRead.hEvent != NULL) {
        CloseHandle(osRead.hEvent);
    }
    if (osWrite.hEvent != NULL) {
        CloseHandle(osWrite.hEvent);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, SignalHandler);
    if (argc != 3) {
        printf("%s row col\n", argv[0]);
    }
    do {
        ListAvailablePorts();
        printf("===== windows tty for Terminal =====\n");
        ConfigureSerialPort();
        printf("\n\nyou can input the cmd of shell when you login shell");
        if (argc == 3) {
            printf("\n\033[1;31m stty rows %s cols %s\n \033[0m", argv[1], argv[2]);
        } else {
            printf("\n\033[1;31m stty rows <num1> cols <num2>\n \033[0m");
        }
        printf("\nIf your system supports xterm");
        printf("\nplease enter the following command after logging into the shell");
        printf("\n\033[1;31m export TERM=xterm\n \033[0m");
    } while (!OpenSerialPort());

    SetTerminalUTF8();
    InteractiveMode();
    SetTerminalGBK();

    if (hSerial != INVALID_HANDLE_VALUE)
        CloseHandle(hSerial);
    return 0;
}

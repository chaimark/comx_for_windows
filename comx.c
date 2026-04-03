#define INITGUID
#include "comx.h"
#include "./C_MyLib/JsonCheckFun.h"
#include "./C_MyLib/JsonDataAnalyzeLib.h"
#include "./C_MyLib/JsonSetLib.h"
#include "./C_MyLib/StrLib.h"
#include <signal.h>

// 全局变量
HANDLE hSerial = INVALID_HANDLE_VALUE;
DWORD  baudRate = 1500000;
BYTE   byteSize = 8;
BYTE   stopBits = ONESTOPBIT;
BYTE   parity = NOPARITY;
char   portName[30] = "COM3";

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
    if (signum == SIGINT) {
        g_KeepRunning = FALSE;
    }
}

void clsInputSpace(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

// 线程函数：接收串口数据 (异步增强版)
DWORD WINAPI ReadSerialThread(LPVOID lpParam) {
    char  buffer[1024];
    DWORD bytesRead;
    BOOL  fWaitingOnRead = FALSE;

    while (g_KeepRunning) {
        if (!fWaitingOnRead) {
            // 发起异步读取请求
            if (!ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, &osRead)) {
                if (GetLastError() != ERROR_IO_PENDING) {
                    printf("\n[Error] read eeror for comx\n");
                    break;
                }
                fWaitingOnRead = TRUE;
            } else if (bytesRead > 0) {
                // 立即读取成功
                fwrite(buffer, 1, bytesRead, stdout);
                fflush(stdout);
            }
        }

        if (fWaitingOnRead) {
            // 等待读取完成事件，超时设为 100ms 保证线程活跃
            DWORD wait = WaitForSingleObject(osRead.hEvent, 100);
            if (wait == WAIT_OBJECT_0) {
                if (GetOverlappedResult(hSerial, &osRead, &bytesRead, FALSE)) {
                    if (bytesRead > 0) {
                        fwrite(buffer, 1, bytesRead, stdout);
                        fflush(stdout);
                    }
                }
                fWaitingOnRead = FALSE;
                ResetEvent(osRead.hEvent);
            }
        }
    }
    return 0;
}

void ListAvailablePorts() {
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return;

    SP_DEVINFO_DATA DeviceInfoData;
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
        TCHAR deviceName[256];
        if (SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)deviceName,
                                             sizeof(deviceName), NULL)) {
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
    snprintf(portName, sizeof(portName), "\\\\.\\%s", userInput);

    printf("Input bandRate (key 1500000): ");
    scanf("%lu", &baudRate);

    printf("bata bit (5-8): ");
    int choice;
    scanf("%d", &choice);
    byteSize = (choice >= 5 && choice <= 8) ? (BYTE)choice : 8;

    printf("1:1\n2:1.5\n3:2\nstop bit :");
    scanf("%d", &choice);
    stopBits = (choice == 2) ? ONE5STOPBITS : (choice == 3 ? TWOSTOPBITS : ONESTOPBIT);

    printf("1:none\n2:odd\n3:even\ncheck bit: ");
    scanf("%d", &choice);
    parity = (choice == 2) ? ODDPARITY : (choice == 3 ? EVENPARITY : NOPARITY);
}

BOOL OpenSerialPort() {
    // 关键点：增加 FILE_FLAG_OVERLAPPED 开启异步模式
    hSerial = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error: An error occurred while opening this serial port %s\n, number of error: %lu\n", portName,
               GetLastError());
        return FALSE;
    }

    // 初始化异步事件句柄
    osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // 异步模式下超时设置逻辑不同，通常设为立即返回
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
        return FALSE;
    }

    printf("\nserial %s ok\n", portName);
    printf("\nIf your system supports xterm\n");
    printf("please enter the following command after logging into the shell:\n");
	printf("\033[1;31mexport TERM=xterm\n\033[0m");
	return TRUE;
}

void InteractiveMode() {
    printf("\n--- Press ctrl + C to exit ---\n\n");

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD  oldMode;
    GetConsoleMode(hStdin, &oldMode);

    // 开启虚拟终端输入，这对 Vim 的转义序列至关重要
    SetConsoleMode(hStdin, ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_EXTENDED_FLAGS);

    HANDLE hThread = CreateThread(NULL, 0, ReadSerialThread, NULL, 0, NULL);
    SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);

    INPUT_RECORD ir;
    DWORD        count, dwWritten;

    while (1) {
        // 阻塞等待键盘事件，不消耗 CPU
        if (ReadConsoleInput(hStdin, &ir, 1, &count) && count > 0) {
            if (!(ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown)) {
                continue;
            }
            char ch = ir.Event.KeyEvent.uChar.AsciiChar;

            // Ctrl+C 退出
            if ((ch == 3) || (g_KeepRunning == FALSE))
                goto EXIT_INTERACTIVE;

            if (ch != 0) {
                // 异步写入：丢进内核缓冲区就走，不等待硬件响应
                if (!WriteFile(hSerial, &ch, 1, &dwWritten, &osWrite)) {
                    if (GetLastError() != ERROR_IO_PENDING) {
                        printf("\n[Error] write err\n");
                        goto EXIT_INTERACTIVE;
                    }
                }
            }
        }
    }

EXIT_INTERACTIVE:
    // 等待线程自己退出 (500毫秒)
    if (WaitForSingleObject(hThread, 500) == WAIT_TIMEOUT) {
        TerminateThread(hThread, 0); // 只有超时了才强制杀掉
    }
    CloseHandle(hThread);
    CloseHandle(osRead.hEvent);
    CloseHandle(osWrite.hEvent);
    SetConsoleMode(hStdin, oldMode);
}

int main() {
    signal(SIGINT, SignalHandler);
    do {
        system("cls");
        ListAvailablePorts();
        printf("===== windows tty for Terminal =====\n");
        ConfigureSerialPort();
        clsInputSpace();

    } while (!OpenSerialPort());

    SetTerminalUTF8();
    InteractiveMode();
    SetTerminalGBK();

    if (hSerial != INVALID_HANDLE_VALUE)
        CloseHandle(hSerial);
    return 0;
}

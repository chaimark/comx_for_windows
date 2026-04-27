#define INITGUID
#include "comx.h"
#include "./C_MyLib/JsonCheckFun.h"
#include "./C_MyLib/JsonDataAnalyzeLib.h"
#include "./C_MyLib/JsonSetLib.h"
#include "./C_MyLib/StrLib.h"
#include <ctype.h>
#include <signal.h>

#define stdin_clean                                                                                                    \
    do {                                                                                                               \
        int c;                                                                                                         \
        while ((c = getchar()) != '\n' && c != EOF)                                                                    \
            continue;                                                                                                  \
    } while (0)

// 串口信息
HANDLE hSerial = INVALID_HANDLE_VALUE;
DWORD baudRate = 1500000;
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
    return;
}

/**
 * 检查字符串是否表示一个合法的整数（支持可选的正负号）
 * 例如："123", "-456", "+789" 均为合法
 * 空字符串或包含非数字字符的字符串不合法
 */
int is_valid_integer(const char* str) {
    if (str == NULL || *str == '\0')
        return 0; // 空字符串非法

    // 跳过可选的正负号
    if (*str == '+' || *str == '-')
        str++;

    // 至少需要一位数字
    if (*str == '\0')
        return 0;

    // 检查剩余字符是否全是数字
    while (*str) {
        if (!isdigit((unsigned char)*str))
            return 0;
        str++;
    }
    return 1;
}

int getUserInt(long int default_value) {
    char input[100]; // 存储用户输入的行
    int value;       // 最终要赋值的变量

    while (1) {
        memset(&input, 0, ARR_SIZE(input));
        value = 0;
        printf("please int number ( %ld ) : ", default_value);

        // 读取一行输入，包括可能的空格
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("read error\n");
            return 1;
        }

        // 去掉末尾的换行符
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n')
            input[len - 1] = '\0';

        // 情况1：空输入（只按了回车）
        if (strlen(input) == 0) {
            value = default_value;
        } else if (is_valid_integer(input)) {
            value = atoi(input); // 转换为 int
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
    // 读取一行输入，包括可能的空格
    scanf("%s", userInput);
    stdin_clean;
    snprintf(portName, sizeof(portName), "\\\\.\\%s", userInput);

    printf("Input bandRate (key 115200)\n");
    baudRate = (DWORD)getUserInt(1500000);

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

    // 显示设置结果
    printf("\nprot:%s\n", userInput);
    printf("band Rate:%ld\n", baudRate);
    printf("byte Size:%d\n", byteSize);
    printf("stop Bit:%d\n", stopBits + 1);
    printf("check bit:%d\n", choice);
    printf("continue the Enter any key:");
    getch();
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
    return TRUE;
}

// 线程函数：接收串口数据
DWORD WINAPI ReadSerialThread(LPVOID lpParam) {
    char buffer[1024];
    DWORD bytesRead;
    BOOL fWaitingOnRead = FALSE;

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
        } else {
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

void InteractiveMode() {
    printf("\n--- Press Ctrl+A then Ctrl+C to exit ---\n\n");

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD oldMode;
    GetConsoleMode(hStdin, &oldMode);
    SetConsoleMode(hStdin, ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_EXTENDED_FLAGS);

    HANDLE hThread = CreateThread(NULL, 0, ReadSerialThread, NULL, 0, NULL);
    SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);

    INPUT_RECORD ir;
    DWORD count, dwWritten;
    int seq_state = 0; // 0:等待Ctrl+A, 1:已收到Ctrl+A等待Ctrl+C

    while (1) {
        if (!(ReadConsoleInput(hStdin, &ir, 1, &count) && count > 0)) {
            continue;
        }
        if (!(ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown)) {
            continue;
        }

        char ch = ir.Event.KeyEvent.uChar.AsciiChar;
        // 序列检测状态机
        if (seq_state == 0) {
            if (ch == 1) { // Ctrl+A
                seq_state = 1;
            }
            // 其他字符（包括单独的 Ctrl+C）直接发送
            if (ch != 0) {
                if (!WriteFile(hSerial, &ch, 1, &dwWritten, &osWrite)) {
                    if (GetLastError() != ERROR_IO_PENDING) {
                        printf("\n[Error] write err\n");
                        goto EXIT_INTERACTIVE;
                    }
                }
            }
            continue;
        }

        if (seq_state == 1) {
            // 等待 Ctrl+C
            if (ch == 3) { // Ctrl+C
                // 序列正确，退出程序
                g_KeepRunning = FALSE; // 通知读取线程退出
                goto EXIT_INTERACTIVE;
            }
            // 收到其他按键：先发送之前缓存的 Ctrl+A（如果需要发送的话）
            // 注意：之前的 Ctrl+A 被忽略了，这里可以选择发送 Ctrl+A 再发送当前字符
            // 为了行为直观，这里将 Ctrl+A 和当前字符都发送
            char ctrlA = 1;
            WriteFile(hSerial, &ctrlA, 1, &dwWritten, &osWrite);
            if (ch != 0) {
                WriteFile(hSerial, &ch, 1, &dwWritten, &osWrite);
            }
            seq_state = 0; // 重置状态
            continue;
        }
    }

EXIT_INTERACTIVE:
    if (WaitForSingleObject(hThread, 500) == WAIT_TIMEOUT) {
        TerminateThread(hThread, 0);
    }
    CloseHandle(hThread);
    CloseHandle(osRead.hEvent);
    CloseHandle(osWrite.hEvent);
    SetConsoleMode(hStdin, oldMode);
}

int main(int argc, char* argv[]) {
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

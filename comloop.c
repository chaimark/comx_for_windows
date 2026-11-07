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

    // 丢掉打开端口前积压的脏数据
    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    printf("\nserial %s ok\n", portName);
    return TRUE;
}

// ======================= 收 <=> 发 之间的共享缓冲 =======================
// 读线程只负责"收", 把字节追加到 g_rxBuf; 主线程负责判断一帧是否收完、打印、再发。
// 这样避免读线程直接 fwrite 和主线程 printf 交叉输出, 也避免用"收到任意字节"当触发条件。
#define RESPONSE_TIMEOUT_MS 2000 // 发送后等"第一个响应字节"的超时, 超时即认为开发板没响应
#define RX_IDLE_MS 30            // 收到首字节后, 连续这么久没有新数据就认为一帧收完
#define SEND_INTERVAL_MS 100     // 两次发送之间的间隔
#define WRITE_TIMEOUT_MS 1000    // 单次异步写超时

static CRITICAL_SECTION g_rxLock;
static char g_rxBuf[4096];
static volatile LONG g_rxLen = 0;
static volatile DWORD g_rxLastTick = 0;

static void RxInit(void) {
    InitializeCriticalSection(&g_rxLock);
}

// 发送前清空, 避免把上一帧的残留数据误判成本次响应
static void RxReset(void) {
    EnterCriticalSection(&g_rxLock);
    g_rxLen = 0;
    g_rxBuf[0] = '\0';
    g_rxLastTick = GetTickCount();
    LeaveCriticalSection(&g_rxLock);
}

// volatile LONG 在 x86/x64 上是原子读, 不需要加锁
static LONG RxLen(void) {
    return g_rxLen;
}

static DWORD RxCopyOut(char *Out, DWORD Cap) {
    DWORD Num;
    EnterCriticalSection(&g_rxLock);
    Num = (DWORD)g_rxLen;
    if (Num >= Cap) {
        Num = Cap - 1;
    }
    memcpy(Out, g_rxBuf, Num);
    Out[Num] = '\0';
    LeaveCriticalSection(&g_rxLock);
    return Num;
}

static void RxAppend(const char *Data, DWORD Len) {
    EnterCriticalSection(&g_rxLock);
    DWORD Room = (DWORD)sizeof(g_rxBuf) - 1 - (DWORD)g_rxLen;
    if (Len > Room) {
        Len = Room;
    }
    if (Len > 0) {
        memcpy(g_rxBuf + g_rxLen, Data, Len);
        g_rxLen += Len;
        g_rxBuf[g_rxLen] = '\0';
        g_rxLastTick = GetTickCount();
    }
    LeaveCriticalSection(&g_rxLock);
}

// 线程函数：只收数据, 打印交给主线程
DWORD WINAPI ReadSerialThread(LPVOID lpParam) {
    char buffer[1024];
    DWORD bytesRead;
    BOOL fWaitingOnRead = FALSE;

    while (g_KeepRunning) {
        if (!fWaitingOnRead) {
            if (!ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, &osRead)) {
                if (GetLastError() != ERROR_IO_PENDING) {
                    printf("\n[Error] read error for comx\n");
                    break;
                }
                fWaitingOnRead = TRUE;
            } else if (bytesRead > 0) {
                RxAppend(buffer, bytesRead);
            } else {
                // 没有数据, 让出 CPU (ReadIntervalTimeout=MAXDWORD 时 ReadFile 会立刻返回 0 字节)
                Sleep(1);
            }
        } else {
            DWORD wait = WaitForSingleObject(osRead.hEvent, 50);
            if (wait == WAIT_OBJECT_0) {
                if (GetOverlappedResult(hSerial, &osRead, &bytesRead, FALSE)) {
                    if (bytesRead > 0) {
                        RxAppend(buffer, bytesRead);
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

// 把一段数据完整写出去(处理异步写), 返回是否全部写成功
static BOOL WriteFrame(const char *Data, DWORD Len, DWORD TimeoutMs) {
    DWORD Total = 0;
    while (Total < Len) {
        DWORD Written = 0;
        if (!WriteFile(hSerial, Data + Total, Len - Total, &Written, &osWrite)) {
            DWORD err = GetLastError();
            if (err != ERROR_IO_PENDING) {
                printf("\n[Error] write error for comx, code: %lu\n", err);
                return FALSE;
            }
            if (WaitForSingleObject(osWrite.hEvent, TimeoutMs) != WAIT_OBJECT_0) {
                printf("\n[Error] write timeout, code: %lu\n", GetLastError());
                return FALSE;
            }
            if (!GetOverlappedResult(hSerial, &osWrite, &Written, FALSE)) {
                printf("\n[Error] write overlapped failed, code: %lu\n", GetLastError());
                ResetEvent(osWrite.hEvent);
                return FALSE;
            }
        }
        // 重置事件状态, 为下次写入做准备
        ResetEvent(osWrite.hEvent);
        if (Written == 0) {
            printf("\n[Error] write 0 bytes\n");
            return FALSE;
        }
        Total += Written;
    }
    return TRUE;
}

void InteractiveMode() {
    printf("\n--- Send fixed cmd, wait reply, repeat until no reply. Ctrl+C to exit ---\n\n");

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
    bytesRead = (DWORD)strlen(buffer);

    RxInit();
    RxReset();

    HANDLE hThread = CreateThread(NULL, 0, ReadSerialThread, NULL, 0, NULL);
    if (hThread != NULL) {
        SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
    }

    char Resp[4096];
    int Round = 0;

    while (g_KeepRunning) {
        Round++;
        RxReset(); // 发之前清掉残留, 保证收到的就是这一次的响应

        if (!WriteFrame(buffer, bytesRead, WRITE_TIMEOUT_MS)) {
            break;
        }
        printf("[%d] tx %lu bytes\n", Round, bytesRead);
        fflush(stdout);

        // 1) 等第一个响应字节, 超时 = 开发板没响应, 结束
        DWORD Start = GetTickCount();
        while (g_KeepRunning && (RxLen() == 0) && ((GetTickCount() - Start) < RESPONSE_TIMEOUT_MS)) {
            Sleep(5);
        }
        if (RxLen() == 0) {
            printf("[%d] no response in %lu ms, stop.\n", Round, (unsigned long)RESPONSE_TIMEOUT_MS);
            break;
        }

        // 2) 等到一帧收完(空闲超时判定)
        while (g_KeepRunning && ((GetTickCount() - g_rxLastTick) < RX_IDLE_MS)) {
            Sleep(1);
        }

        DWORD n = RxCopyOut(Resp, sizeof(Resp));
        printf("[%d] rx %lu bytes:\n%s\n", Round, n, Resp);
        fflush(stdout);

        // 3) 稍微等一下再发下一次
        Sleep(SEND_INTERVAL_MS);
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
    DeleteCriticalSection(&g_rxLock);
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

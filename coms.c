#define INITGUID
#include "coms.h"
#include "./WorkLib/StrLib.h"
#include "./WorkLib/cJson.h"
#include <ctype.h>
#include <signal.h>

struct _FileInfoSpaces {
    bool IsShowTime;
    char CmdNameArray[2048];
    char CmdVarArray[2048];
} FileInfoSpaces;
JsonArray CmdName = {0};
JsonArray CmdVar = {0};

#define stdin_clean                                           \
    do {                                                      \
        int c;                                                \
        while ((c = getchar()) != '\n' && c != EOF) continue; \
    } while (0)

// 串口信息
HANDLE hSerial = INVALID_HANDLE_VALUE;
DWORD baudRate = 115200 
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
            }
        } else {
            DWORD wait = WaitForSingleObject(osRead.hEvent, 100);
            if (wait == WAIT_OBJECT_0) {
                if (GetOverlappedResult(hSerial, &osRead, &bytesRead, FALSE)) {
                    if (bytesRead > 0) {
                        fwrite(buffer, 1, bytesRead, stdout);
                        fflush(stdout);
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

// 底层只读文件，无交互
int _readFile(FILE *file) {
    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long FileSize = ftell(file);
    rewind(file);

    // Allocate memory for the file content
    strnew_malloc(Content, FileSize + 1);
    if (Content.Name._char == NULL) {
        perror("Failed to allocate memory");
        return -1;
    }

    // Read the file content into the buffer
    size_t ReadSize = fread(Content.Name._char, 1, FileSize, file);
    if (ReadSize != (size_t)FileSize) {
        perror("Failed to read the entire file");
        return -1;
    }

    // Null-terminate the string
    Content.Name._char[FileSize] = '\0';

    JsonObject FileInfo = newJsonObjectByString(Content);
    FileInfoSpaces.IsShowTime = FileInfo.getBool(&FileInfo, "isShowTime");
    if ((FileInfo.isJsonNull(&FileInfo, "cmd_Name_Array") >= 0) && (FileInfo.isJsonNull(&FileInfo, "cmd_Var_Array") >= 0)) {
        CmdVar = FileInfo.getArray(&FileInfo, "cmd_Var_Array", NEW_NAME(FileInfoSpaces.CmdVarArray));
        CmdName = FileInfo.getArray(&FileInfo, "cmd_Name_Array", NEW_NAME(FileInfoSpaces.CmdNameArray));
    }
    if (CmdName.sizeItemNum(&CmdName) != CmdVar.sizeItemNum(&CmdVar)) {
        JsonArray Zero = {0};
        CmdName = Zero;
        CmdVar = Zero;
        printf("CmdName_Array and CmdVar_Array size not equal\n");
        return -1;
    }
    return 0;
}

void displayHelp(const char *FileName) {
    // 重新读文件更新配置
    FILE *file = fopen(FileName, "rb"); // 使用"rb"模式
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    if (_readFile(file) == -1) {
        printf("read file error");
        fclose(file);
        return;
    }
    fclose(file);
    printf("\n>>  %s\n>>  %s\n", CmdName.JsonString.Name._char, CmdVar.JsonString.Name._char);
}

// 提取 CmdLen 中输入的参数
bool getUserArg(strnew OutItemStr, strnew CmdLen, int Item_I) {
    // stringSlice(IntStr, CmdName, , );
    int count = 0;
    int n = 0;
    char *ptr = CmdLen.Name._char;
    while (sscanf(ptr, "%s%n", OutItemStr.Name._char, &n) == 1) {
        if (count == Item_I) {
            return true;
        }
        memset(OutItemStr.Name._char, 0, OutItemStr.MaxLen);
        count++;
        ptr += n;
    }
    return false;
}

// 将 ChrA 换成 ChrB, 如果确实需要 ChrA 就写两个 ChrA
void changeChr(strnew InputStr, char ChrA, char ChrB) {
    for (int i = 0; i < InputStr.MaxLen; i++) {
        if (InputStr.Name._char[i] != ChrA) {
            continue;
        }
        InputStr.Name._char[i] = ChrB;
    }
}
// CmdLen: "read 02345678902"
// SendJsonSpace: "{'checkFlag':'$$chai',Id':'$','data':'read'}"
// 装载用户快捷指令的参数, 如果需要发送 $ 输入两个 $$ 代替
void makeCmdStr(strnew SendJsonSpace, strnew CmdLen) {
    int Arg_i = 1;
    int OverAddr = 0;
    strnew_malloc(OverStr, strlen(SendJsonSpace.Name._char) + strlen(CmdLen.Name._char));
    strnew_malloc(IntStr, CmdLen.MaxLen);
    for (int i = 0; (i < SendJsonSpace.MaxLen) && (SendJsonSpace.Name._char[i] != '\0'); i++) {
        if (i + 1 >= SendJsonSpace.MaxLen) {
            OverStr.Name._char[OverAddr++] = '\0';
            break;
        }
        if (SendJsonSpace.Name._char[i] != '$') {
            OverStr.Name._char[OverAddr++] = (SendJsonSpace.Name._char[i] == '\'' ? '\"' : SendJsonSpace.Name._char[i]);
            continue;
        }
        if (SendJsonSpace.Name._char[i + 1] == '$') {
            OverStr.Name._char[OverAddr++] = SendJsonSpace.Name._char[i];
            i++;
            continue;
        }
        // SendJsonSpace.Name._char[i] 是 $ 变量
        // 提取 CmdLen 中输入的参数
        if (getUserArg(IntStr, CmdLen, Arg_i++)) {
            // 追加到 OverStr
            OverAddr = catString(OverStr.Name._char, IntStr.Name._cschar, OverStr.MaxLen, strlen(IntStr.Name._char));
        } else {
            changeChr(SendJsonSpace, '\'', '\"');
            return; // 中止，直接发 SendJsonSpace
        }
    }
    Arg_i = strlen(OverStr.Name._char);
    // 替换结束，替换 SendJsonSpace
    memcpy(SendJsonSpace.Name._char, OverStr.Name._char, strlen(OverStr.Name._char) + 1);
    SendJsonSpace.Name._char[Arg_i] = '\0';
    return;
}

void findCmdByUserKeyStr(strnew UserKeyStr) {
    // 如果用户输入的字符串长度大于快捷指令的长度,则不可能是会计指令
    if (strlen(UserKeyStr.Name._char) > 256) {
        return; // 可以发送
    }
    newString(TempCmdSpace, 257);
    for (int i = 0; i < CmdName.sizeItemNum(&CmdName); i++) {
        CmdName.get(&CmdName, TempCmdSpace, i);
        // 指令表是用户输入的字串
        if (strstr(UserKeyStr.Name._char, TempCmdSpace.Name._char) != NULL) {
            TempCmdSpace.Name._int[0] = i;
            TempCmdSpace.Name._int[1] = -1;
            TempCmdSpace.Name._int[2] = 0;
            break;
        }
        memset(TempCmdSpace.Name._char, 0, TempCmdSpace.MaxLen);
    }
    // 如果是 -1 表示找到了
    if (TempCmdSpace.Name._int[1] == -1) {
        int Addr = TempCmdSpace.Name._int[0];
        memset(TempCmdSpace.Name._char, 0, TempCmdSpace.MaxLen);
        // 保存用户输入
        memcpy(TempCmdSpace.Name._char, UserKeyStr.Name._char, strlen(UserKeyStr.Name._char));
        memset(UserKeyStr.Name._char, 0, UserKeyStr.MaxLen);
        // 将用户的快捷指令改成 config.json 中存的指令
        CmdVar.get(&CmdVar, UserKeyStr, Addr);
        // 将用户输入的指令参数 $, 替换到 UserKeyStr 中保存的模板指令
        makeCmdStr(UserKeyStr, TempCmdSpace);
    }
    return;
}

DWORD isShortCmd(strnew CmdLine, DWORD bytesRead) {
    if (strlen(CmdLine.Name._char) == 0) {
        return 0;
    }
    if (strstr(CmdLine.Name._char, "clear") != NULL) {
        system("cls");
        return 0;
    }
    if (strstr(CmdLine.Name._char, "help") != NULL) {
        displayHelp("coms_config.json");
        return 0;
    }

    // 检查用户输入的是不是指令表中的指令
    findCmdByUserKeyStr(CmdLine);
    return strlen(CmdLine.Name._char);
}


void InteractiveMode() {
    printf("\n--- Press Ctrl+A then Ctrl+C to exit ---\n\n");

    HANDLE hThread = CreateThread(NULL, 0, ReadSerialThread, NULL, 0, NULL);
    SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);

    DWORD dwWritten;
    char buffer[1024];
    DWORD bytesRead;

    while (g_KeepRunning) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        bytesRead = strlen(buffer);

        // 移除控制台输入自带的 \r 字符
        if (bytesRead > 0 && buffer[bytesRead - 1] == '\r') {
            bytesRead--;
        }
        bytesRead = isShortCmd(NEW_NAME(buffer), bytesRead);
        if (bytesRead > 0) {
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

    displayHelp("coms_config.json");

    SetTerminalUTF8();
    InteractiveMode();
    SetTerminalGBK();

    if (hSerial != INVALID_HANDLE_VALUE)
        CloseHandle(hSerial);
    return 0;
}

#include "framework.h"
#include "czr_extraktor.h"
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <string>
#include <windows.h>

#define MAX_LOADSTRING 100
#define IDC_FILEPATH_EDIT 1001
#define IDC_BROWSE_BTN 1002
#define IDC_EXTRACT_BTN 1003

// Глобальные переменные:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Обработчик выбора файла
HWND hFilePathEdit = nullptr;

// Объявления функций:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Вспомогательные функции
std::string WideToString(const std::wstring& wide) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wide[0], (int)wide.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wide[0], (int)wide.size(), &str[0], size_needed, NULL, NULL);
    return str;
}

std::wstring StringToWide(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

// Извлечение PNG из файла
bool ExtractPNG(const WCHAR* inputFilePath, const WCHAR* outputFilePath) {
    try {
        // Открытие входного файла в бинарном режиме
        std::ifstream inputFile(inputFilePath, std::ios::binary);
        if (!inputFile.is_open()) {
            MessageBox(nullptr, L"Не удалось открыть входной файл!", L"Ошибка", MB_ICONERROR);
            return false;
        }

        // Чтение всего файла
        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        std::string fileContent = buffer.str();
        inputFile.close();

        // Поиск маркеров PNG
        size_t pngPos = std::string::npos;

        // Поиск маркера PNG (‰PNG)
        const unsigned char pngMagic[] = { 0x89, 0x50, 0x4E, 0x47 };

        // Преобразуем в поиск по сырым байтам
        for (size_t i = 0; i < fileContent.size() - 3; ++i) {
            if (memcmp(&fileContent[i], pngMagic, 4) == 0) {
                pngPos = i;
                break;
            }
        }

        if (pngPos == std::string::npos) {
            MessageBox(nullptr, L"PNG маркер не найден в файле!", L"Ошибка", MB_ICONWARNING);
            return false;
        }

        // Извлечение PNG данных (от маркера до конца файла)
        std::ofstream outputFile(outputFilePath, std::ios::binary);
        if (!outputFile.is_open()) {
            MessageBox(nullptr, L"Не удалось создать выходной файл!", L"Ошибка", MB_ICONERROR);
            return false;
        }

        outputFile.write(&fileContent[pngPos], fileContent.size() - pngPos);
        outputFile.close();

        // Сообщение об успехе
        std::wstring message = L"PNG успешно извлечено!\nФайл сохранен: ";
        message += outputFilePath;
        MessageBox(nullptr, message.c_str(), L"Успех", MB_ICONINFORMATION);
        return true;

    }
    catch (const std::exception& e) {
        std::string errorMsg = std::string("Ошибка при обработке файла: ") + e.what();
        MessageBox(nullptr, StringToWide(errorMsg).c_str(), L"Ошибка", MB_ICONERROR);
        return false;
    }
}

// Диалог выбора файла
bool OpenFileDialog(HWND hWnd, WCHAR* filePath, DWORD bufferSize) {
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = bufferSize;
    ofn.lpstrFilter = L"CZR файлы (*.czr)\0*.czr\0Все файлы (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    return GetOpenFileNameW(&ofn);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CZREXTRAKTOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CZREXTRAKTOR));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CZREXTRAKTOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CZREXTRAKTOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    // Создание элементов интерфейса
    // Статический текст "Выберите файл:"
    CreateWindowW(L"STATIC", L"Выберите файл:",
        WS_VISIBLE | WS_CHILD, 10, 10, 100, 20, hWnd, nullptr, hInstance, nullptr);

    // Поле редактирования для пути к файлу
    hFilePathEdit = CreateWindowW(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY, 10, 35, 250, 25,
        hWnd, (HMENU)IDC_FILEPATH_EDIT, hInstance, nullptr);

    // Кнопка "Обзор"
    CreateWindowW(L"BUTTON", L"Обзор...",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 270, 35, 80, 25,
        hWnd, (HMENU)IDC_BROWSE_BTN, hInstance, nullptr);

    // Кнопка "Записать PNG"
    CreateWindowW(L"BUTTON", L"Записать PNG",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 75, 340, 30,
        hWnd, (HMENU)IDC_EXTRACT_BTN, hInstance, nullptr);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static WCHAR szFilePath[MAX_PATH] = L"";

    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        switch (wmId)
        {
        case IDC_BROWSE_BTN:
        {
            if (OpenFileDialog(hWnd, szFilePath, MAX_PATH))
            {
                SetWindowTextW(hFilePathEdit, szFilePath);
            }
        }
        break;

        case IDC_EXTRACT_BTN:
        {
            if (wcslen(szFilePath) == 0)
            {
                MessageBox(hWnd, L"Пожалуйста, выберите файл!", L"Предупреждение", MB_ICONWARNING);
                break;
            }

            // Генерирование имени выходного файла
            WCHAR outputPath[MAX_PATH];
            wcscpy_s(outputPath, MAX_PATH, szFilePath);

            // Замена расширения на .png
            WCHAR* dot = wcsrchr(outputPath, L'.');
            if (dot) {
                wcscpy_s(dot, MAX_PATH - (dot - outputPath), L".png");
            }
            else {
                wcscat_s(outputPath, MAX_PATH, L".png");
            }

            ExtractPNG(szFilePath, outputPath);
        }
        break;

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

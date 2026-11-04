#include "framework.h"
#include "czr_extraktor.h"
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

#define MAX_LOADSTRING 100
#define IDC_FILEPATH_EDIT 1001
#define IDC_BROWSE_BTN 1002
#define IDC_EXTRACT_BTN 1003

// Глобальные переменные:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Обработчик выбора файлов
HWND hFilePathEdit = nullptr;
std::vector<std::wstring> g_selectedFiles;

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

// Чтение всего файла в память
bool ReadWholeFile(const WCHAR* filePath, std::string& content) {
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile.is_open())
        return false;
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    content = buffer.str();
    return true;
}

// Извлечение PNG из файла (как раньше — от первого маркера до конца файла)
bool ExtractPNG(const WCHAR* inputFilePath, const WCHAR* outputFilePath) {
    try {
        std::string fileContent;
        if (!ReadWholeFile(inputFilePath, fileContent)) {
            MessageBox(nullptr, L"Не удалось открыть входной файл!", L"Ошибка", MB_ICONERROR);
            return false;
        }

        // Поиск маркера PNG (‰PNG)
        const unsigned char pngMagic[] = { 0x89, 0x50, 0x4E, 0x47 };
        size_t pngPos = std::string::npos;

        if (fileContent.size() >= 4) {
            for (size_t i = 0; i + 4 <= fileContent.size(); ++i) {
                if (memcmp(&fileContent[i], pngMagic, 4) == 0) {
                    pngPos = i;
                    break;
                }
            }
        }

        if (pngPos == std::string::npos) {
            MessageBox(nullptr, L"PNG маркер не найден в файле!", L"Ошибка", MB_ICONWARNING);
            return false;
        }

        std::ofstream outputFile(outputFilePath, std::ios::binary);
        if (!outputFile.is_open()) {
            MessageBox(nullptr, L"Не удалось создать выходной файл!", L"Ошибка", MB_ICONERROR);
            return false;
        }

        outputFile.write(&fileContent[pngPos], fileContent.size() - pngPos);
        outputFile.close();

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

// Извлечение всех JPEG из файла (XML+JPG содержимое)
// Возвращает количество извлечённых изображений (-1 при ошибке открытия)
int ExtractJpegs(const WCHAR* inputFilePath) {
    try {
        std::string fileContent;
        if (!ReadWholeFile(inputFilePath, fileContent)) {
            MessageBox(nullptr, L"Не удалось открыть входной файл!", L"Ошибка", MB_ICONERROR);
            return -1;
        }

        // Маркер начала JPEG (SOI + первый байт маркера): FF D8 FF
        const unsigned char jpgStart[] = { 0xFF, 0xD8, 0xFF };
        // Маркер конца JPEG (EOI): FF D9
        const unsigned char jpgEnd[] = { 0xFF, 0xD9 };
        const size_t startLen = 3;
        const size_t endLen = 2;

        // Собираем все позиции начала JPEG
        std::vector<size_t> starts;
        if (fileContent.size() >= startLen) {
            for (size_t i = 0; i + startLen <= fileContent.size(); ++i) {
                if (memcmp(&fileContent[i], jpgStart, startLen) == 0)
                    starts.push_back(i);
            }
        }

        if (starts.empty()) {
            MessageBox(nullptr, L"JPG маркеры не найдены в файле!", L"Ошибка", MB_ICONWARNING);
            return 0;
        }

        // Формируем базу имени выходных файлов (без расширения)
        std::wstring inputPath(inputFilePath);
        size_t slash = inputPath.find_last_of(L"\\/");
        std::wstring dir = (slash == std::wstring::npos) ? L"" : inputPath.substr(0, slash + 1);
        std::wstring name = (slash == std::wstring::npos) ? inputPath : inputPath.substr(slash + 1);
        size_t dot = name.find_last_of(L'.');
        std::wstring base = (dot == std::wstring::npos) ? name : name.substr(0, dot);

        int extracted = 0;
        for (size_t s = 0; s < starts.size(); ++s) {
            // Ищем конец (FF D9) начиная со следующего байта после SOI
            size_t endPos = std::string::npos;
            for (size_t i = starts[s] + startLen; i + endLen <= fileContent.size(); ++i) {
                if (memcmp(&fileContent[i], jpgEnd, endLen) == 0) {
                    endPos = i;
                    break;
                }
            }
            if (endPos == std::string::npos) {
                // Незавершённый JPEG — пропускаем
                continue;
            }

            // Имя файла: исходное имя _001.jpg, _002.jpg, ...
            WCHAR suffix[16];
            swprintf_s(suffix, L"_%03d.jpg", (int)(extracted + 1));
            std::wstring outPath = dir + base + suffix;

            std::ofstream outputFile(outPath.c_str(), std::ios::binary);
            if (!outputFile.is_open())
                continue;

            // Записываем JPEG целиком: от SOI включительно до EOI включительно
            size_t length = (endPos + endLen) - starts[s];
            outputFile.write(&fileContent[starts[s]], length);
            outputFile.close();
            ++extracted;
        }

        return extracted;
    }
    catch (const std::exception& e) {
        std::string errorMsg = std::string("Ошибка при обработке файла: ") + e.what();
        MessageBox(nullptr, StringToWide(errorMsg).c_str(), L"Ошибка", MB_ICONERROR);
        return -1;
    }
}

// Диалог выбора нескольких файлов.
// Возвращает список полных путей; если выбран один файл — список из одного элемента.
std::vector<std::wstring> OpenFileDialogMulti(HWND hWnd) {
    std::vector<std::wstring> result;

    // Буфер под многофайловый выбор: каталог + все имена файлов
    static const DWORD bufferSize = 32768;
    WCHAR* filePath = new WCHAR[bufferSize];
    filePath[0] = L'\0';

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = bufferSize;
    ofn.lpstrFilter = L"CZI файлы (*.czi)\0*.czi\0CZR файлы (*.czr)\0*.czr\0Все файлы (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
        | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    if (GetOpenFileNameW(&ofn)) {
        WCHAR* p = filePath;

        // Первый фрагмент: каталог (при мультивыборе) или полный путь (одиночный выбор)
        std::wstring first(p);
        p += first.size() + 1;

        if (*p == L'\0') {
            // Выбран один файл — первый фрагмент уже полный путь
            result.push_back(first);
        }
        else {
            // Мультивыбор: первый фрагмент — каталог, дальше — имена файлов
            std::wstring dir = first;
            if (!dir.empty() && dir.back() != L'\\')
                dir += L'\\';
            while (*p) {
                std::wstring fileName(p);
                result.push_back(dir + fileName);
                p += fileName.size() + 1;
            }
        }
    }

    delete[] filePath;
    return result;
}

// Обработка одного выбранного файла в зависимости от расширения
bool ProcessFile(HWND hWnd, const std::wstring& filePath) {
    // Определяем расширение
    size_t dot = filePath.find_last_of(L'.');
    std::wstring ext = (dot == std::wstring::npos) ? L"" : filePath.substr(dot);
    // Приводим к нижнему регистру
    for (auto& c : ext) c = (WCHAR)towlower(c);

    if (ext == L".czr") {
        // Старый режим: один PNG
        std::wstring outputPath = filePath.substr(0, dot == std::wstring::npos ? filePath.size() : dot) + L".png";
        return ExtractPNG(filePath.c_str(), outputPath.c_str());
    }
    else if (ext == L".czi") {
        // Новый режим: извлечение всех JPEG
        int count = ExtractJpegs(filePath.c_str());
        if (count <= 0)
            return false;
        std::wstring message = L"Файл: " + filePath + L"\nИзвлечено изображений: " + std::to_wstring(count);
        MessageBox(hWnd, message.c_str(), L"Успех", MB_ICONINFORMATION);
        return true;
    }
    else {
        MessageBox(hWnd,
            (L"Неизвестный тип файла: " + filePath + L"\nПоддерживаются .czr (PNG) и .czi (JPG).").c_str(),
            L"Предупреждение", MB_ICONWARNING);
        return false;
    }
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
        CW_USEDEFAULT, 0, 400, 200, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    // Создание элементов интерфейса
    // Статический текст "Выберите файлы:"
    CreateWindowW(L"STATIC", L"Выберите файлы:",
        WS_VISIBLE | WS_CHILD, 10, 10, 120, 20, hWnd, nullptr, hInstance, nullptr);

    // Поле редактирования для отображения выбора
    hFilePathEdit = CreateWindowW(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_AUTOHSCROLL, 10, 35, 250, 25,
        hWnd, (HMENU)IDC_FILEPATH_EDIT, hInstance, nullptr);

    // Кнопка "Обзор"
    CreateWindowW(L"BUTTON", L"Обзор...",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 270, 35, 80, 25,
        hWnd, (HMENU)IDC_BROWSE_BTN, hInstance, nullptr);

    // Кнопка "Извлечь изображения"
    CreateWindowW(L"BUTTON", L"Извлечь изображения",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 75, 340, 30,
        hWnd, (HMENU)IDC_EXTRACT_BTN, hInstance, nullptr);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        switch (wmId)
        {
        case IDC_BROWSE_BTN:
        {
            g_selectedFiles = OpenFileDialogMulti(hWnd);
            if (!g_selectedFiles.empty()) {
                if (g_selectedFiles.size() == 1) {
                    SetWindowTextW(hFilePathEdit, g_selectedFiles[0].c_str());
                }
                else {
                    std::wstring info = L"Выбрано файлов: " + std::to_wstring(g_selectedFiles.size());
                    SetWindowTextW(hFilePathEdit, info.c_str());
                }
            }
        }
        break;

        case IDC_EXTRACT_BTN:
        {
            if (g_selectedFiles.empty())
            {
                MessageBox(hWnd, L"Пожалуйста, выберите файлы!", L"Предупреждение", MB_ICONWARNING);
                break;
            }

            int ok = 0, failed = 0;
            for (const auto& file : g_selectedFiles) {
                if (ProcessFile(hWnd, file))
                    ++ok;
                else
                    ++failed;
            }

            if (g_selectedFiles.size() > 1) {
                std::wstring summary = L"Обработано успешно: " + std::to_wstring(ok)
                    + L"\nС ошибками: " + std::to_wstring(failed);
                MessageBox(hWnd, summary.c_str(), L"Итог", MB_ICONINFORMATION);
            }
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
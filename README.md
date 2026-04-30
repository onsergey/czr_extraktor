# CZR Scan Extractor to PNG / Извлечение PNG из CZR

## English Description

**CZR Scan Extractor to PNG** is a Windows desktop application designed to extract embedded PNG images from CZR scanned files.

### Features
- File selection dialog with path display
- Automatic detection of PNG markers within CZR files
- One-click PNG extraction and saving
- Error handling and user notifications
- Simple and intuitive user interface

### Version
**1.0**

### Requirements
- Windows 7 or later
- Visual Studio 2015 or later (for compilation)
- C++11 or later

### How to Use
1. Click "Обзор..." (Browse) button to select a CZR file
2. The file path will be displayed in the text field
3. Click "Записать PNG" (Write PNG) button
4. The PNG image will be extracted and saved as a separate file

### Project Structure
```
czr_extraktor/
├── czr_extraktor.cpp     - Main application source code
├── framework.h           - Windows framework includes
├── czr_extraktor.h       - Header file with resource definitions
└── README.md             - This file
```

---

## Русское описание

**CZR Scan Extractor to PNG** — это приложение для Windows, предназначенное для извлечения встроенных изображений PNG из отсканированных файлов формата CZR.

### Возможности
- Диалог выбора файла с отображением пути
- Автоматическое определение маркеров PNG в файлах CZR
- Одноклавишное извлечение и сохранение PNG
- Обработка ошибок и уведомления пользователю
- Простой и интуитивный интерфейс

### Версия
**1.0**

### Требования
- Windows 7 или позже
- Visual Studio 2015 или позже (для компиляции)
- C++11 или позже

### Как использовать
1. Нажмите кнопку "Обзор..." для выбора файла CZR
2. Путь к файлу отобразится в текстовом поле
3. Нажмите кнопку "Записать PNG"
4. PNG изображение будет извлечено и сохранено в отдельный файл

### Структура проекта
```
czr_extraktor/
├── czr_extraktor.cpp     - Исходный код приложения
├── framework.h           - Включаемые файлы Windows
├── czr_extraktor.h       - Заголовочный файл с определениями ресурсов
└── README.md             - Этот файл
```

### Технические детали

**PNG маркер:** Программа ищет магический байт PNG (`0x89 0x50 0x4E 0x47`) и извлекает все содержимое от маркера до конца файла.

**Входной формат:** CZR файл начинается с XML-кода, затем содержит маркер PNG и данные изображения.

**Выходной файл:** Сохраняется с тем же именем, но с расширением `.png`.

### Лицензия
Open Source

### Автор
onsergey

---

**Version 1.0** | CZR PNG Extractor | 2026
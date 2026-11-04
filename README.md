# CZR/CZI Scan Extractor to PNG/JPG | Извлечение PNG/JPG из CZR/CZI

## English Description

**CZR/CZI Scan Extractor to PNG/JPG** is a Windows desktop application designed to extract embedded PNG and JPG images from CZR/CZI scanned files.
### Features
- File selection dialog with path display
- Automatic detection of PNG/JPG markers in CZR/CZI files
- Multy choice for PNG/JPG and saving
- Error handling and user notifications
- Simple and intuitive user interface

### Version
**1.2**

### Requirements
- Windows 7 or later
- Visual Studio 2015 or later (for compilation)
- C++11 or later

### How to Use
1. Click "Обзор..." (Browse) button to select CZR or CZI files
2. The file path will be displayed in the text field
3. Click "Записать PNG/JPG" (Write PNG/JPG) button
4. The PNG/JPG image will be extracted and saved as a separate file

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

**CZR/CZI Scan Extractor to PNG/JPG** — это приложение для Windows, предназначенное для извлечения встроенных изображений PNG и JPG из отсканированных файлов формата CZR/CZI.

### Возможности
- Диалог выбора файла с отображением пути
- Автоматическое определение маркеров PNG/JPG в файлах CZR/CZI
- Мультивыбор файлов, извлечение и сохранение PNG/JPG изображений
- Обработка ошибок и уведомления пользователю
- Простой и интуитивный интерфейс

### Версия
**1.2**

### Требования
- Windows 7 или позже
- Visual Studio 2015 или позже (для компиляции)
- C++11 или позже

### Как использовать
1. Нажмите кнопку "Обзор..." для выбора файла CZR/CZI
2. Путь к файлу отобразится в текстовом поле
3. Нажмите кнопку "Записать"
4. PNG/JPG изображение будет извлечено и сохранено в отдельный файл c тем же именем, но с расширением `.png` или `.jpg`.

### Структура проекта
```
czr_extraktor/
├── czr_extraktor.cpp     - Исходный код приложения
├── framework.h           - Включаемые файлы Windows
├── czr_extraktor.h       - Заголовочный файл с определениями ресурсов
└── README.md             - Этот файл
```

### Технические детали

**PNG маркер:** 
// Поиск маркера PNG (‰PNG)
const unsigned char pngMagic[] = { 0x89, 0x50, 0x4E, 0x47 };

**JPEG маркер:** 
// Маркер начала JPEG (SOI + первый байт маркера): FF D8 FF
const unsigned char jpgStart[] = { 0xFF, 0xD8, 0xFF };
// Маркер конца JPEG (EOI): FF D9
const unsigned char jpgEnd[] = { 0xFF, 0xD9 };

**Входной формат:** CZR файл начинается с XML-кода, затем содержит маркер PNG и данные изображения.
**Входной формат:** CZI файл начинается с XML-кода, затем содержит маркер JPG, данные изображения, и заканчивается маркером JPG.

**Выходной файл:** Сохраняется с тем же именем, но с расширением `.png` или `.jpg` в зависимости от исходного формата изображения.

### Лицензия
Open Source

### Автор
onsergey

---

**Version 1.2** | CZR/CZI Extractor PNG/JPG | 2026
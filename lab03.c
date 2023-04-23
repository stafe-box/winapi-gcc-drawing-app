#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszClassName = "PaintApp";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("PaintApp", "Paint App", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static bool isDrawing = false;
    static int prevX, prevY;

    switch (msg)
    {
    case WM_CREATE:
        hdc = GetDC(hwnd);
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdcPaint = BeginPaint(hwnd, &ps);

            // Здесь можно нарисовать начальное изображение

            EndPaint(hwnd, &ps);
        }
        break;
    case WM_LBUTTONDOWN:
        isDrawing = true;
        prevX = LOWORD(lParam);
        prevY = HIWORD(lParam);
        break;
    case WM_MOUSEMOVE:
        if (isDrawing)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            HPEN hPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
            SelectObject(hdc, hPen);
            MoveToEx(hdc, prevX, prevY, NULL);
            LineTo(hdc, x, y);
            DeleteObject(hPen);

            prevX = x;
            prevY = y;
        }
        break;
    case WM_LBUTTONUP:
        isDrawing = false;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            // Очистка экрана
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            FillRect(hdc, &rcClient, (HBRUSH)(COLOR_WINDOW + 1));
        }
        else if (wParam == 'S')
        {
            // Сохранение изображения
            OPENFILENAMEA ofn = { 0 };
            char filename[MAX_PATH] = "";
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = "Bitmap Image (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = filename;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrDefExt = "bmp";
            ofn.Flags = OFN_OVERWRITEPROMPT;

            if (GetSaveFileNameA(&ofn))
            {
                HDC hdcMem = CreateCompatibleDC(hdc);
                HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 800, 600);
                SelectObject(hdcMem, hBitmap);

                BitBlt(hdcMem, 0, 0, 800, 600, hdc, 0, 0, SRCCOPY);

                BITMAPINFO bmi = { 0 };
                bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
                bmi.bmiHeader.biWidth = 800;
                bmi.bmiHeader.biHeight = 600;
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 24;
                bmi.bmiHeader.biCompression = BI_RGB;

                DWORD dwBmpSize = ((800 * bmi.bmiHeader.biBitCount + 31) / 32) * 4 * 600;
                BYTE* pData = (BYTE*)malloc(dwBmpSize);
                GetDIBits(hdcMem, hBitmap, 0, 600, pData, &bmi, DIB_RGB_COLORS);

                HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE)
                {
                    BITMAPFILEHEADER bmfh = { 0 };
                    bmfh.bfType = 0x4D42;
                    bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
                    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

                    DWORD bytesWritten;
                    WriteFile(hFile, &bmfh, sizeof(BITMAPFILEHEADER), &bytesWritten, NULL);
                    WriteFile(hFile, &bmi.bmiHeader, sizeof(BITMAPINFOHEADER), &bytesWritten, NULL);
                    WriteFile(hFile, pData, dwBmpSize, &bytesWritten, NULL);

                    CloseHandle(hFile);
                }

                free(pData);
                DeleteObject(hBitmap);
                DeleteDC(hdcMem);
            }
        }
        break;
    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }

    return 0;
}
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <gdiplus.h>
#include <wincrypt.h>
#include <strsafe.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "crypt32.lib")
using namespace Gdiplus;

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

const char* base64IconStr =
"iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAACXBIWXMAAAsTAAALEwEAmpwYAAAFlElEQVR4nO1Xa2xURRQeQERUrG1ntoD4woIS8bmdu+UR6967xQooYCxBpSq7Zy4v8REV/0iqxkR8JMZfYkKMiUZC/eEzJEi6Z5YCf6j4iATUKA9jCA+x3XtLA0HWnNlH19Jaomy7Nf2SyXbPvZ39zsx3vjPD2BCGMIR+QadTNZENZniOfGhQJ+E51tpkRC5ngxFts6rLfFse9hz5AhtsSDWy4Z5jbfIdK9VhW0vZYEKqvn6E58j1RN6z5ekTkeBVbLAgVVNzke9YTUTeJODId9mg0rxjJfLI70/WBDk9S9VVjvLDodm+bb3l29ZGzw7dzIoJ7Y6c7Dvypxx52+rww9YtR6dPH+M78nnfsY6mn8kNbTNnlrJiQkc4FPRt+Xse+ZO+I+u82qqpvmPtzdaCZ1srWbGhsyZ0jedYh7rIy9MdtlxA8eyq0250ONY8VozwHLk5S55G0qlaQXGSSi6piFzIihFe7bSA51hn8op2HcVTweBIz5GdGc3vZsWK9khoUn7RZouzw66+Im9X9rLi7rby10wCO3PxuspRxoWysrozOIMVK3xHrjFEbastVV09Oi/+ZtcuyGZW1J3XtvYYohGpuh/kchILyxtZsSI5K3iD78jjpHeSVTbuReSDuV0IW8+wYsYJR05LNzM5NxujZHzbOpJJIsqKHclIaIrvyC/JRrMx37aOkZSOR4IlA8uOMcYhHhRKr+aA7wnATVzh2kBM11au2jQq+05HjTXBd6puM3+HQ/d6jtxHR43uc5UrLQXoJqHwBwEY527ijoIRL3kELxeACaF0Kj3wTF8/aIo7bDUcq7MuM4Sj28YLF2cKhQ1lgBMqFm++xMQUrqc5OeCJUrelMPeGAGB1F/lcEkmu8FsOegsN2hFaUa70J+a70q0C9B6h8CBXujP7P8LV9d1SHSaU3pt5/lhBEjBJKIwIwE856KNnJ9PbwCQH/E4o3MhBN5ZHm6/vaW7RtburWX+Atp5HcZGIYoMAvJ9WVcT0fZRkWSxhlcX0FLECLz2XuUqWt5RypX0jIzc+p+DkJzy1YzTJJLPCDSSBf3p//Mot5QEX5/embw76dTMX6D2sES8oGPHcD7rxOX+TCeABAfoLofAdrvTbNITCDzK1sU8A/pkuUu1xVz/OGlO5RscB5wqlT3PAw4HY1n66YjamhnNARQVMbnSOtXBQAK4TbmIWc1tNjwhAfF5aOriRXIkNBDhsmRyI4eIA4DIB+jkO+hUaAvBZDolYwI0741w0l/p8iBhWCsBVY6PNV7OBAlc6SivIQX8sVuDYriepYUQ64OqbBOiHhdIfUuO7LLajbMDIsvqmERVRnMrdxO0Vy7YHKCQAj+fLxMjBxHqWFQd9iByqX3mXLG8pNZ0ySzb9+TkdH7jCRaZI+9A/FagAvUFA/K6eHGvskq2CK/1aX272r8CV/rrLbbSm9n/WO0v1pADoBdk6IG0b/cd0rXCbr+uLGAe9kOavcOOh856AcY8uCewjCZ2vuSuWbQ8I0C8JwFO0SwWpEVoVrvBIN9//SgC+wQEfELH4DNOgMtbYI+q/v5C7iXHk86Zrp0knhMKTGYvdRfWVn9h5TaI8um1MxiK/6VXr1KyoPtLjAFf4Mw0B2N7L++3k/+UxfU9+YyPbpS7PCgVqOKR3AfgiV/p9rvQ2qhNDGPShPGf6gyv8hVZXKGw2p1TAV8l6TcetbxqRP68AvJUr/ZEA3Ub3hIIlQKtV+mjiSpLFf52qAhLXcsAnyRzS1ou7+uc44baO5ErfzZV+gm5kdEzmLgKdkcpdrDKutKRlIg3quKaOYno2HfwE4MsC8DOu9P6MlE4ZW3Zxfr6U+hGpYYasi/O5i2vMFRH0TgH4o1D4G0kp7TJGXrs56BajfVc/TbezcW7rxQNAeghD+F/gL76ssK8TFCCXAAAAAElFTkSuQmCC";

HINSTANCE g_hInst = nullptr;
NOTIFYICONDATA g_nid = {};
UINT_PTR g_timerId = 0;
HWND g_hwnd = nullptr;

// Set your hardcoded interval (seconds)
const DWORD g_intervalSeconds = 60;

// Icon loader from PNG/base64
HICON CreateIconFromBase64()
{
    ULONG decodedSize = 0;
    if (!CryptStringToBinaryA(base64IconStr, 0, CRYPT_STRING_BASE64, NULL, &decodedSize, NULL, NULL))
        return LoadIcon(nullptr, IDI_APPLICATION);
    BYTE* decodedBytes = new BYTE[decodedSize];
    if (!CryptStringToBinaryA(base64IconStr, 0, CRYPT_STRING_BASE64, decodedBytes, &decodedSize, NULL, NULL)) {
        delete[] decodedBytes;
        return LoadIcon(nullptr, IDI_APPLICATION);
    }
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok) {
        delete[] decodedBytes;
        return LoadIcon(nullptr, IDI_APPLICATION);
    }
    IStream* pStream = SHCreateMemStream(decodedBytes, decodedSize);
    delete[] decodedBytes;
    if (!pStream) {
        GdiplusShutdown(gdiplusToken);
        return LoadIcon(nullptr, IDI_APPLICATION);
    }
    Bitmap* bitmap = Bitmap::FromStream(pStream);
    pStream->Release();
    if (!bitmap || bitmap->GetLastStatus() != Ok) {
        delete bitmap;
        GdiplusShutdown(gdiplusToken);
        return LoadIcon(nullptr, IDI_APPLICATION);
    }
    HICON hIcon = NULL;
    if (bitmap->GetHICON(&hIcon) != Ok)
        hIcon = NULL;
    delete bitmap;
    GdiplusShutdown(gdiplusToken);
    if (!hIcon)
        return LoadIcon(nullptr, IDI_APPLICATION);
    return hIcon;
}

void AddTrayIcon(HWND hwnd)
{
    ZeroMemory(&g_nid, sizeof(g_nid));
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = CreateIconFromBase64();
    StringCchCopy(g_nid.szTip, ARRAYSIZE(g_nid.szTip), L"Stealth Mouse Mover");
    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void RemoveTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    if (g_nid.hIcon)
        DestroyIcon(g_nid.hIcon);
    g_nid.hIcon = nullptr;
}

// Mouse mover logic (stealth)
void StealthMouseMove()
{
    POINT pt;
    GetCursorPos(&pt);
    SetCursorPos(pt.x + 1, pt.y);
    SetCursorPos(pt.x, pt.y);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            if (hMenu)
            {
                InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT, L"Exit");
                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
        }
        // Do nothing on left click, stealth mode!
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT)
            PostQuitMessage(0);
        break;
    case WM_TIMER:
        if (wParam == 1)
            StealthMouseMove();
        break;
    case WM_DESTROY:
        RemoveTrayIcon();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    g_hInst = hInstance;
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"StealthMouseMoverClass";
    RegisterClass(&wc);

    g_hwnd = CreateWindow(wc.lpszClassName, L"Stealth Mouse Mover", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr);
    if (!g_hwnd)
        return 0;

    AddTrayIcon(g_hwnd);
    g_timerId = SetTimer(g_hwnd, 1, g_intervalSeconds * 1000, nullptr);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    RemoveTrayIcon();
    return 0;
}

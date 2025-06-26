#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include "logikaWindy.h"
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;

#define BUTTON_ID_BASE 1000

Winda winda;
vector<Pasazer> pasazerowie;
vector<Pasazer> pasazerowieDoDodania;
int flipflop=0;


void DrawWinda(Graphics& g)
{
    SolidBrush windaBrush(Color(128, 128, 128));
    SolidBrush pasazerBrush(Color(0, 180, 0));
    SolidBrush czekaBrush(Color(255, 100, 100));
    SolidBrush dojechalBrush(Color(100, 100, 255));

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 12, FontStyleRegular, UnitPixel);
    SolidBrush tekstBrush(Color(0, 0, 0));
    Pen pen(Color(0, 0, 0));

    int marginY = 300;
    int wysokoscPietro = 50;
    int szerokoscWindy = 125;
    int wysokoscWindy = 50;
    int szerokoscPasazer = 10;
    int wysokoscPasazer = 20;
    int marginX = 50;
    int czekajacyX = marginX + 80;
    int windaX = czekajacyX + 100;
    int dojechaliX = windaX + szerokoscWindy + 30;
 

    wstring wagaStr = L"Waga: " + to_wstring(winda.waga) + L" kg";
    g.DrawString(wagaStr.c_str(), -1, &font, PointF(marginX, 10), &tekstBrush);

    for (int f = 0; f < 5; ++f) {
        int fy = marginY - f * wysokoscPietro;

        g.DrawLine(&pen, marginX, fy, dojechaliX + 100, fy);

        wstring pietroText = L"Pietro " + to_wstring(f);
        g.DrawString(pietroText.c_str(), -1, &font, PointF(marginX, fy - wysokoscPietro + 5), &tekstBrush);

        int count = 0;
        for (const Pasazer& p : pasazerowie) {
            if (p.stan == czeka && p.pietroStart == f) {
                int px = czekajacyX + (count % 8) * (szerokoscPasazer + 2);
                int py = fy - wysokoscPasazer - 5 - (count / 8) * (wysokoscPasazer + 2);
                g.FillRectangle(&czekaBrush, px, py, szerokoscPasazer, wysokoscPasazer);
                count++;
            }
        }

        count = 0;
        for (const Pasazer& p : pasazerowie) {
            if (p.stan == dojechal && p.pietroKoniec == f) {
                int px = dojechaliX + (count % 8) * (szerokoscPasazer + 2);
                int py = fy - wysokoscPasazer - 5 - (count / 8) * (wysokoscPasazer + 2);
                g.FillRectangle(&dojechalBrush, px, py, szerokoscPasazer, wysokoscPasazer);
                count++;
            }
        }
    }

    int wy = marginY - ((winda.pietro + 1) * wysokoscPietro);
    g.FillRectangle(&windaBrush, windaX, wy, szerokoscWindy, wysokoscWindy);

    int i = 0;
    for (auto* p : winda.vectorPasazerow) {
        int px = windaX + 5 + (i % 8) * (szerokoscPasazer + 5);
        int py = wy + wysokoscWindy - wysokoscPasazer - 5 - (i / 8) * (wysokoscPasazer + 2);
        g.FillRectangle(&pasazerBrush, px, py, szerokoscPasazer, wysokoscPasazer);
        i++;
    }
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 500, NULL);

        for (int start = 0; start < 5; ++start) {
            for (int cel = 0; cel < 5; ++cel) {
                if (cel == start) continue;
                wchar_t label[16];
                swprintf(label, 16, L"Do %d", cel);
                int btnId = BUTTON_ID_BASE + start * 10 + cel;

                CreateWindow(L"BUTTON", label,
                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    500+cel * 45, // x
                    300 - start * 50 -30, // y
                    40, 20, hWnd, (HMENU)btnId, NULL, NULL);
            }
        }
        return 0;

    case WM_COMMAND:
    {
        int btnId = LOWORD(wParam);
        if (btnId >= BUTTON_ID_BASE && btnId < BUTTON_ID_BASE + 500) {
            int floor = (btnId - BUTTON_ID_BASE) / 10;
            int cel = (btnId - BUTTON_ID_BASE) % 10;
            pasazerowieDoDodania.push_back(Pasazer(floor, cel));
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_TIMER:
        for (auto& p : pasazerowieDoDodania) {
            pasazerowie.push_back(p);
        }
        pasazerowieDoDodania.clear();
        for (Pasazer& p : pasazerowie) winda.wezwij(p);
        for (Pasazer& p : pasazerowie) winda.odbierz(p);
        winda.ruch();
        winda.odstaw();
        
        winda.pierwszyRuchJeœliPotrzeba();
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        {
            Graphics g(hdc);
            RECT winRect;
            GetClientRect(hWnd, &winRect);
            int width = winRect.right - winRect.left;
            int height = winRect.bottom - winRect.top;
            SolidBrush whiteBrush(Color(255, 255, 255));
            g.FillRectangle(&whiteBrush, Rect(0, 0, width, height));
            DrawWinda(g);
        }
        EndPaint(hWnd, &ps);
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SymulacjaWindy";

    RegisterClass(&wc);
    HWND hWnd = CreateWindowEx(WS_EX_COMPOSITED, L"SymulacjaWindy", L"Symulator Windy", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 400, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}
/* Color-fidelity probe: replicates the stub capture pipeline (GetDC,
 * CreateCompatibleDC(NULL), CreateCompatibleBitmap, SelectObject,
 * BitBlt SRCCOPY, GetDIBits 24bpp top-down) on a window with known
 * colored content; dumps caps + captured PPM. */
#include <windows.h>
#include <stdio.h>

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    PAINTSTRUCT ps;
    HDC dc;
    if (m == WM_PAINT) {
        dc = BeginPaint(h, &ps);
        {
            HBRUSH b;
            b = CreateSolidBrush(RGB(255, 0, 0));    FillRect(dc, &ps.rcPaint, b);            DeleteObject(b);
            b = CreateSolidBrush(RGB(0, 255, 0));    FillRect(dc, &(RECT){10, 10, 100, 100}, b);   DeleteObject(b);
            b = CreateSolidBrush(RGB(0, 0, 255));    FillRect(dc, &(RECT){120, 10, 220, 100}, b);  DeleteObject(b);
            b = CreateSolidBrush(RGB(255, 255, 0));  FillRect(dc, &(RECT){10, 120, 100, 200}, b);  DeleteObject(b);
            b = CreateSolidBrush(RGB(0, 0, 0));      FillRect(dc, &(RECT){120, 120, 220, 200}, b); DeleteObject(b);
        }
        EndPaint(h, &ps);
        return 0;
    }
    return DefWindowProc(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int s)
{
    WNDCLASS wc;
    HWND h;
    HDC wnddc, memdc;
    HBITMAP hbm, old;
    BITMAPINFO bmi;
    static char cap[300*250*3];
    static char cap32[300*250*4];
    int w = 300, hh = 250, bits, planes, k;
    FILE *f;

    memset(&wc, 0, sizeof wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = i;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = "CapProbe";
    RegisterClass(&wc);
    h = CreateWindowEx(0, "CapProbe", "probe", WS_OVERLAPPEDWINDOW,
                       0, 0, 400, 300, 0, 0, i, 0);
    ShowWindow(h, SW_SHOWNORMAL);
    UpdateWindow(h);
    Sleep(200);

    wnddc = GetDC(h);
    bits = GetDeviceCaps(wnddc, BITSPIXEL);
    planes = GetDeviceCaps(wnddc, PLANES);

    memdc = CreateCompatibleDC(0);
    hbm = CreateCompatibleBitmap(wnddc, w, hh);
    old = SelectObject(memdc, hbm);
    int bbrc = BitBlt(memdc, 0, 0, w, hh, wnddc, 0, 0, SRCCOPY);
    int bberr = GetLastError();
    BITMAP bm;
    GetObject(hbm, sizeof bm, &bm);
    COLORREF c1 = GetPixel(wnddc, 55, 55);
    COLORREF c2 = GetPixel(wnddc, 170, 55);
    COLORREF c3 = GetPixel(memdc, 55, 55);

    memset(&bmi, 0, sizeof bmi);
    bmi.bmiHeader.biSize = 40;
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = hh;      /* positive = top-down */
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    memset(cap, 0xEE, 300*250*3);
    int gdrc = GetDIBits(memdc, hbm, 0, hh, cap + 15, &bmi, DIB_RGB_COLORS);
    int gderr = GetLastError();

    memset(&bmi, 0, sizeof bmi);
    bmi.bmiHeader.biSize = 40;
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = hh;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    int gd32rc = GetDIBits(memdc, hbm, 0, hh, cap32 + 16, &bmi, DIB_RGB_COLORS);
    int gd32err = GetLastError();
    fprintf(f, "GetDIBits32 rc=%d err=%d\n", gd32rc, gd32err);
    {
        FILE *f32 = fopen("capprobe32.ppm", "wb");
        int x, y;
        fprintf(f32, "P6\n%d %d\n255\n", w, hh);
        for (y = 0; y < hh; y++)
            for (x = 0; x < w; x++) {
                char *p = cap32 + 16 + (y * w + x) * 4;
                fputc(p[2], f32); fputc(p[1], f32); fputc(p[0], f32); /* R,G,B */
            }
        fclose(f32);
    }

    f = fopen("capprobe.ppm", "wb");
    fprintf(f, "P6\n%d %d\n255\n", w, hh);
    fwrite(cap + 15, 1, (size_t)w * hh * 3, f);
    fclose(f);

    f = fopen("capprobe.txt", "wb");
    fprintf(f, "window DC: BITSPIXEL=%d PLANES=%d\n", bits, planes);
    fprintf(f, "BitBlt rc=%d err=%d\n", bbrc, bberr);
    fprintf(f, "GetObject hbm: w=%d h=%d planes=%d bpp=%d compress=%s\n",
            bm.bmWidth, bm.bmHeight, bm.bmPlanes, bm.bmBitsPixel,
            bm.bmBitsPixel ? (bmi.bmiHeader.biCompression == BI_RGB ? "BI_RGB" : "?") : "?");
    fprintf(f, "GetDIBits24 rc=%d err=%d\n", gdrc, gderr);
    {
        int n = 0;
        for (int q = 0; q < 300*250*3 && n < 12; q++)
            if (cap[q] != (char)0xEE) { fprintf(f, "cap[%d]=%d ", q, cap[q]); n++; }
        fprintf(f, "(others still 0xEE = unwritten)\n");
    }
    fprintf(f, "GetPixel(wnddc 55,55) = %06X (want 0000FF=green BGR? no: RGB macro -> 0x00BBGGRR = 00FF00)\n", c1);
    fprintf(f, "GetPixel(wnddc 170,55) = %06X (want 0000FF)\n", c2);
    fprintf(f, "GetPixel(memdc 55,55) = %06X\n", c3);
    {
        int xs[5] = {55, 170, 55, 170, 290};
        int ys[5] = {55, 55, 160, 160, 240};
        for (k = 0; k < 5; k++) {
            char *p = cap + 15 + (ys[k] * w + xs[k]) * 3;
            char *q = cap32 + 16 + (ys[k] * w + xs[k]) * 4;
            fprintf(f, "px(%d,%d) 24b=B%d G%d R%d | 32b=B%d G%d R%d A%d\n",
                    xs[k], ys[k], p[0], p[1], p[2], q[0], q[1], q[2], q[3]);
        }
    }
    fclose(f);
    ReleaseDC(h, wnddc);
    ExitProcess(0);
}

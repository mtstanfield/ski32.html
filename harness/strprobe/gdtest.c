/* GetDIBits path matrix: 32bpp bottom-up (raw copy?), 32bpp top-down,
 * 24bpp bottom-up, 24bpp top-down. Reports rc + 5 sample pixels each. */
#include <windows.h>
#include <stdio.h>

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    PAINTSTRUCT ps; HDC dc;
    if (m == WM_PAINT) {
        dc = BeginPaint(h, &ps);
        HBRUSH b;
        b = CreateSolidBrush(RGB(255, 0, 0));    FillRect(dc, &ps.rcPaint, b);        DeleteObject(b);
        b = CreateSolidBrush(RGB(0, 255, 0));    FillRect(dc, &(RECT){10, 10, 100, 100}, b);    DeleteObject(b);
        b = CreateSolidBrush(RGB(0, 0, 255));    FillRect(dc, &(RECT){120, 10, 220, 100}, b);   DeleteObject(b);
        b = CreateSolidBrush(RGB(255, 255, 0));  FillRect(dc, &(RECT){10, 120, 100, 200}, b);   DeleteObject(b);
        b = CreateSolidBrush(RGB(0, 0, 0));      FillRect(dc, &(RECT){120, 120, 220, 200}, b);  DeleteObject(b);
        EndPaint(h, &ps);
        return 0;
    }
    return DefWindowProc(h, m, w, l);
}

#define W 300
#define H 250

static void check(FILE *f, const char *name, BITMAPINFO *bmi, void *buf, int rc, int err)
{
    /* interpret buffer as BGR(A); sample 5 points; for bottom-up, row y
     * is at (H-1-y) in the buffer */
    int bpp = bmi->bmiHeader.biBitCount / 8;
    int td = bmi->bmiHeader.biHeight < 0;
    int pts[5][2] = {{55,55},{170,55},{55,160},{170,160},{290,240}};
    fprintf(f, "%-18s rc=%d err=%d | ", name, rc, err);
    for (int k = 0; k < 5; k++) {
        int x = pts[k][0], y = pts[k][1];
        int row = td ? y : (H - 1 - y);
        unsigned char *p = (unsigned char *)buf + (row * W + x) * bpp;
        fprintf(f, "[%d,%d]=%d,%d,%d ", x, y, p[2], p[1], p[0]); /* R,G,B */
    }
    fprintf(f, "\n");
}

int WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int s)
{
    WNDCLASS wc; HWND h; HDC wnddc, memdc; HBITMAP hbm;
    BITMAPINFO bmi; static unsigned char buf[4 * W * H];
    FILE *f;

    memset(&wc, 0, sizeof wc);
    /* match the game's class style exactly: 0x2023 */
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = i;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = "GdTest";
    RegisterClass(&wc);
    h = CreateWindowEx(0, "GdTest", "probe", WS_OVERLAPPEDWINDOW, 0, 0, 400, 300, 0, 0, i, 0);
    ShowWindow(h, SW_SHOWNORMAL);
    UpdateWindow(h);
    Sleep(300);

    wnddc = GetDC(h);
    memdc = CreateCompatibleDC(0);
    hbm = CreateCompatibleBitmap(wnddc, W, H);
    SelectObject(memdc, hbm);
    BitBlt(memdc, 0, 0, W, H, wnddc, 0, 0, SRCCOPY);

    f = fopen("gdtest.txt", "wb");
    fprintf(f, "BITSPIXEL(wnddc)=%d RASTERCAPS(wnddc)=%08x\n",
            GetDeviceCaps(wnddc, BITSPIXEL), GetDeviceCaps(wnddc, RASTERCAPS));
    {
        int idxs[] = {8,10,11,12,14,15,32,88,89,110,111};
        const char *nms[] = {"HORZRES","VERTRES","SIZEPALETTE","BITSPIXEL","PLANES","NUMCOLORS","RASTERCAPS","LOGPIXELSX","LOGPIXELSY","DESKTOPHORZRES","DESKTOPVERTRES"};
        for (int k = 0; k < 11; k++)
            fprintf(f, "cap %s (%d) = %x\n", nms[k], idxs[k], GetDeviceCaps(wnddc, idxs[k]));
    }

    /* A: 32bpp bottom-up (matches internal storage: raw copy expected) */
    memset(&bmi, 0, 40); bmi.bmiHeader.biSize = 40; bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = H; bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB;
    memset(buf, 0xEE, sizeof buf);
    int rc = GetDIBits(memdc, hbm, 0, H, buf, &bmi, DIB_RGB_COLORS);
    check(f, "32bu", &bmi, buf, rc, GetLastError());

    /* B: 32bpp top-down */
    memset(&bmi, 0, 40); bmi.bmiHeader.biSize = 40; bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = -H; bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB;
    rc = GetDIBits(memdc, hbm, 0, H, buf, &bmi, DIB_RGB_COLORS);
    check(f, "32td", &bmi, buf, rc, GetLastError());

    /* C: 24bpp bottom-up */
    memset(&bmi, 0, 40); bmi.bmiHeader.biSize = 40; bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = H; bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24; bmi.bmiHeader.biCompression = BI_RGB;
    rc = GetDIBits(memdc, hbm, 0, H, buf, &bmi, DIB_RGB_COLORS);
    check(f, "24bu", &bmi, buf, rc, GetLastError());

    /* D: 24bpp top-down (the in-game stub's current call) */
    memset(&bmi, 0, 40); bmi.bmiHeader.biSize = 40; bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = -H; bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24; bmi.bmiHeader.biCompression = BI_RGB;
    rc = GetDIBits(memdc, hbm, 0, H, buf, &bmi, DIB_RGB_COLORS);
    check(f, "24td", &bmi, buf, rc, GetLastError());

    /* E: screen-DC path (bypasses the window DC entirely) */
    {
        HDC sdc = GetDC(0);
        RECT wr, cr;
        GetWindowRect(h, &wr);
        GetClientRect(h, &cr);
        int cx = wr.left + (wr.right - wr.left - cr.right) / 2;
        int cy = wr.top + (wr.bottom - wr.top - cr.bottom);
        fprintf(f, "screen client origin=%d,%d\n", cx, cy);
        HBITMAP hbm2 = CreateCompatibleBitmap(sdc, W, H);
        HBITMAP old2 = SelectObject(memdc, hbm2);
        BitBlt(memdc, 0, 0, W, H, sdc, cx, cy, SRCCOPY);
        memset(&bmi, 0, 40); bmi.bmiHeader.biSize = 40; bmi.bmiHeader.biWidth = W;
        bmi.bmiHeader.biHeight = -H; bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24; bmi.bmiHeader.biCompression = BI_RGB;
        rc = GetDIBits(memdc, hbm2, 0, H, buf, &bmi, DIB_RGB_COLORS);
        check(f, "screen24td", &bmi, buf, rc, GetLastError());
        SelectObject(memdc, old2);
        DeleteObject(hbm2);
        ReleaseDC(0, sdc);
    }

    fclose(f);
    ReleaseDC(h, wnddc);
    ExitProcess(0);
}

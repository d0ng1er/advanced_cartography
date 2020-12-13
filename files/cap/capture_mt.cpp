// Copyright (c) 2020 Reese Danzer
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <iostream>
using namespace std;
#include <windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#include "jerror.h"
#include "turbojpeg.h"
#pragma comment(lib, "turbojpeg-static.lib")

//#define _WIN32_WINNT 0x0600


struct imgQueueItem
{
    const char* path;
    const unsigned char* lpbitmap;
    BITMAP bmpScreen;
    HANDLE hDIB;
};


extern "C" {
    bool WINAPI DllMain(
        HINSTANCE hinstDLL,
        DWORD fdwReason,
        LPVOID lpReserved )
    {
        switch( fdwReason ) 
        { 
            case DLL_PROCESS_ATTACH:
                /*PTP_POOL tPool = CreateThreadpool(NULL);
                if(!tPool)
                {
                    cerr << "AC Error: failed to create threadpool\n";
                    break;
                }
                SetThreadPoolThreadMinimum(tPool, 2)
                SetThreadPoolThreadMaximum(tPool, 20)*/
                //creating a threadpool might not be necessary,
                //it might just be fine running on default

                break;

            case DLL_THREAD_ATTACH:
                break;

            case DLL_THREAD_DETACH:
                break;

            case DLL_PROCESS_DETACH:
                break;
        }
        return true;
    }
}


VOID WINAPI compressfunc(
        PTP_CALLBACK_INSTANCE instance,
        PVOID imgvoid,
        PTP_WORK Wrk )
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(Wrk);
    imgQueueItem* img = (imgQueueItem*) imgvoid;

    tjhandle compressor = tjInitCompress();
    //Compress the image from BMP to JPG
    unsigned char *jpegBuf = NULL;
    unsigned long jpegSize = 0;
    int flag = 0;
    flag |= TJFLAG_ACCURATEDCT;
    if(tjCompress2(compressor,  // tjCompress returns 0 on success and -1 on failure
                   img->lpbitmap,
                   img->bmpScreen.bmWidth,
                   TJPAD(img->bmpScreen.bmWidth*tjPixelSize[TJPF_BGRX]),
                   img->bmpScreen.bmHeight,
                   TJPF_BGRX,
                   &jpegBuf,
                   &jpegSize,
                   TJSAMP_444,
                   100,
                   flag))
    {
        tjFree(jpegBuf);
        cerr << "AC Error: compression failed\n";
        cerr << tjGetErrorStr() << "\n";
        goto done;
    }

    // Make a file and write the JPG in
    HANDLE hFile = CreateFile(img->path,
                              GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);   
    if(!hFile)
    {
        cerr << "AC Error: file initialization error\n";
        CloseHandle(hFile);
        goto done;
    }

    DWORD dwBytesWritten = 0;
    if(!WriteFile(hFile,
                  (LPSTR)jpegBuf,
                  jpegSize,
                  &dwBytesWritten,
                  NULL))
    {
        cerr << "AC Error: file write error\n";
        CloseHandle(hFile);
        goto done;
    }

    done:
        HeapDestroy(img->hDIB);
        CloseHandle(hFile);
        tjFree(jpegBuf);
        tjDestroy(compressor);
        free(imgvoid);
}


// ADAPTED FROM:
// https://stackoverflow.com/questions/11705844/win32-create-bitmap-from-device-context-to-file-and-or-blob
extern "C" {
    __declspec(dllexport) void capture(
            const char *path,
            const int vResX,
            const int vResY )
    {
        HDC hdcWindow;
        HDC hdcMemDC = NULL;
        HBITMAP hbmScreen = NULL;
        BITMAP bmpScreen;
        imgQueueItem imgs;

        HWND gameWnd = GetForegroundWindow();
        //yes, this is terrible, for now I don't care.

        // Retrieve the handle to a display device context for the client 
        // area of the window. 
        hdcWindow = GetDC(gameWnd);

        // Create a compatible DC which is used in a BitBlt from the window DC
        hdcMemDC = CreateCompatibleDC(hdcWindow); 
        if(!hdcMemDC)
        {
            cerr << "AC Error: unable to initialize compatible device context\n";
            goto done;
        }
        RECT rcClient;
        GetClientRect(gameWnd, &rcClient);
        unsigned int rWidth = rcClient.right-rcClient.left;
        unsigned int rHeight = rcClient.bottom-rcClient.top;

        hbmScreen = CreateCompatibleBitmap(hdcWindow, vResX, vResY);
        if(!hbmScreen)
        {
            cerr << "AC Error: unable to initialize compatible bitmap\n";
            goto done;
        }

        SetStretchBltMode(hdcMemDC, COLORONCOLOR);
        SelectObject(hdcMemDC,hbmScreen);

        if(!StretchBlt(
                    hdcMemDC, 
                    0,0, 
                    vResX, vResY,
                    hdcWindow, 
                    0, rHeight-1,  //needs to be flipped vertically because BMP
                    rWidth, -rHeight, 
                    SRCCOPY ))
        {
            cerr << "AC Error: unable to transform bitmap\n";
            goto done;
        }
        GetObject(hbmScreen,sizeof(BITMAP),&bmpScreen);

        BITMAPFILEHEADER   bmfHeader;    
        BITMAPINFOHEADER   bi;

        bi.biSize = sizeof(BITMAPINFOHEADER);    
        bi.biWidth = bmpScreen.bmWidth;    
        bi.biHeight = bmpScreen.bmHeight;  
        bi.biPlanes = 1;    
        bi.biBitCount = 32;    

        bi.biCompression = BI_RGB;    
        bi.biSizeImage = 0;  
        bi.biXPelsPerMeter = 0;    
        bi.biYPelsPerMeter = 0;    
        bi.biClrUsed = 0;    
        bi.biClrImportant = 0;

        DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * 
                          bmpScreen.bmHeight;

        HANDLE hDIB = HeapCreate(0, dwBmpSize, dwBmpSize*2); 
        if(!hDIB)
        {
            cerr << "AC Error: bmp heap creation failed\n";
            goto done;
        }

        char *lpbitmap = (char *)HeapAlloc(hDIB, 0, dwBmpSize);    
        if(!lpbitmap)
        {
            cerr << "AC Error: bmp heap allocation failed\n";
            HeapDestroy(hDIB);
            goto done;
        }       
        // Gets the "bits" from the bitmap and copies them into a buffer 
        // which is pointed to by lpbitmap.
        GetDIBits(hdcWindow, hbmScreen, 0,
                  (UINT)bmpScreen.bmHeight,
                  lpbitmap,
                  (BITMAPINFO *)&bi, DIB_RGB_COLORS);
        
        PTP_WORK_CALLBACK work = compressfunc;
        imgs = {path, (const unsigned char*)lpbitmap, bmpScreen, hDIB};
        unsigned int size = sizeof(imgs);
        PVOID img = malloc(size);
        if(!img)
        {
            cerr << "AC Error: image data structure allocation failed.";
            goto done;
        }
        memmove(img, (const void*)&imgs, size);

        PTP_WORK wk = CreateThreadpoolWork(work, img, nullptr);
        if(!wk)
        {
            cerr << "AC Error: work allocation failed\n";
            goto done;
        }
        SubmitThreadpoolWork(wk);
        
        done:
            DeleteObject(hbmScreen);
            DeleteObject(hdcMemDC);
            ReleaseDC(gameWnd,hdcWindow);
    }
}

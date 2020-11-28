#include <iostream>
using namespace std;
#include <windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#include "jerror.h"
#include "turbojpeg.h"
#pragma comment(lib, "turbojpeg-static.lib")

#ifdef __cplusplus
extern "C" {
#endif

tjhandle compressor;


bool WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
            compressor = tjInitCompress();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            tjDestroy(compressor);
            break;
    }
    return true;  // Successful DLL_PROCESS_ATTACH.
}


bool EnumWindowsPro(
        HWND hWnd,
        LPDWORD tpAddress
        )
{
    //cout << "at callback function\n";
    DWORD testProc = 0;
    GetWindowThreadProcessId(hWnd, &testProc);
    if(*tpAddress == testProc)
    {
        *tpAddress = (DWORD)hWnd;  //move currently iterating HWND to tempParam's adress
        return false;  //Stop EnumWindows iteration on successful match
    }
    return true; //Continue iterating if no match
}


HWND GetProcHwnd()
{
    DWORD pID = GetCurrentProcessId();
    DWORD tempParam = pID;
    EnumWindows(
            reinterpret_cast<WNDENUMPROC>(&EnumWindowsPro),
            (LPARAM)&tempParam
            );
    if(tempParam == pID)
    { 
        return 0;
    }
    return (HWND)tempParam; // now an HWND
}


// ADAPTED FROM:
// https://stackoverflow.com/questions/11705844/win32-create-bitmap-from-device-context-to-file-and-or-blob
__declspec(dllexport) void capture(const char *fullPath)
{
    HDC hdcWindow;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;
	HWND mainWind = GetProcHwnd();

    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    hdcWindow = GetDC(mainWind);

    // Create a compatible DC which is used in a BitBlt from the window DC
    hdcMemDC = CreateCompatibleDC(hdcWindow); 
    if(!hdcMemDC)
    {
        goto done;
    }
    RECT rcClient;
    GetClientRect(mainWind, &rcClient);
    hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right-rcClient.left, 
                                       rcClient.bottom-rcClient.top);
    if(!hbmScreen)
    {
        goto done;
    }

    SelectObject(hdcMemDC,hbmScreen);

    if(!BitBlt(hdcMemDC, 
               0,0, 
               rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, 
               hdcWindow, 
               0,0,
               SRCCOPY))
    {
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

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize); 
    char *lpbitmap = (char *)GlobalLock(hDIB);    
    
    // Gets the "bits" from the bitmap and copies them into a buffer 
    // which is pointed to by lpbitmap.
    GetDIBits(hdcWindow, hbmScreen, 0,
              (UINT)bmpScreen.bmHeight,
              lpbitmap,
              (BITMAPINFO *)&bi, DIB_RGB_COLORS);
	
    /*---------------ASYNC COMPRESSION STARTS HERE (I THINK)-----------------------*/

    // Add the size of the headers to the size of the bitmap to get the total file size
    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + 
                        sizeof(BITMAPINFOHEADER);

    //Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + 
                          (DWORD)sizeof(BITMAPINFOHEADER); 

    //Size of the file
    bmfHeader.bfSize = dwSizeofDIB; 

    //bfType must always be BM for Bitmaps
    bmfHeader.bfType = 0x4D42; //BM   

    // A file is created, this is where we will save the screen capture.
	HANDLE hFile = CreateFile(fullPath,
                              GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);   

    DWORD dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten,  NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    //Unlock and Free the DIB from the heap
    GlobalUnlock(hDIB);    
    GlobalFree(hDIB);

    //Close the handle for the file that was created
    CloseHandle(hFile);

    //Clean up
    done:
        DeleteObject(hbmScreen);
        DeleteObject(hdcMemDC);
        ReleaseDC(mainWind,hdcWindow);
}


#ifdef __cplusplus
}
#endif

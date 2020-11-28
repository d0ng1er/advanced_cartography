#include <iostream>
#include <cerrno>
#include <fstream>
#include <string>
#include <windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#include "turbojpeg.h"
#include "jerror.h"
#pragma comment(lib, "turbojpeg-static.lib")
#ifdef __cplusplus
extern "C" {
#endif
using namespace std;

#define DllExport  __declspec(dllexport)

tjhandle compressor;


BOOL DllMain (
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
            compressor = tjInitCompress();
            break;
        case DLL_PROCESS_DETACH:
            tjDestroy(compressor);
            free(compressor);
            break;
        default:
            return FALSE;  //this dll shouldn't get called any other way           free(compressor);
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}


BOOL EnumWindowsPro(
        HWND hWnd,
        LPDWORD tpAddress
        )
{
    //cout << "at callback function\n";
    DWORD testProc = 0;
    GetWindowThreadProcessId(hWnd, &testProc);
    if(*tpAddress == testProc)
    {
        //cout << "callback match\n";
        *tpAddress = (DWORD)hWnd;  //move currently iterating HWND to tempParam's adress
        return FALSE;  //Stop EnumWindows iteration on successful match
    }
    //cout << "callback no match\n";
    return TRUE; //Continue iterating if no match

}


HWND GetProcHwnd()
{
    //cout << "GetProcHwnd started\n";
    DWORD pID = GetCurrentProcessId();
    DWORD tempParam = pID;
    EnumWindows(
            reinterpret_cast<WNDENUMPROC>(&EnumWindowsPro),
            (LPARAM)&tempParam
            );
    //cout << "made it out of EnumWindows\n";
    if(tempParam == pID)
    { 
        //cout << "tempParam and pID are the same!\n";
        return 0; }
    //cout << "GetProcHwnd complete\n";
    return (HWND)tempParam; // now an HWND
}


BOOL GetRect(RECT* rectAddress)
{
    HWND hWnd = GetProcHwnd();
    if(!hWnd){ return FALSE; }

    if(!rectAddress){ return FALSE; }

    if(!GetClientRect(hWnd, rectAddress)){ return FALSE; }

    ClientToScreen(hWnd, (POINT*)rectAddress->left);
    ClientToScreen(hWnd, (POINT*)rectAddress->right);
    return TRUE;
}


DllExport BOOL Capture(
        int x,  // Virtual X Coord from game
        int y,  // Same for Y
        const char path  // Output path
        )
{
    HWND mainWnd = GetProcHwnd();
    if(!mainWnd){ return FALSE; }
    //cout << "mainWnd is TRUE\n";

    RECT rect;
    if(!GetRect(&rect)){ return FALSE; }
    //cout << "GetRect is TRUE\n";
    unsigned int imWidth = rect.right-rect.left;
    unsigned int imHeight = rect.bottom-rect.top;
    //cout << imWidth << "\n";
    //cout << imHeight << "\n";

    HDC srchdc = GetDC(mainWnd);
    if(!srchdc){ return FALSE; }
    //cout << "GetDC is TRUE\n";
    
    HDC comhdc = CreateCompatibleDC(srchdc);
    if(!comhdc){ return FALSE; }
    //cout << "comHDC is TRUE\n";

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = static_cast<LONG>(imWidth);
    bmi.bmiHeader.biHeight = static_cast<LONG>(imHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    LPVOID pBits;

    HBITMAP combmp = CreateDIBSection(
            comhdc,
            &bmi,
            DIB_RGB_COLORS,
            &pBits,
            NULL,
            0
            );
    if(!combmp)
    {
        //cout << "comBMP is FALSE!\n";
        return FALSE;
    }
    //cout << "comBMP is TRUE\n";

    HGDIOBJ chunky = SelectObject(comhdc, combmp);
    if(!chunky){ return FALSE; }

    if(!BitBlt(
                comhdc,
                0, 0,
                imWidth, imHeight,
                srchdc,
                0, 0,
                SRCCOPY))
    {
        ReleaseDC(mainWnd, srchdc);
        return FALSE;
        //cout << "BitBlt did not work\n";
    }
    //cout << "BitBlt Worked\n";
    ReleaseDC(mainWnd, srchdc);
    
    unsigned char *jpegBuf = NULL;
    unsigned long jpegSize = 0;
    int flag = 0;
    flag |= TJFLAG_FASTDCT;

    if(tjCompress2(
                compressor,
                static_cast<unsigned char*>(pBits),
                imWidth,
                TJPAD(imWidth * tjPixelSize[TJPF_BGR]),
                imHeight,
                TJPF_RGB,
                &jpegBuf,
                &jpegSize,
                TJSAMP_444,
                100,
                flag
                ))
    {
        cout << "compression Failed\n";
        cout << tjGetErrorStr2(compressor) << "\n";
        cout << "Source Pointer: " << pBits << "\n";
        cout << "Image Width: " << imWidth << "\n";
        cout << "TJ Padding: " << TJPAD(imWidth * tjPixelSize[TJPF_BGR]) << "\n";
        cout << "Image Height: " << imHeight << "\n";
        cout << "TJPF_BGR: " << TJPF_BGR << "\n";
        cout << "Output Buffer Adress: " << &jpegBuf << "\n";
        cout << "Output Size: " << jpegSize << "\n";
        cout << "Chroma Subsampling: " << TJSAMP_444 << "\n";
        cout << "Quality: " << 100 << "\n";
        cout << "Flag(s): " << flag << "\n";
        return FALSE;
    }
    //cout << "compression Worked\n";
    //cout << "Size in Memory: " << jpegSize << " Bytes\n";

    ofstream fout;
    fout.open("test.jpg");
    fout.write((char*)jpegBuf, jpegSize);
    fout.close();

    tjFree(jpegBuf);
    free(&jpegSize);    
    DeleteDC(comhdc);

    return TRUE;
}

#ifdef __cplusplus
}
#endif

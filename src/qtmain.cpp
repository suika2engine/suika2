#include "mainwindow.h"

extern "C" {
#include "suika.h"
#include "glrender.h"
};

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize.
    init_locale_code();
    if(!init_file())
        abort();
    if(!init_conf())
        abort();

    // Create the main window and run app.
    MainWindow w;
    w.show();
    int ret = a.exec();

    // Cleanup.
    cleanup_conf();
    cleanup_file();

    return ret;
}

//
// Windows specific functions
//
#ifdef WIN
#include <windows.h>
extern "C" {
const wchar_t *conv_utf8_to_utf16(const char *utf8_message)
{
    const int CONV_SIZE = 65536;
    static wchar_t utf16buf[CONV_SIZE];
    assert(utf8_message != NULL);
    MultiByteToWideChar(CP_UTF8, 0, utf8_message, -1, utf16buf, CONV_SIZE - 1);
    return utf16buf;
}
const char *conv_utf16_to_utf8(const wchar_t *utf16_message)
{
    const int CONV_SIZE = 65536;
    static char utf8buf[CONV_SIZE];
    assert(utf16_message != NULL);
    WideCharToMultiByte(CP_UTF8, 0, utf16_message, -1, utf8buf, CONV_SIZE - 1, NULL, NULL);
    return utf8buf;
}
}; // extern "C"
#endif

//
// mingw cross compilation
//
#if defined(WIN) && defined(MINGW_CROSS)
extern "C" {
#include <dxgi1_3.h>
__asm__(".global CreateDXGIFactory2@12");
HRESULT CreateDXGIFactory2(UINT Flags, REFIID riid, void **ppFactory)
{
    return CreateDXGIFactory1(riid, ppFactory);
}
};
#endif

#include "mainwindow.h"

extern "C" {
#include "suika.h"
#include "glrender.h"
#if defined(WIN) && !defined(NO_SOUND)
#include "dsound.h"
#elif defined(OSX) && !defined(NO_SOUND)
#include "aunit.h"
#elif defined(LINUX) && !defined(NO_SOUND)
#include "asound.h"
#endif
};

#include <QApplication>

#include <stdlib.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize.
    init_locale_code();
    if(!init_file())
        abort();
    if(!init_conf())
        abort();
#if defined(WIN) && !defined(NO_SOUND)
    if(!init_dsound())
        abort();
#elif defined(OSX) && !defined(NO_SOUND)
    if(!init_aunit())
        abort();
#elif defined(LINUX) && !defined(NO_SOUND)
    // Avoid using sound on WSL2.
    if (system("grep -i Microsoft /proc/version") != 0) {
        if(!init_asound())
            abort();
    }
#endif

    // Create the main window and run app.
    MainWindow w;
    w.show();
    int ret = a.exec();

    // Cleanup.
#if defined(WIN) && !defined(NO_SOUND)
    DSCleanup();
#elif defined(OSX) && !defined(NO_SOUND)
    cleanup_aunit();
#elfi defined(LINUX) && !defined(NO_SOUND)
    cleanup_asound();
#endif
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

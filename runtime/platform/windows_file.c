#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include "../../include/builtins.h"

Value platform_ask_file() {
    char filename[MAX_PATH] = "";

    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.txt\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        return make_text(filename);
    }

    return make_text("");
}

Value platform_save_file(const char* default_name) {
    char filename[MAX_PATH] = "";
    if (default_name) {
        strncpy(filename, default_name, MAX_PATH-1);
    }

    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = "txt";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        return make_text(filename);
    }

    return make_text("");
}

Value platform_get_camera() {
    // Simplified - in real app would use Windows.Media.Capture
    return make_text("C:\\fakepath\\photo.jpg");
}
#include "../../include/builtins.h"
#import <Cocoa/Cocoa.h>

Value platform_ask_file() {
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    [openDlg setCanChooseFiles:YES];
    [openDlg setCanChooseDirectories:NO];
    [openDlg setAllowsMultipleSelection:NO];

    if ([openDlg runModal] == NSModalResponseOK) {
        NSURL* fileURL = [openDlg.URLs firstObject];
        NSString* path = [fileURL path];
        return make_text([path UTF8String]);
    }

    return make_text("");
}

Value platform_save_file(const char* default_name) {
    NSSavePanel* saveDlg = [NSSavePanel savePanel];

    if (default_name) {
        NSString* filename = [NSString stringWithUTF8String:default_name];
        [saveDlg setNameFieldStringValue:filename];
    }

    if ([saveDlg runModal] == NSModalResponseOK) {
        NSURL* fileURL = saveDlg.URL;
        NSString* path = [fileURL path];
        return make_text([path UTF8String]);
    }

    return make_text("");
}

Value platform_get_camera() {
    // Simplified - in real app would use AVFoundation
    return make_text("/tmp/camera_photo.jpg");
}
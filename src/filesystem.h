#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "main.h"

typedef struct {
    /* os depdendent config directory */
    char *config_dir;
    /* modulo_dir -> config_dir/modulo */
    char *modulo_dir;
    /* modulo_json_filepath -> config_dir/modulo/modulo.json */
    char *modulo_json_filepath;
    char *user_env_var;
    char path_separator;
} OSContext;

// app dir
#define APP_DATA_DIR "modulo"
// app json filename
#define APP_DATA_FILENAME "modulo.json"
/*

OS depdendent app data directories

Linux: $XDG_CONFIG_HOME or "$HOME/.config"
Windows: %APPDATA% or "C:\\Users\\%USER%\\AppData\\Roaming"
macOS: $HOME/Library/Application Support

*/

typedef enum {
    OS_LINUX,
    OS_WINDOWS,
    OS_MACOS,
    OS_UNKNOWN
} OS;

#if defined(__linux__)
    #define CURRENT_OS OS_LINUX
#elif defined(_WIN32)
    #define CURRENT_OS OS_WINDOWS
#elif defined(__APPLE__)
    #define CURRENT_OS OS_MACOS
#else
    #define CURRENT_OS OS_UNKNOWN
#endif

// load program data from disk
Modulo *load_modulo(OSContext *c);
// write program data to disk
int save_modulo(Modulo *modulo, OSContext *c);

// read text data from disk
char *read_text_data(char *filepath);
// write text data to disk
int write_text_data(char *text, char *filepath);

OSContext *get_context();

char *get_system_username(OSContext *c);

#endif
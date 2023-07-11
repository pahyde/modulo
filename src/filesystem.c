#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <cjson/cJSON.h>

#include "filesystem.h"
#include "util.h"

/*
    Loads Modulo struct from config_dir/modulo.json file if it exists
    Otherwise returns NULL
*/
Modulo *load_modulo(OSContext *c) {
    char *filepath = c->modulo_json_filepath;
    char *json_str = read_text_data(filepath);
    if (json_str == NULL) {
        return NULL;
    } 
    // parse json string
    cJSON *json = cJSON_Parse(json_str);
    // json to Modulo
    Modulo *modulo = json_to_modulo(json);
    return modulo;
}

/*
    Saves the Modulo struct as json in config_dir/modulo.json 
    Creates the necessary directories and files if config_dir/modulo.json doesn't exist
*/
int save_modulo(Modulo *modulo, OSContext *c) {
    // Modulo to json
    cJSON *json = modulo_to_json(modulo);
    // serialize json string
    char *json_str = cJSON_Print(json_str);

    char *filepath = c->modulo_json_filepath;
    if (write_text_data(json_str, filepath) == -1) {
        // filepath doesn't exist
        // create config_dir/modulo/modulo.json
        create_modulo_dir(c);
        if (write_text_data(json_str, filepath) == -1) {
            return -1;
        }
    }
    free(json_str);
    return 0;
}

OSContext *get_context() {
    OS os = CURRENT_OS;
    char *config_dir;
    char *user_env_var = "USER";
    char separator = '/';
    switch (os) {
        case OS_WINDOWS:
            config_dir = getenv("APPDATA");
            user_env_var = "USERNAME";
            separator = '\\';
            break;
        case OS_MACOS:
            char *macos_home = getenv("HOME"); 
            config_dir = path_join(macos_home, "Library/Application Data", separator);
            break;
        case OS_LINUX:
            char *linux_home = getenv("HOME"); 
            config_dir = path_join(linux_home, ".config", separator);
            break;
        default:
            printf("Unknown operating system detected\n");
            printf("Program data will be written to the current directory.\n");
            exit(-1);
    }
    char *modulo_dir = path_join(config_dir, "modulo", separator);
    char *filepath = path_join(modulo_dir, "modulo.json", separator);
    OSContext *c = malloc(sizeof(OSContext));
    c->config_dir = config_dir;
    c->modulo_dir = modulo_dir;
    c->modulo_json_filepath = filepath;
    c->user_env_var = user_env_var;
    c->path_separator = separator;
    return c;
}

/*
    Reads modulo data from user ~/.config directory

    returns pointer to Modulo struct
*/
char *read_text_data(char *filepath) {
    // read_text_from_file(filepath)
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        if (errno == ENOENT) {
            // filepath doesn't exist
            return NULL;
        } else {
            // unknown error
            fprintf(stderr, "Unknown error occurred while opening %s\n", filepath);
        }
    }
    int size = 0;
    size_t capacity = 1024; 
    char *text = malloc(capacity);

    char buffer[1024];
    while (fgets(buffer, sizeof buffer, fp) != NULL) {
        if (size + 1024 > capacity) {
            capacity *= 2;
            realloc(text, capacity);
        }
        strcpy(text + size, buffer);
        size += strlen(buffer);
    } 
    fclose(fp);
    return text;
}

/*
    Writes modulo data to user ~/.config directory

    returns -1 if the write fails. returns 0 otherwise.
*/
int write_text_data(char *text, char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (fp == NULL) {
        if (errno == ENOENT) {
            return -1;
        } else {
            // unknown error
            fprintf(stderr, "Unknown error occurred while opening %s\n", filepath);
        }
    }
    if (fputs(text, filepath) == EOF) {
        perror("Failed to write text to file\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

char *get_system_username(OSContext *c) {
    return getenv(c->user_env_var);
}

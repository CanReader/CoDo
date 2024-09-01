#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string.h>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#include <Windows.h>
#endif

#define WIN_INIT_W 1280
#define WIN_INIT_H 720

#define BG_COLOR (LfColor){31, 31, 31, 255}
#define TEXT_COLOR (LfColor){180, 180, 180, 255}
#define BTN_COLOR (LfColor){165, 117, 255, 255} 

char DIR[MAX_PATH];

#define FONT "Assets\\fonts\\inter.ttf"
#define FONT_BOLD "Assets\\fonts\\inter-bold.ttf"

#define TODO_DATA_FILE "Data\\tod.dat"

#define BACK_ICON "Assets\\textures\\back.png"
#define REMOVE_ICON "Assets\\textures\\remove.png"
#define RAISE_ICON "Assets\\textures\\raise.png"

#define GLOBAL_MARGIN 25.0f

#define INPUT_BUF_SIZE 512

#define DA_INIT_CAP 64 

#define SMOOTH_SCROLL false

#define DATE_CMD "date +\"%d.%m.%Y, %H:%M\""

typedef enum {
    FILTER_ALL = 0,
    FILTER_IN_PROGRESS,
    FILTER_COMPLETED,
    FILTER_LOW,
    FILTER_MEDIUM,
    FILTER_HIGH
} todo_filter;

typedef enum {
    TAB_DASHBOARD = 0,
    TAB_NEW_TASK
} tab;

typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_MEDIUM,
    PRIORITY_HIGH,
    PRIORITY_COUNT
} entry_priority;

typedef struct {
    bool completed;
    char* desc, * date;

    entry_priority priority;
} todo_entry;

typedef struct {
    todo_entry** entries;
    uint32_t count, cap;
} entries_da;

typedef struct {
    GLFWwindow* win;
    int32_t winw, winh;

    todo_filter crnt_filter;
    tab crnt_tab;
    entries_da todo_entries;

    LfFont titlefont, smallfont;

    LfInputField new_task_input;
    char new_task_input_buf[INPUT_BUF_SIZE];
    LfTexture backicon, removeicon, raiseicon;

    FILE* serialization_file;

    char tododata_file[128];
} state;

static void         resizecb(GLFWwindow* win, int32_t w, int32_t h);
static void         rendertopbar();
static void         renderfilters();
static void         renderentries();

static void         InitializeWindow();
static void         InitializeUI();
static void         InitializeData();
static void         Destroy();

static void         renderdashboard();
static void         rendernewtask();

static void         entries_da_init(entries_da* da);
static void         entries_da_resize(entries_da* da, int32_t new_cap);
static void         entries_da_push(entries_da* da, todo_entry* entry);
static void         entries_da_remove_i(entries_da* da, uint32_t i);
static void         entries_da_free(entries_da* da);

static int          compare_entry_priority(const void* a, const void* b);
static void         sort_entries_by_priority(entries_da* da);

static void         serialize_todo_entry(FILE* file, todo_entry* entry);
static void         serialize_todo_list(const char* filename, entries_da* da);
static todo_entry*  deserialize_todo_entry(FILE* file);
static void         deserialize_todo_list(const char* filename, entries_da* da);

static char*        get_command_output(const char* cmd);
static void         print_requires_argument(const char* option, uint32_t numargs);

static void str_to_lower(char* str) {
    while (*str) {
        *str = tolower((unsigned char)*str);
        str++;
    }
}

static state s;

#endif
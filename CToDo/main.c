#include <GLFW/glfw3.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <leif/leif.h>
#include <stdint.h>
#include <string.h>

#include "config.h"

#ifdef _RELEASE
#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow){
#else
int main() {
#endif
#else
int main() {
#endif 

#ifdef _WIN32
    GetCurrentDirectoryA(sizeof(DIR), DIR);
#elif
    _getcwd(DIR, sizeof(DIR));
#endif

    InitializeWindow();
    InitializeUI();

    vec4s bgcol = lf_color_to_zto(BG_COLOR);
    while (!glfwWindowShouldClose(s.win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bgcol.r, bgcol.g, bgcol.b, bgcol.a);

        lf_begin();

        lf_div_begin(((vec2s) { GLOBAL_MARGIN, GLOBAL_MARGIN }), ((vec2s) { s.winw - GLOBAL_MARGIN * 2.0f, s.winh - GLOBAL_MARGIN * 2.0f }), true);

        switch (s.crnt_tab) {
        case TAB_DASHBOARD:
            renderdashboard();
            break;
        case TAB_NEW_TASK:
            rendernewtask();
            break;
        }

        lf_div_end();
        lf_end();

        glfwPollEvents();
        glfwSwapBuffers(s.win);
    }
    Destroy();

    return EXIT_SUCCESS;
}

//Events
void resizecb(GLFWwindow* win, int32_t w, int32_t h) {
    s.winw = w;
    s.winh = h;
    lf_resize_display(w, h);
    glViewport(0, 0, w, h);
}

//Initialize functions
void InitializeWindow() {
    glfwInit();

    s.winw = WIN_INIT_W;
    s.winh = WIN_INIT_H;

    s.win = glfwCreateWindow(s.winw, s.winh, "Task Manager", NULL, NULL);
    glfwMakeContextCurrent(s.win);
    glfwSetFramebufferSizeCallback(s.win, resizecb);
    lf_init_glfw(s.winw, s.winh, s.win);

}

void InitializeUI() {
    s.titlefont = lf_load_font(FONT_BOLD, 40);
    s.smallfont = lf_load_font(FONT, 20);

    s.crnt_filter = FILTER_ALL;

    LfTheme theme = lf_get_theme();
    theme.div_props.color = LF_NO_COLOR;
    lf_free_font(&theme.font);
    theme.font = lf_load_font(FONT, 24);
    theme.scrollbar_props.corner_radius = 2;
    theme.scrollbar_props.color = lf_color_brightness(BG_COLOR, 3.0);
    theme.div_smooth_scroll = SMOOTH_SCROLL;
    lf_set_theme(theme);

    memset(s.new_task_input_buf, 0, INPUT_BUF_SIZE);
    s.new_task_input = (LfInputField){
      .width = 400,
      .buf = s.new_task_input_buf,
      .buf_size = INPUT_BUF_SIZE,
      .placeholder = (char*)"What is there to do?"
    };

    s.backicon = lf_load_texture(BACK_ICON, true, LF_TEX_FILTER_LINEAR);
    s.removeicon = lf_load_texture(REMOVE_ICON, true, LF_TEX_FILTER_LINEAR);
    s.raiseicon = lf_load_texture(RAISE_ICON, true, LF_TEX_FILTER_LINEAR);
    InitializeData();
}

void InitializeData() {
    strcat(s.tododata_file, TODO_DATA_FILE);

    entries_da_init(&s.todo_entries);
    deserialize_todo_list(s.tododata_file, &s.todo_entries);
}

//Render functions
void rendertopbar() {
    lf_push_font(&s.titlefont);
    {
        LfUIElementProps props = lf_get_theme().text_props;
        lf_push_style_props(props);
        lf_text("Progress your tasks");
        lf_pop_style_props();
    }
    lf_pop_font();

    {
        const char* text = "Add Task";
        const float width = 150.0f;

        LfUIElementProps props = lf_get_theme().button_props;
        props.border_width = 0.0f;
        props.margin_top = 0.0f;
        props.color = BTN_COLOR;
        props.corner_radius = 4.0f;
        lf_push_style_props(props);
        lf_set_ptr_x_absolute(s.winw - width - GLOBAL_MARGIN * 2.0f);

        lf_set_line_should_overflow(false);
        if (lf_button_fixed("Add Task", width, -1) == LF_CLICKED) {
            s.crnt_tab = TAB_NEW_TASK;
        }
        lf_set_line_should_overflow(true);

        lf_pop_style_props();
    }
}

void renderfilters() {
    // Filters 
    uint32_t itemcount = 6;
    static const char* items[] = {
      "ALL", "IN PROGRESS", "COMPLETED", "LOW", "MEDIUM", "HIGH"
    };

    // UI Properties
    LfUIElementProps props = lf_get_theme().button_props;
    props.margin_left = 10.0f;
    props.margin_right = 10.0f;
    props.margin_top = 20.0f;
    props.padding = 10.0f;
    props.border_width = 0.0f;
    props.color = LF_NO_COLOR;
    props.corner_radius = 8.0f;
    props.text_color = TEXT_COLOR;

    lf_push_font(&s.smallfont);

    // Calculating width
    float width = 0.0f;
    {
        float ptrx_before = lf_get_ptr_x();
        lf_push_style_props(props);
        lf_set_cull_end_x(s.winw);
        lf_set_cull_end_y(s.winh);
        lf_set_no_render(true);
        for (uint32_t i = 0; i < itemcount; i++) {
            lf_button(items[i]);
        }
        lf_unset_cull_end_x();
        lf_unset_cull_end_y();
        lf_set_no_render(false);
        width = lf_get_ptr_x() - ptrx_before - props.margin_right - props.padding;
    }

    lf_set_ptr_x_absolute(s.winw - width - GLOBAL_MARGIN);

    // Rendering the filter items
    lf_set_line_should_overflow(false);
    for (uint32_t i = 0; i < itemcount; i++) {
        // If the filter is currently selected, render a 
        // box around it to indicate selection.
        if (s.crnt_filter == (uint32_t)i) {
            props.color = (LfColor){ 255, 255, 255, 50 };
        }
        else {
            props.color = LF_NO_COLOR;
        }
        // Rendering the button
        lf_push_style_props(props);
        if (lf_button(items[i]) == LF_CLICKED) {
            s.crnt_filter = i;
        }
        lf_pop_style_props();
    }
    // Popping props
    lf_set_line_should_overflow(true);
    lf_pop_style_props();
    lf_pop_font();
}

void renderentries() {
    lf_div_begin(((vec2s) { lf_get_ptr_x(), lf_get_ptr_y() }),
        ((vec2s) {
        (s.winw - lf_get_ptr_x()) - GLOBAL_MARGIN, (s.winh - lf_get_ptr_y()) - GLOBAL_MARGIN
    }),
        true);

    uint32_t renderedcount = 0;
    for (uint32_t i = 0; i < s.todo_entries.count; i++) {
        todo_entry* entry = s.todo_entries.entries[i];
        // Filtering the entries
        if (s.crnt_filter == FILTER_COMPLETED && !entry->completed) continue;
        if (s.crnt_filter == FILTER_IN_PROGRESS && entry->completed) continue;
        if (s.crnt_filter == FILTER_LOW && entry->priority != PRIORITY_LOW) continue;
        if (s.crnt_filter == FILTER_MEDIUM && entry->priority != PRIORITY_MEDIUM) continue;
        if (s.crnt_filter == FILTER_HIGH && entry->priority != PRIORITY_HIGH) continue;

        {
            float ptry_before = lf_get_ptr_y();
            float priority_size = 15.0f;
            lf_set_ptr_y_absolute(lf_get_ptr_y() + priority_size);
            lf_set_ptr_x_absolute(lf_get_ptr_x() + 5.0f);
            bool clicked_priority = lf_hovered((vec2s) { lf_get_ptr_x(), lf_get_ptr_y() }, (vec2s) { priority_size, priority_size }) &&
                lf_mouse_button_went_down(GLFW_MOUSE_BUTTON_LEFT);
            if (clicked_priority) {
                if (entry->priority + 1 >= PRIORITY_COUNT) {
                    entry->priority = 0;
                }
                else {
                    entry->priority++;
                }
                sort_entries_by_priority(&s.todo_entries);
            }
            switch (entry->priority) {
            case PRIORITY_LOW: {
                lf_rect(priority_size, priority_size, (LfColor) { 76, 175, 80, 255 }, 4.0f);
                break;
            }
            case PRIORITY_MEDIUM: {
                lf_rect(priority_size, priority_size, (LfColor) { 255, 235, 59, 255 }, 4.0f);
                break;
            }
            case PRIORITY_HIGH: {
                lf_rect(priority_size, priority_size, (LfColor) { 244, 67, 54, 255 }, 4.0f);
                break;
            }
            default:
                break;
            }
            lf_set_ptr_y_absolute(ptry_before);
        }
        {
            LfUIElementProps props = lf_get_theme().button_props;
            props.color = LF_NO_COLOR;
            props.border_width = 0.0f; props.padding = 0.0f; props.margin_top = 13; props.margin_left = 10.0f;
            lf_push_style_props(props);
            if (lf_image_button(((LfTexture) { .id = s.removeicon.id, .width = 20, .height = 20 })) == LF_CLICKED) {
                entries_da_remove_i(&s.todo_entries, i);
                serialize_todo_list(s.tododata_file, &s.todo_entries);
            }
            lf_pop_style_props();
        }
        {
            LfUIElementProps props = lf_get_theme().checkbox_props;
            props.border_width = 1.0f; props.corner_radius = 0; props.margin_top = 11; props.padding = 5.0f;
            props.color = BG_COLOR;
            lf_push_style_props(props);
            if (lf_checkbox("", &entry->completed, LF_NO_COLOR, BTN_COLOR) == LF_CLICKED) {
                serialize_todo_list(s.tododata_file, &s.todo_entries);
            }
            lf_pop_style_props();
        }

        float textptrx = lf_get_ptr_x();
        lf_text(entry->desc);

        lf_set_ptr_x_absolute(textptrx);
        lf_set_ptr_y_absolute(lf_get_ptr_y() + lf_get_theme().font.font_size);
        {
            LfUIElementProps props = lf_get_theme().text_props;
            props.margin_top = 2.5f;
            props.text_color = (LfColor){ 150, 150, 150, 255 };
            lf_push_style_props(props);
            lf_push_font(&s.smallfont);
            lf_text(entry->date);
            lf_pop_font();
            lf_pop_style_props();
        }

        {
            uint32_t texw = 15, texh = 8;
            lf_set_ptr_x_absolute(s.winw - GLOBAL_MARGIN - texw);
            lf_set_line_should_overflow(false);

            LfUIElementProps props = lf_get_theme().button_props;
            props.color = LF_NO_COLOR;
            props.border_width = 0.0f; props.padding = 0.0f; props.margin_left = 0.0f; props.margin_right = 0.0f;
            lf_push_style_props(props);
            lf_set_image_color((LfColor) { 120, 120, 120, 255 });
            if (lf_image_button(((LfTexture) { .id = s.raiseicon.id, .width = texw, .height = texh })) == LF_CLICKED) {
                todo_entry* tmp = s.todo_entries.entries[0];
                s.todo_entries.entries[0] = entry;
                s.todo_entries.entries[i] = tmp;
                serialize_todo_list(s.tododata_file, &s.todo_entries);
            }
            lf_unset_image_color();
            lf_set_line_should_overflow(true);
            lf_pop_style_props();
        }

        lf_next_line();
        renderedcount++;
    }
    if (!renderedcount) {
        lf_text("No task has been assigned!");
    }

    lf_div_end();
}

void renderdashboard() {
    rendertopbar();
    lf_next_line();
    renderfilters();
    lf_next_line();
    renderentries();
}

void rendernewtask() {
    // Title
    lf_push_font(&s.titlefont);
    {
        LfUIElementProps props = lf_get_theme().text_props;
        props.margin_bottom = 15.0f;
        lf_push_style_props(props);
        lf_text("Add a new Task");
        lf_pop_style_props();
        lf_pop_font();
    }

    lf_next_line();

    // Description input field 
    {
        lf_push_font(&s.smallfont);
        lf_text("Description");
        lf_pop_font();

        lf_next_line();
        LfUIElementProps props = lf_get_theme().inputfield_props;
        props.padding = 15;
        props.border_width = 0;
        props.color = BG_COLOR;
        props.corner_radius = 11;
        props.text_color = LF_WHITE;
        props.border_width = 1.0f;
        props.border_color = s.new_task_input.selected ? LF_WHITE : (LfColor) { 170, 170, 170, 255 };
        props.corner_radius = 2.5f;
        props.margin_bottom = 10.0f;
        lf_push_style_props(props);
        lf_input_text(&s.new_task_input);
        lf_pop_style_props();
    }

    lf_next_line();

    lf_next_line();

    // Priority dropdown
    static int32_t selected_priority = -1;
    {
        lf_push_font(&s.smallfont);
        lf_text("Priority");
        lf_pop_font();

        lf_next_line();
        static const char* items[3] = {
          "Low",
          "Medium",
          "High"
        };
        static bool opened = false;
        LfUIElementProps props = lf_get_theme().button_props;
        props.color = (LfColor){ 70, 70, 70, 255 };
        props.text_color = LF_WHITE;
        props.border_width = 0.0f;
        props.corner_radius = 5.0f;
        lf_push_style_props(props);
        lf_dropdown_menu(items, "Priority", 3, 200, 80, &selected_priority, &opened);
        lf_pop_style_props();
    }

    // Add Button
    {
        bool form_complete = (strlen(s.new_task_input_buf) && selected_priority != -1);
        const char* text = "Add";
        const float width = 150.0f;

        LfUIElementProps props = lf_get_theme().button_props;
        props.margin_left = 0.0f;
        props.margin_right = 0.0f;
        props.corner_radius = 5.0f;
        props.border_width = 0.0f;
        props.color = !form_complete ? (LfColor) { 80, 80, 80, 255 } : BTN_COLOR;
        lf_push_style_props(props);
        lf_set_line_should_overflow(false);
        lf_set_ptr_x_absolute(s.winw - (width + lf_get_theme().button_props.padding * 2.0f) - GLOBAL_MARGIN);
        lf_set_ptr_y_absolute(s.winh - (lf_button_dimension(text).y + lf_get_theme().button_props.padding * 2.0f) - GLOBAL_MARGIN);

        // If the user wants to add a new task: 
        if ((lf_button_fixed(text, width, -1) == LF_CLICKED && form_complete) ||
            (lf_key_went_down(GLFW_KEY_ENTER) && form_complete)) {

            // Copy the description input buffers content to a new pointer
            char* desc = malloc(strlen(s.new_task_input_buf));
            strcpy(desc, s.new_task_input_buf);

            // Allocate a new entry
            todo_entry* entry = malloc(sizeof(todo_entry));
            entry->desc = desc;
            entry->date = get_command_output(DATE_CMD);
            entry->completed = false;
            entry->priority = (entry_priority)selected_priority;
            entries_da_push(&s.todo_entries, entry);
            sort_entries_by_priority(&s.todo_entries);

            // Serialize entries 
            serialize_todo_list(s.tododata_file, &s.todo_entries);

            // Reset interface state
            memset(s.new_task_input_buf, 0, sizeof(s.new_task_input_buf));
            s.new_task_input.cursor_index = 0;
            lf_input_field_unselect_all(&s.new_task_input);
        }
        lf_set_line_should_overflow(true);
        lf_pop_style_props();
    }

    // Back Icon button
    lf_next_line();
    {
        LfUIElementProps props = lf_get_theme().button_props;
        props.color = LF_NO_COLOR;
        props.border_width = 0.0f;
        props.padding = 0.0f;
        props.margin_left = 0.0f;
        props.margin_right = 0.0f;
        props.margin_top = 0.0f;
        props.margin_bottom = 0.0f;
        lf_push_style_props(props);
        lf_set_line_should_overflow(false);
        LfTexture backbutton = (LfTexture){ .id = s.backicon.id, .width = 20, .height = 40 };
        lf_set_ptr_y_absolute(s.winh - backbutton.height - GLOBAL_MARGIN * 2.0f);
        lf_set_ptr_x_absolute(GLOBAL_MARGIN);

        if (lf_image_button(backbutton) == LF_CLICKED) {
            s.crnt_tab = TAB_DASHBOARD;
        }
        lf_set_line_should_overflow(true);
        lf_pop_style_props();
    }
}

//Entries
void entries_da_init(entries_da* da) {
    da->cap = DA_INIT_CAP;
    da->count = 0;
    da->entries = (todo_entry**)malloc(sizeof(todo_entry) * da->cap);
}

void entries_da_push(entries_da* da, todo_entry* entry) {
    if (da->count == da->cap) {
        entries_da_resize(da, da->cap * 2);
    }
    da->entries[da->count++] = entry;
}

void entries_da_resize(entries_da* da, int32_t new_cap) {
    todo_entry** temp = (todo_entry**)realloc(da->entries, new_cap * sizeof(todo_entry));
    if (!temp) {
        fprintf(stderr, "Failed to reallocate memory\n");
        exit(EXIT_FAILURE);
    }
    da->entries = temp;
    da->cap = new_cap;
}

void entries_da_remove_i(entries_da* da, uint32_t i) {
    // Bounds check 
    if (i < 0 || i >= da->count) {
        printf("Index out of bounds\n");
        return;
    }

    // Remove element
    for (uint32_t idx = i; idx < da->count - 1; idx++) {
        da->entries[idx] = da->entries[idx + 1];
    }

    // Decrease the count
    da->count--;
}

void entries_da_free(entries_da* da) {
    if (da->entries)
        free(da->entries);
    da->cap = 0;
    da->count = 0;
}

int compare_entry_priority(const void* a, const void* b) {
    todo_entry* entry_a = *(todo_entry**)a;
    todo_entry* entry_b = *(todo_entry**)b;
    return (entry_b->priority - entry_a->priority);
}

void sort_entries_by_priority(entries_da* da) {
    qsort(da->entries, da->count, sizeof(todo_entry*), compare_entry_priority);
}

//Serialization
void serialize_todo_entry(FILE* file, todo_entry* entry) {
    // Write completed to file
    fwrite(&entry->completed, sizeof(bool), 1, file);

    // Write description to file
    size_t desc_len = strlen(entry->desc) + 1; // +1 for null terminator
    // Writing the description length to the file for later 
    // deserialization
    fwrite(&desc_len, sizeof(size_t), 1, file);
    fwrite(entry->desc, sizeof(char), desc_len, file);

    // Write date to file
    size_t date_len = strlen(entry->date) + 1; // +1 for null terminator
    // Writing the date length to the file for later 
    // deserialization
    fwrite(&date_len, sizeof(size_t), 1, file);
    fwrite(entry->date, sizeof(char), date_len, file);

    // Write priority to file
    fwrite(&entry->priority, sizeof(entry_priority), 1, file);
}

void serialize_todo_list(const char* filename, entries_da* da) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open data file.\n");
        return;
    }
    for (uint32_t i = 0; i < da->count; i++) {
        serialize_todo_entry(file, da->entries[i]);
    }
    fclose(file);
}

todo_entry* deserialize_todo_entry(FILE* file) {
    // Allocate entry
    todo_entry* entry = malloc(sizeof(todo_entry));

    // Read if entry is completed
    if (fread(&entry->completed, sizeof(bool), 1, file) != 1) {
        free(entry);
        return NULL;
    }

    // Read the length of the description
    size_t desc_len;
    if (fread(&desc_len, sizeof(size_t), 1, file) != 1) {
        free(entry);
        return NULL;
    }

    // Allocating space to store the entries description
    entry->desc = malloc(desc_len);
    if (!entry->desc) {
        free(entry);
        return NULL;
    }
    // Read the description from the file
    if (fread(entry->desc, sizeof(char), desc_len, file) != desc_len) {
        free(entry->desc);
        free(entry);
        return NULL;
    }

    // Read the date length from the file
    size_t date_len;
    if (fread(&date_len, sizeof(size_t), 1, file) != 1) {
        free(entry->desc);
        free(entry);
        return NULL;
    }
    // Allocating space for the date
    entry->date = malloc(date_len);
    if (!entry->date) {
        free(entry->desc);
        free(entry);
        return NULL;
    }
    // Reading the date string
    if (fread(entry->date, sizeof(char), date_len, file) != date_len) {
        free(entry->desc);
        free(entry->date);
        free(entry);
        return NULL;
    }

    // Reading the entires priority
    if (fread(&entry->priority, sizeof(entry_priority), 1, file) != 1) {
        free(entry->desc);
        free(entry->date);
        free(entry);
        return NULL;
    }

    return entry;
}

void deserialize_todo_list(const char* filename, entries_da* da) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        // If file does not exist, create it 
        file = fopen(filename, "w");
        fclose(file);
    }
    file = fopen(filename, "rb");
    todo_entry* entry;
    while ((entry = deserialize_todo_entry(file)) != NULL) {
        entries_da_push(da, entry);
    }
    fclose(file);
}

char* get_command_output(const char* cmd) {
    FILE* fp;
    char buffer[1024];
    char* result = NULL;
    size_t result_size = 0;

    // Opening a new pipe with the fiven command
    fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        return NULL;
    }

    // Reading the output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t buffer_len = strlen(buffer);
        char* temp = realloc(result, result_size + buffer_len + 1);
        if (temp == NULL) {
            printf("Memory allocation failed\n");
            free(result);
            pclose(fp);
            return NULL;
        }
        result = temp;
        strcpy(result + result_size, buffer);
        result_size += buffer_len;
    }
    pclose(fp);
    return result;
}

void print_requires_argument(const char* option, uint32_t numargs) {
    printf("todo: option requires %i argument(s): '%s'\n", numargs, option);
    printf("Try todo --help for more information\n");
}

void Destroy() {
    lf_terminate();

    lf_free_font(&s.smallfont);
    lf_free_font(&s.titlefont);
    entries_da_free(&s.todo_entries);

    glfwDestroyWindow(s.win);
    glfwTerminate();
}

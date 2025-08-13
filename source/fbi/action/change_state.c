#include <3ds.h>
#include <string.h>
#include <stdlib.h>

#include "action.h"
#include "../task/uitask.h"
#include "../../core/ui/error.h"
#include "../../core/ui/prompt.h"
#include "../../core/screen.h"
#include "../../core/ui/list_old.h"
#include "../../core/ui/ui.h"
#include "../locale.h"

// TODO duplicate code? (see section/config.c)
typedef struct {
    list_item* selected;
    u32 count;
    Handle cancelEvent;
    bool populated;
    list_item_old items[128];
} state_data;

static void action_set_state(state_data* data, char* name, bool populated) {
    title_info* info = (title_info*) data->selected->data;
    char* country = country_for_title(info->titleId);
    const char* code = getStateCodeByName(country, name);
//    error_display(NULL, NULL, code);
    if (R_SUCCEEDED(set_state_for_title((info->titleId), code))) {
        // Refresh locale info

        info->locale = locale_for_title(info->titleId);

        char* template = "应用的详细地区已被设定至\n%s";
        char* message = calloc(strlen(template) + strlen(name), sizeof(char));
        snprintf(message, strlen(template) + strlen(name), template, name);
        prompt_display_notify("设定详细地区", message, COLOR_TEXT, NULL, NULL, NULL);
    }
    else {
        // error_display_res(data, ui_draw_title_info, false, "Failed to set state (does locales directory exist?)");
        error_display(data, NULL, "设定详细地区失败\n（目标文件夹是否存在？）");
    }
}

static void state_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item_old* selected) {
    task_draw_title_info(view, ((state_data*) data)->selected->data, x1, y1, x2, y2);
}

static void state_update(ui_view* view, void* data, list_item_old** items, u32** itemCount, list_item_old* selected, bool selectedTouched) {
    state_data* listData = (state_data*) data;

    if(hidKeysDown() & KEY_B) {
        ui_pop();
        list_destroy_old(view);
        free(listData);
        return;
    }

//    if(!listData->selected) { // This probably should never trigger
//        for (int i = 0; i < RGN_MAX; i++) {
//            list_item_old item;
//            strncpy(item.name, state_to_string(i), NAME_MAX);
//            item.color = COLOR_TEXT;
//            item.data = action_set_state;
//            listData->items[i] = item;
//        }
//        listData->title_info = NULL;
//        listData->populated = true;
//        listData->count = RGN_MAX;
//    }

    if(selected != NULL && selected->data != NULL && (selectedTouched || (hidKeysDown() & KEY_A))) {
        void(*action)(state_data*, char* name, bool*)
        = (void(*)(state_data*, char* name, bool*)) selected->data;

        ui_pop();
        list_destroy_old(view);

        action(listData, selected->name, &listData->populated);

        free(data);

        return;
    }

    if(*itemCount != &listData->count || *items != listData->items) {
        *itemCount = &listData->count;
        *items = listData->items;
    }
}

void change_state(list_item* selected) {
    title_info* info = selected->data;
    char* country = country_for_title(info->titleId);
    if (strlen(country) == 0){
        error_display(NULL, NULL, "请先设定国家/地区。");
        return;
    }
    char* filtered_states[128] = {};
    int state_count = listStates(country, filtered_states);
    if (state_count == 0){
        error_display(NULL, NULL, "选定的国家/地区没有可用的详细地区设置信息。");
        return;
    }

    state_data* data = (state_data*) calloc(1, sizeof(state_data));
    for (int i = 0; i < state_count; i++) {
        list_item_old item;
        strncpy(item.name, filtered_states[i], NAME_MAX);
        item.color = COLOR_TEXT;
        item.data = action_set_state;
        data->items[i] = item;
    }
    data->selected = selected;
    data->populated = true;
    data->count = state_count;
    list_display_old("设定详细地区", "A：选择，B：返回", data, state_update, state_draw_top);
}

static void state_system_title_warning_onresponse(ui_view* view, void* data, u32 response) {
    FS_ArchiveID archive = (FS_ArchiveID) data;

    if(response == PROMPT_YES) {
        change_state((list_item*)data);
    }
}

void action_change_state(linked_list* items, list_item* selected) {
    title_info* info = (title_info*) selected ->data;
    if (info -> mediaType == MEDIATYPE_NAND){
        prompt_display_yes_no("确认", "修改系统应用是危险的。\n这可能导致系统无法正常运行，\n请您确保自己知道正在做什么。\n\n确认继续?", COLOR_TEXT, (void*) selected, NULL, state_system_title_warning_onresponse);
    } else {
        change_state(selected);
    }
}
#include <3ds.h>
#include <string.h>
#include <stdlib.h>

#include "action.h"
#include "../task/uitask.h"
#include "../../core/ui/error.h"
#include "../../core/ui/prompt.h"
#include "../../core/ui/ui.h"
#include "../../core/screen.h"
#include "../locale.h"
#include "../../core/ui/list_old.h"

// TODO duplicate code? (see section/config.c)
typedef struct {
    list_item_old items[LNG_MAX];
    list_item* selected;
    u32 count;
    Handle cancelEvent;
    bool populated;
} language_data;

static void action_set_language(language_data* data, char* name, bool populated) {
    title_info* info = (title_info*) data->selected->data;
    if (R_SUCCEEDED(set_language_for_title(info->titleId, language_from_string_display(name)))) {
        // Refresh locale info
        info->locale = locale_for_title(info->titleId);

        char* template = "应用的语言已被设定至\n%s";
        char* message = calloc(strlen(template) + strlen(name), sizeof(char));
        snprintf(message, strlen(template) + strlen(name), template, name);
        prompt_display_notify("设定语言", message, COLOR_TEXT, NULL, NULL, NULL);
    }
    else {
        error_display(data, NULL, "设定语言失败\n（目标文件夹是否存在？）");
    }
}

static void language_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item_old* selected) {
    task_draw_title_info(view, ((language_data*) data)->selected->data, x1, y1, x2, y2);
}

static void language_update(ui_view* view, void* data, list_item_old** items, u32** itemCount, list_item_old* selected, bool selectedTouched) {
    language_data* listData = (language_data*) data;

    if(hidKeysDown() & KEY_B) {
        ui_pop();
        list_destroy_old(view);
        free(listData);
        return;
    }

//    if(!listData->populated) { // This probably should never trigger
//        for (int i = 0; i < LNG_MAX; i++) {
//            list_item item;
//            strncpy(item.name, language_to_string(i), NAME_MAX - 1);
//            item.color = COLOR_TEXT;
//            item.data = action_set_language;
//            listData->items[i] = item;
//        }
//        listData->title_info = NULL;
//        listData->populated = true;
//        listData->count = LNG_MAX;
//    }

    if(selected != NULL && selected->data != NULL && (selectedTouched || (hidKeysDown() & KEY_A))) {
        void(*action)(language_data*, char* name, bool*)
            = (void(*)(language_data*, char* name, bool*)) selected->data;

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

void action_change_language(linked_list* items, list_item* selected) {
    title_info* info = (title_info*)selected ->data;
    if (info->mediaType == MEDIATYPE_NAND){
        error_display(NULL, NULL, "无法为系统应用设定语言。");
        return;
    }
    language_data* data = (language_data*) calloc(1, sizeof(language_data));
    for (int i = 0; i < LNG_MAX; i++) {
        list_item_old item;
        strncpy(item.name, language_to_string_display(i), NAME_MAX);
        item.color = COLOR_TEXT;
        item.data = action_set_language;
        data->items[i] = item;
    }
    data->selected = selected;
    data->populated = true;
    data->count = LNG_MAX;
    list_display_old("设定语言", "A：选择，B：返回", data, language_update, language_draw_top);
}

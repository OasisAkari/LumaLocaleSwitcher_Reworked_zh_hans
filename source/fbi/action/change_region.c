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
    list_item_old items[RGN_MAX];
    list_item* selected;
    u32 count;
    Handle cancelEvent;
    bool populated;
} region_data;

static void action_set_region(region_data* data, char* name, bool populated) {
    title_info* info = (title_info*) data->selected->data;
    if (R_SUCCEEDED(set_region_for_title((info->titleId), region_from_string_display(name)))) {
        // Refresh locale info

        info->locale = locale_for_title(info->titleId);

        char* template = "应用的区域已被设定至\n%s";
        char* message = calloc(strlen(template) + strlen(name), sizeof(char));
        snprintf(message, strlen(template) + strlen(name), template, name);
        prompt_display_notify("设定区域", message, COLOR_TEXT, NULL, NULL, NULL);
    }
    else {
        // error_display_res(data, ui_draw_title_info, false, "Failed to set region (does locales directory exist?)");
        error_display(data, NULL, "设定区域失败\n（目标文件夹是否存在？）");
    }
}

static void region_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item_old* selected) {
    task_draw_title_info(view, ((region_data*) data)->selected->data, x1, y1, x2, y2);
}

static void region_update(ui_view* view, void* data, list_item_old** items, u32** itemCount, list_item_old* selected, bool selectedTouched) {
    region_data* listData = (region_data*) data;

    if(hidKeysDown() & KEY_B) {
        ui_pop();
        list_destroy_old(view);
        free(listData);
        return;
    }

//    if(!listData->selected) { // This probably should never trigger
//        for (int i = 0; i < RGN_MAX; i++) {
//            list_item_old item;
//            strncpy(item.name, region_to_string(i), NAME_MAX);
//            item.color = COLOR_TEXT;
//            item.data = action_set_region;
//            listData->items[i] = item;
//        }
//        listData->title_info = NULL;
//        listData->populated = true;
//        listData->count = RGN_MAX;
//    }

    if(selected != NULL && selected->data != NULL && (selectedTouched || (hidKeysDown() & KEY_A))) {
        void(*action)(region_data*, char* name, bool*)
        = (void(*)(region_data*, char* name, bool*)) selected->data;

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

void action_change_region(linked_list* items, list_item* selected) {
    title_info* info = (title_info*)selected ->data;
    if (info->mediaType == MEDIATYPE_NAND){
        error_display(NULL, NULL, "无法为系统应用设定区域。");
        return;
    }
    region_data* data = (region_data*) calloc(1, sizeof(region_data));
    for (int i = 0; i < RGN_MAX; i++) {
        list_item_old item;
        strncpy(item.name, region_to_string_display(i), NAME_MAX);
        item.color = COLOR_TEXT;
        item.data = action_set_region;
        data->items[i] = item;
    }
    data->selected = selected;
    data->populated = true;
    data->count = RGN_MAX;
    list_display_old("设定区域", "A：选择，B：返回", data, region_update, region_draw_top);
}

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
    list_item_old items[COUNTRY_COUNT];
} country_data;

typedef struct {
    list_item_old items[RGN_MAX];
    list_item* selected;
    u32 count;
    Handle cancelEvent;
    bool populated;
} region_data;

static void action_set_country(country_data* data, char* name, bool populated) {
    title_info* info = (title_info*) data->selected->data;
    const char* full_name = getCountryCodeByFullName(name);
    if (R_SUCCEEDED(set_country_for_title((info->titleId), full_name))) {
        // Refresh locale info

        info->locale = locale_for_title(info->titleId);

        char* template = "应用的国家/地区已被设定至\n%s";
        char* message = calloc(strlen(template) + strlen(name), sizeof(char));
        snprintf(message, strlen(template) + strlen(name), template, name);
        prompt_display_notify("设定国家/地区", message, COLOR_TEXT, NULL, NULL, NULL);
    }
    else {
        // error_display_res(data, ui_draw_title_info, false, "Failed to set country (does locales directory exist?)");
        error_display(data, NULL, "设定国家/地区失败\n（目标文件夹是否存在？）");
    }
}

static void country_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item_old* selected) {
    task_draw_title_info(view, ((country_data*) data)->selected->data, x1, y1, x2, y2);
}

static void country_update(ui_view* view, void* data, list_item_old** items, u32** itemCount, list_item_old* selected, bool selectedTouched) {
    country_data* listData = (country_data*) data;

    if(hidKeysDown() & KEY_B) {
        ui_pop();
        list_destroy_old(view);
        free(listData);
        return;
    }

//    if(!listData->selected) { // This probably should never trigger
//        for (int i = 0; i < RGN_MAX; i++) {
//            list_item_old item;
//            strncpy(item.name, country_to_string(i), NAME_MAX);
//            item.color = COLOR_TEXT;
//            item.data = action_set_country;
//            listData->items[i] = item;
//        }
//        listData->title_info = NULL;
//        listData->populated = true;
//        listData->count = RGN_MAX;
//    }

    if(selected != NULL && selected->data != NULL && (selectedTouched || (hidKeysDown() & KEY_A))) {
        void(*action)(country_data*, char* name, bool*)
        = (void(*)(country_data*, char* name, bool*)) selected->data;

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


static void action_set_region(region_data* data, char* name, bool populated) {
    title_info* info = (title_info*) data->selected->data;
    char* region_str = region_map(name);
    CountryInfo filtered_regions[COUNTRY_COUNT] = {};
    int country_count = filterCountriesByRegion(region_str, filtered_regions);
//    char* msg = (char*) calloc(PATH_MAX+18, sizeof(char));
//    snprintf(msg, 2048, "%i", country_count);
//    error_display(NULL, NULL, msg);

    country_data* c_data = (country_data*) calloc(1, sizeof(country_data));
    for (int i = 0; i < country_count; i++) {
        list_item_old item;
        strncpy(item.name, filtered_regions[i].full_name, NAME_MAX);
        item.color = COLOR_TEXT;
        item.data = action_set_country;
        c_data->items[i] = item;
    }
    c_data->selected = data->selected;
    c_data->populated = true;
    c_data->count = country_count;
    list_display_old("设定国家/地区", "A：选择，B：返回", c_data, country_update, country_draw_top);

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



void change_country(list_item* selected){
    region_data* data = (region_data*) calloc(1, sizeof(region_data));
    for (int i = 0; i < RGN_MAX; i++) {
        list_item_old item;
        strncpy(item.name, region_to_string(i), NAME_MAX);
        item.color = COLOR_TEXT;
        item.data = action_set_region;
        data->items[i] = item;
    }
    data->selected = selected;
    data->populated = true;
    data->count = RGN_MAX;
    list_display_old("选择区域", "A：选择，B：返回", data, region_update, region_draw_top);
}

static void country_system_title_warning_onresponse(ui_view* view, void* data, u32 response) {
    FS_ArchiveID archive = (FS_ArchiveID) data;

    if(response == PROMPT_YES) {
        change_country((list_item*)data);
    }
}

void action_change_country(linked_list* items, list_item* selected) {
    title_info* info = (title_info*) selected ->data;
    if (info -> mediaType == MEDIATYPE_NAND){
        prompt_display_yes_no("确认", "修改系统应用是危险的。\n这可能导致系统无法正常运行，\n请您确保自己知道正在做什么。\n\n确认继续?", COLOR_TEXT, (void*) selected, NULL, country_system_title_warning_onresponse);
    } else {
        change_country(selected);
    }
}
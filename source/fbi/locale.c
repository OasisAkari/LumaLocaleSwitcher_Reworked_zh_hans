#include <3ds.h>
#include <sys/syslimits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "locale.h"
#include "../core/util.h"

Region region_from_string(char* string) {
    return
            strcmp(string, "JPN") == 0 ? JPN :
            strcmp(string, "USA") == 0 ? USA :
            strcmp(string, "EUR") == 0 ? EUR :
            strcmp(string, "AUS") == 0 ? AUS :
            strcmp(string, "CHN") == 0 ? CHN :
            strcmp(string, "KOR") == 0 ? KOR :
            strcmp(string, "TWN") == 0 ? TWN :
            RGN_NONE;
}

Region region_from_string_display(char* string) {
    return
            strcmp(string, "日版（JPN）") == 0 ? JPN :
            strcmp(string, "美版（USA）") == 0 ? USA :
            strcmp(string, "欧版（EUR）") == 0 ? EUR :
            strcmp(string, "澳版（AUS）") == 0 ? AUS :
            strcmp(string, "神游版（CHN）") == 0 ? CHN :
            strcmp(string, "韩国（KOR）") == 0 ? KOR :
            strcmp(string, "港台版（TWN）") == 0 ? TWN :
            RGN_NONE;
}

Language language_from_string(char* string) {
    return
            strcmp(string, "JP") == 0 ? JP :
            strcmp(string, "EN") == 0 ? EN :
            strcmp(string, "FR") == 0 ? FR :
            strcmp(string, "DE") == 0 ? DE :
            strcmp(string, "IT") == 0 ? IT :
            strcmp(string, "ES") == 0 ? ES :
            strcmp(string, "ZH") == 0 ? ZH :
            strcmp(string, "KO") == 0 ? KO :
            strcmp(string, "NL") == 0 ? NL :
            strcmp(string, "PT") == 0 ? PT :
            strcmp(string, "RU") == 0 ? RU :
            strcmp(string, "TW") == 0 ? TW :
            LNG_NONE;
}

Language language_from_string_display(char* string) {
    return
            strcmp(string, "日语（JP）") == 0 ? JP :
            strcmp(string, "英语（EN）") == 0 ? EN :
            strcmp(string, "法语（FR）") == 0 ? FR :
            strcmp(string, "德语（DE）") == 0 ? DE :
            strcmp(string, "意大利语（IT）") == 0 ? IT :
            strcmp(string, "西班牙语（ES）") == 0 ? ES :
            strcmp(string, "简体中文（ZH）") == 0 ? ZH :
            strcmp(string, "韩语（KO）") == 0 ? KO :
            strcmp(string, "荷兰语（NL）") == 0 ? NL :
            strcmp(string, "葡萄牙语（PT）") == 0 ? PT :
            strcmp(string, "俄语（RU）") == 0 ? RU :
            strcmp(string, "繁体中文（TW）") == 0 ? TW :
            LNG_NONE;
}


static const char* _Region_Strings[] = { "JPN", "USA", "EUR", "AUS", "CHN", "KOR", "TWN" };

static const char* _Region_Strings_Display[] = { "日版（JPN）", "美版（USA）", "欧版（EUR）", "澳版（AUS）", "神游版（CHN）", "韩国（KOR）", "港台版（TWN）" };


const char* region_to_string(Region region) {
    if (region == RGN_NONE || region >= RGN_MAX)
        return "System Default";
    return _Region_Strings[region];
}

const char* region_to_string_display(Region region) {
    if (region == RGN_NONE || region >= RGN_MAX)
        return "系统默认";
    return _Region_Strings_Display[region];
}

static const char* _Language_Strings[] = { "JP", "EN", "FR", "DE", "IT", "ES", "ZH", "KO", "NL", "PT", "RU", "TW"};

static const char* _Language_Strings_Display[] = { "日语（JP）", "英语（EN）", "法语（FR）", "德语（DE）", "意大利语（IT）", "西班牙语（ES）", "简体中文（ZH）", "韩语（KO）", "荷兰语（NL）", "葡萄牙语（PT）", "俄语（RU）", "繁体中文（TW）"};

const char* language_to_string(Language language) {
    if (language == LNG_NONE || language >= LNG_MAX)
        return "系统默认";
    return _Language_Strings[language];
}

const char* language_to_string_display(Language language) {
    if (language == LNG_NONE || language >= LNG_MAX)
        return "系统默认";
    return _Language_Strings_Display[language];
}

char* locale_path_for_title(u64 titleId) {
    char* path = calloc(PATH_MAX, sizeof(char));
    char* cfg_path = calloc(PATH_MAX, sizeof(char));
    util_get_locale_path(cfg_path, PATH_MAX);

    char title_id_str[17];
    snprintf(title_id_str, 17, "%016llX", titleId);
    title_id_str[16] = '\0';

    snprintf(path, PATH_MAX, cfg_path, title_id_str);
    free(cfg_path);

    return path;
}

Locale* locale_for_title(u64 titleId) {

    FS_Archive sdmc_archive;

    Locale *locale_info = (Locale*) calloc(1, sizeof(Locale));

    // Defaults
    locale_info->region = RGN_NONE;
    locale_info->language = LNG_NONE;
    locale_info->country = "";
    locale_info->state = "";

    Result res;
    if (R_FAILED(res = FSUSER_OpenArchive(&sdmc_archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY,"")))) {
        return locale_info;
    }

    Handle handle;
    char* path = locale_path_for_title(titleId);
    FS_Path* fs_path = util_make_path_utf8(path);
    if(R_FAILED(res = FSUSER_OpenFileDirectly(&handle, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY,""), *fs_path, FS_OPEN_READ, 0))) {
        free(fs_path);
        free(path);
        return locale_info;
    }

    char* buffer = (char*) calloc(13, sizeof(char)); // ex., "JPN JP\0"
    u32 bytes_read;
    FSFILE_Read(handle, &bytes_read, 0, buffer, 12);
    FSFILE_Close(handle);

    util_free_path_utf8(fs_path);
    free(path);

    FSUSER_CloseArchive(sdmc_archive);

    if (bytes_read < 6) { // we need at least "JPN JP"
        locale_info->region = RGN_NONE;
        locale_info->language = LNG_NONE;
        locale_info->country = "";
        locale_info->state = "";
        return locale_info;
    }

    buffer[bytes_read] = '\0';

    char* region_str = (char*) calloc(4, sizeof(char));
    char* lang_str = (char*) calloc(3, sizeof(char));
    char* country_str = (char*) calloc(3, sizeof(char));
    char* state_str = (char*) calloc(3, sizeof(char));
    if (sscanf(buffer, "%3s %2s %2s %2s", region_str, lang_str, country_str, state_str) == 4){
        locale_info->region = region_from_string(region_str);
        locale_info->language = language_from_string(lang_str);
        locale_info->country = country_str;
        locale_info->state = state_str;

    } else if (sscanf(buffer, "%3s %2s %2s", region_str, lang_str, country_str) == 3){
        locale_info->region = region_from_string(region_str);
        locale_info->language = language_from_string(lang_str);
        locale_info->country = country_str;
    } else if (sscanf(buffer, "%3s %2s", region_str, lang_str) == 2) {
        locale_info->region = region_from_string(region_str);
        locale_info->language = language_from_string(lang_str);
    }
    else {
        locale_info->region = RGN_NONE;
        locale_info->language = LNG_NONE;
    }

    return locale_info;
}

Region region_for_title(u64 titleId) {
    Locale* locale = locale_for_title(titleId);
    return locale->region;
}

Language language_for_title(u64 titleId) {
    Locale* locale = locale_for_title(titleId);
    return locale->language;
}

char* country_for_title(u64 titleId){
    Locale* locale = locale_for_title(titleId);
    return locale->country;
}

char* state_for_title(u64 titleId){
    Locale* locale = locale_for_title(titleId);
    return locale->state;
}

Result _set_locale_for_title(u64 titleId, Locale* locale) {
    char* locale_dir = (char*) calloc(PATH_MAX, sizeof(char));
    util_get_locale_dir(locale_dir, PATH_MAX);
    FS_Archive sdmc_archive;

    Result res;
    if (R_FAILED(res = FSUSER_OpenArchive(&sdmc_archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY,"")))) return res;

    // Create the locale directory if it doesn't exist
    // XXX This probably doesn't work if more than one path in the hierarchy DNE
    util_ensure_dir(&sdmc_archive, locale_dir);
    free(locale_dir);

    char* locale_path = locale_path_for_title(titleId);
    FS_Path* fs_path = util_make_path_utf8(locale_path);

    // Make sure all directories exist
    int pathlen = strlen(locale_path);
    int last_virgule = pathlen;

    // Find the deepest directory
    for (int i = 0; i < pathlen; i++)
        if (locale_path[i] == '/')
            last_virgule = i;

    char* deepest_path = (char*) malloc(sizeof(char) * (last_virgule + 1));
    strncpy(deepest_path, locale_path, last_virgule);
    deepest_path[last_virgule] = '\0';
    util_ensure_dir(&sdmc_archive, deepest_path);

    Handle handle;
    FSUSER_DeleteFile(sdmc_archive, *fs_path);
    // If this fails, probably means locale directory does not exist
    // TODO create locale directory if not exist
    if(R_SUCCEEDED(
        FSUSER_OpenFileDirectly(&handle, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY,""), *fs_path, FS_OPEN_WRITE | FS_OPEN_CREATE, 0)
    )) {
        char* country = locale->country;
        char* state = locale->state;
        char* region_s = region_to_string(locale->region);
        char* language_s = language_to_string(locale->language);
        int s_len = strlen(region_s) + strlen(language_s) + strlen(country) + strlen(state);
        u32 bytes_written;
        if (s_len == 5){
            char* buffer = (char*) calloc(8, sizeof(char)); // ex: "JPN JP\0"
            snprintf(buffer, 64, "%s %s\n",
                     region_to_string(locale->region),
                     language_to_string(locale->language));
            buffer[7] = '\0';
            FSFILE_Write(handle, &bytes_written, 0, buffer, 6, FS_WRITE_FLUSH);
        }
        else if (s_len == 7){
            char* buffer = (char*) calloc(11, sizeof(char)); // ex: "JPN JP JP\0"
            snprintf(buffer, 64, "%s %s %s\n",
                     region_s,
                     language_s,
                     country
            );
            buffer[10] = '\0';
            FSFILE_Write(handle, &bytes_written, 0, buffer, 9, FS_WRITE_FLUSH);
        } else if  (s_len == 9){
            char* buffer = (char*) calloc(14, sizeof(char)); // ex: "JPN JP JP 00\0"
            snprintf(buffer, 64, "%s %s %s %s\n",
                     region_s,
                     language_s,
                     country,
                     state
            );
            buffer[13] = '\0';
            FSFILE_Write(handle, &bytes_written, 0, buffer, 12, FS_WRITE_FLUSH);

        }

        FSFILE_Close(handle);
        util_free_path_utf8(fs_path);
        FSUSER_CloseArchive(sdmc_archive);

        return bytes_written >= 6 ? true : -1;
    }
    return -1;
}

Result set_region_language_country_state_for_title(u64 titleId, Region region, Language language, const char* country, char* state) {
    Locale* locale = locale_for_title(titleId);

    Result result;

    // Default to system region/language
    if (region == RGN_NONE) {
        result = CFGU_SecureInfoGetRegion((u8*)&region);
        if (region <= RGN_NONE || region >= RGN_MAX)
            return result;
    }
    if (language == LNG_NONE) {
        result = CFGU_GetSystemLanguage((u8*)&language);
        if (language <= LNG_NONE || language >= LNG_MAX)
            return result;
    }

    locale->region = region;
    locale->language = language;
    locale->country = country;
    locale->state = state;
    return _set_locale_for_title(titleId, locale);
}

Result set_region_for_title(u64 titleId, Region region) {
    Locale* locale = locale_for_title(titleId);

    return set_region_language_country_state_for_title(titleId, region, locale->language, locale->country, locale->state);
}

Result set_language_for_title(u64 titleId, Language language) {
    Locale* locale = locale_for_title(titleId);

    return set_region_language_country_state_for_title(titleId, locale->region, language, locale->country, locale->state);
}

Result set_country_for_title(u64 titleId, const char* country) {
    Locale* locale = locale_for_title(titleId);
    locale->state = "";

    return set_region_language_country_state_for_title(titleId, locale->region, locale->language, country, locale->state);
}

Result set_state_for_title(u64 titleId, char* state) {
    Locale* locale = locale_for_title(titleId);

    return set_region_language_country_state_for_title(titleId, locale->region, locale->language, locale->country, state);
}
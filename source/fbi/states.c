#include <stdio.h>
#include <string.h>
#include <3ds/services/fs.h>
#include "states.h"

#include "../core/util.h"


StatesInfo filterStatesByCountry(char* country) {
    // 遍历所有国家信息
    for (int i = 0; i < STATES_COUNT; i++) {
        // 比较地区是否匹配
        if (strcmp(all_states[i].code, country) == 0) {
            return all_states[i];
        }
    }
    return all_states[STATES_COUNT];
}

int listStates(char* country, char* result[MAX_VALUE_LENGTH]){
    StatesInfo state_filtered = filterStatesByCountry(country);
    if (state_filtered.state_count != 0){
        for (int i = 0; i < state_filtered.state_count; i++){
            result[i] = state_filtered.states[i] -> name;
        }
        return state_filtered.state_count;
    }
    return 0;
}

const char* getStateCodeByName(char* country, char* name){
    StatesInfo state_filtered = filterStatesByCountry(country);
    for (int i = 0; i < state_filtered.state_count; i++){
        if (strcmp(name, state_filtered.states[i] -> name) == 0){
            return state_filtered.states[i] -> code;
        }
    }
    return "";
}
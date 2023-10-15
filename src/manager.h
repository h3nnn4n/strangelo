#ifndef _MANAGER_H
#define _MANAGER_H

typedef struct {
    /////////////////
    // Timer Stuff
    //
    float delta_time;
    float current_time;
    float current_frame_time;
    float last_frame_time;
} Manager;

extern Manager *manager;

Manager *init_manager();

void Manager_tick_timer(Manager *manager);

#endif

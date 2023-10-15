#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include "manager.h"

Manager *manager;

Manager *init_manager() {
    Manager *manager = malloc(sizeof(Manager));

    return manager;
}

void Manager_tick_timer(Manager *manager) {
    manager->current_time = glfwGetTime();

    manager->last_frame_time    = manager->current_frame_time;
    manager->current_frame_time = manager->current_time;
    manager->delta_time         = manager->current_frame_time - manager->last_frame_time;
}

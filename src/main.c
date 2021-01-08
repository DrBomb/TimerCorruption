#include "mgos.h"

//Long Timer CB that gets called on creation

static void long_cb(void *ud){
    LOG(LL_ERROR,("Long Timer"));
}

//Short Timer CB that will print "Timer"
static void short_cb(void *ud){
    LOG(LL_ERROR,("Timer"));
}

//EVENT_REBOOT handler
static void resethandler(int ev, void *ev_data, void *userdata){
    if(mgos_sys_config_get_App_save_time()){
        mgos_sys_config_set_App_saved_time(mg_time());
        save_cfg(&mgos_sys_config, NULL);
        LOG(LL_ERROR, ("Saved wall time %d", (int)mg_time()));
    }
}

enum mgos_app_init_result mgos_app_init(void) {
    //Set long timer
    if(mgos_sys_config_get_App_long_timer()) mgos_set_timer(60000, MGOS_TIMER_REPEAT | MGOS_TIMER_RUN_NOW, long_cb, NULL);
    
    //Check if "load_time" is enabled
    if(mgos_sys_config_get_App_save_time()){
        //Check if "saved_time" is not -1
        if(mgos_sys_config_get_App_saved_time() != -1){
            //Set wall time;
            mgos_settimeofday(mgos_sys_config_get_App_saved_time(), NULL);
            LOG(LL_ERROR, ("Wall time set to %d from config", mgos_sys_config_get_App_saved_time()));
            //Set "saved_time" to -1 and save config
            mgos_sys_config_set_App_saved_time(-1);
            save_cfg(&mgos_sys_config, NULL);
        }
    }
    
    //Set short timer after setting the wall time from config
    mgos_set_timer(5000, MGOS_TIMER_REPEAT, short_cb, NULL);
    
    //Set an event handler to listen for the reboot event
    mgos_event_add_handler(MGOS_EVENT_REBOOT, resethandler, NULL);

    return MGOS_APP_INIT_SUCCESS;
}
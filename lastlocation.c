#include "lastlocation.h"
#include <privacy_privilege_manager.h>
#include <locations.h>
#include <storage.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <openssl/aes.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

typedef struct appdata {
	location_manager_h location;
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *nf;
	Evas_Object *button;
} appdata_s;

Evas_Object *label1;
Evas_Object *label2;
Evas_Object *label3;
Evas_Object *label4;
Evas_Object *label5;

double latitude;
double longitude;

#define BUFLEN 300 /* Buffer size, used in functions */

sqlite3 *db; /* Database handle */
static int internal_storage_id;

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}
static int callback(void *counter, int argc, char **argv, char **azColName)
{
   int *localcounter = (int *)counter;
   int i;
   dlog_print(DLOG_DEBUG, LOG_TAG, "PES: %s, %s", argv[0], argv[1]);
   char buffer [512];
   snprintf(buffer,512,"<align=center>%s,%s<align=center>",argv[0], argv[1]);

   if(strcmp(elm_object_text_get(label1), "1")==0){
	   elm_object_text_set(label1, buffer);
   }
   else if(strcmp(elm_object_text_get(label2), "2")==0){
	   elm_object_text_set(label2, buffer);
   }
   else if(strcmp(elm_object_text_get(label3), "3")==0){
	   elm_object_text_set(label3, buffer);
   }
   else if(strcmp(elm_object_text_get(label4), "4")==0){
	   elm_object_text_set(label4, buffer);
   }

   (*localcounter)++;
   return 0;
}
void clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "PES: Button clicked\n");

    ui_app_exit();
}
static int InsertRecord(double lat, double lon)
{
   char sqlbuff[BUFLEN];
   char *ErrMsg;
   snprintf(sqlbuff, BUFLEN, "INSERT INTO EncryptedData VALUES(\'%f\', %f, NULL);", lat, lon);

   int ret = sqlite3_exec(db, sqlbuff, callback, 0, &ErrMsg);
   if (ret)
   {
      dlog_print(DLOG_DEBUG, LOG_TAG, "PES: Insert Error: %s\n", ErrMsg);
      sqlite3_free(ErrMsg);
   }

   return 0;
}
void save_cb(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "PES: Save clicked\n");

	int ret = InsertRecord(latitude, longitude);
    if (ret) {
        dlog_print(DLOG_DEBUG, LOG_TAG, "PES: Insert MessageError\n");
    }
    else{
    	dlog_print(DLOG_DEBUG, LOG_TAG, "PES: Record Inserted");
    }

   char styled_text[1024];
   snprintf(styled_text, sizeof(styled_text), "<align=center>%f,%f</align>", latitude, longitude);
   elm_object_text_set(label5, styled_text);
}

static void create_base_gui(appdata_s *ad)
{
    /* Window */
    /* Create and initialize elm_win,
       which is mandatory to manipulate a window
    */
    ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
    elm_win_autodel_set(ad->win, EINA_TRUE);

    if (elm_win_wm_rotation_supported_get(ad->win)) {
        int rots[4] = {0, 90, 180, 270};
        elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
    }

    evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
    eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

    /* Conformant */
    /* Create and initialize elm_conformant,
       which is mandatory for the base UI to have a proper size
       when an indicator or virtual keypad is visible
    */
    ad->conform = elm_conformant_add(ad->win);
    elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_HIDE);
    evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(ad->win, ad->conform);
    evas_object_show(ad->conform);

    Evas_Object *grid;
    grid = elm_grid_add(ad->conform);
    elm_grid_size_set(grid, 100, 100);
    elm_object_content_set(ad->conform, grid);
    evas_object_show(grid);
    elm_naviframe_item_push(ad->conform, "Grid", NULL, NULL, grid, NULL);

	/* Label */
	/* Create an actual view of the base gui.
	   Modify this part to change the view. */

	label1 = elm_label_add(grid);
	elm_object_text_set(label1, "1"); //<font align=center>1: </align>
    elm_grid_pack(grid, label1, 2, 20, 100, 20);
    evas_object_show(label1);

	label2 = elm_label_add(grid);
	elm_object_text_set(label2, "2");
    elm_grid_pack(grid, label2, 2, 30, 100, 20);
    evas_object_show(label2);

	label3 = elm_label_add(grid);
	elm_object_text_set(label3, "3");
    elm_grid_pack(grid, label3, 2, 40, 100, 20);
    evas_object_show(label3);

	label4 = elm_label_add(grid);
	elm_object_text_set(label4, "4");
    elm_grid_pack(grid, label4, 2, 50, 100, 20);
    evas_object_show(label4);

	label5 = elm_label_add(grid);
	elm_object_text_set(label5, "<color=#FF4500FF>5</color>");
    elm_grid_pack(grid, label5, 2, 60, 100, 20);
    evas_object_show(label5);


    ad->button = elm_button_add(grid);
    elm_object_text_set(ad->button, "save");
    elm_grid_pack(grid, ad->button, 0, 0, 100, 20);
    evas_object_show(ad->button);
    evas_object_smart_callback_add(ad->button, "clicked", save_cb, NULL);

    ad->button = elm_button_add(grid);
    elm_object_text_set(ad->button, "exit");
    elm_grid_pack(grid, ad->button, 0, 80, 100, 20);
    evas_object_show(ad->button);
    evas_object_smart_callback_add(ad->button, "clicked", clicked_cb, NULL);

    evas_object_show(ad->win);
}


static location_service_state_e service_state;
static void __state_changed_cb(location_service_state_e state, void *user_data)
{
    service_state = state;
    appdata_s *ad = (appdata_s *)user_data;

    double altitude;
	double climb;
	double direction;
	double speed;
	double horizontal;
	double vertical;
	location_accuracy_level_e level;
	time_t timestamp;

	dlog_print(DLOG_DEBUG, LOG_TAG, "PES: Location State Changed\n");

	if (state == LOCATIONS_SERVICE_ENABLED) {
	   location_manager_get_location(ad->location, &altitude, &latitude, &longitude,
										   &climb, &direction, &speed, &level,
										   &horizontal, &vertical, &timestamp);

	   dlog_print(DLOG_DEBUG, LOG_TAG, "PES: Got Location %f %f\n", latitude, longitude);
	   char styled_text[1024];
	   snprintf(styled_text, sizeof(styled_text), "<color=#FF4500FF><align=center>%f,%f</align></color>", latitude, longitude);
	   elm_object_text_set(label5, styled_text);
	}

    dlog_print(DLOG_ERROR, LOG_TAG, "PES: DONE");
}
/* Create the location service */
static void create_location_service(void *data)
{
    dlog_print(DLOG_ERROR, LOG_TAG, "PES: Creating Location Service");
    appdata_s *ad = (appdata_s *)data;

    location_manager_h manager;
    int ret = location_manager_create(LOCATIONS_METHOD_GPS, &manager);
    ad->location = manager;

    ret = location_manager_set_service_state_changed_cb(manager, __state_changed_cb, ad);
    ret = location_manager_start(manager);
}

static int
CreateTable()
{
    char *ErrMsg;
    char *sql = "CREATE TABLE IF NOT EXISTS EncryptedData("  \
                                                          "LATITUDE DOUBLE NOT NULL," \
                                                          "LONGITUDE DOUBLE NOT NULL,"\
                                                          "KEY INTEGER PRIMARY KEY);";

    sqlite3_exec(db, sql, callback, 0, &ErrMsg);
    return 0;
}

static void ShowRecords()
{
   char *sql = "SELECT * FROM (SELECT * FROM EncryptedData ORDER BY KEY DESC LIMIT 4) ORDER BY KEY ASC;";
   int counter = 0;
   char *ErrMsg;

   sqlite3_exec(db, sql, callback, &counter, &ErrMsg);

   return;
}


storage_cb(int storage_id, storage_type_e type, storage_state_e state, const char *path, void *user_data){
    if (type == STORAGE_TYPE_INTERNAL) {
        internal_storage_id = storage_id;
        return false;
    }

    return true;
}
void app_request_response_cb(ppm_call_cause_e cause, ppm_request_result_e result,
const char *privilege, void *user_data)
{
	appdata_s *ad = (appdata_s *)user_data;

    dlog_print(DLOG_ERROR, LOG_TAG, "PES: Permission Asked");


    if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
        /* Log and handle errors */
        return;
    }

    switch (result) {
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
            /* Update UI and start accessing protected functionality */
            dlog_print(DLOG_ERROR, LOG_TAG, "PES: Allow Forever");

            break;
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
            dlog_print(DLOG_ERROR, LOG_TAG, "PES: Deny Forever");
            /* Show a message and terminate the application */
            break;
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
            dlog_print(DLOG_ERROR, LOG_TAG, "PES: Deny Once");
            /* Show a message with explanation */
            break;
    }
}
void app_request_multiple_response_cb(ppm_call_cause_e cause, ppm_request_result_e* results,
                                 const char **privileges, size_t privileges_count, void *user_data)
{
	appdata_s *ad = (appdata_s *)user_data;
    if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
        /* Log and handle errors */
        return;
    }
    for (int it = 0; it < privileges_count; ++it) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "PES: Checking Permission cb %i",it);
        switch (results[it]) {
            case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
                /* Update UI and start accessing protected functionality */
                dlog_print(DLOG_ERROR, LOG_TAG, "PES: Asking Permission");
                break;
            case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
                /* Show a message and terminate the application */
                break;
            case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
                /* Show a message with explanation */
                break;
        }
    }

	int error;
	storage_type_e type;
	error = storage_get_type(internal_storage_id, &type);

	sqlite3_shutdown();
	sqlite3_config(SQLITE_CONFIG_URI, 1);
	sqlite3_initialize();
	char file_path[BUFLEN];
	char *document_path;
	int internal_storage_id = 0;
	storage_get_directory(internal_storage_id, STORAGE_DIRECTORY_DOCUMENTS, &document_path);
	snprintf(file_path, 128, "%s/test.db", document_path);
	dlog_print(DLOG_ERROR, LOG_TAG, "PES: Document Path %s", file_path);
	sqlite3_open(file_path, &db);

	CreateTable();
	dlog_print(DLOG_ERROR, LOG_TAG, "PES: Table Created");

	dlog_print(DLOG_ERROR, LOG_TAG, "PES: Showing Records");
	ShowRecords();

	create_location_service(ad);
}
static bool app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
    dlog_print(DLOG_ERROR, LOG_TAG, "PES: Starting Last Location");
    appdata_s *ad = data;

    ppm_check_result_e results[2];
	const char* privileges [] = {"http://tizen.org/privilege/location",
								 "http://tizen.org/privilege/mediastorage"};
	char* askable_privileges[2];
	int askable_privileges_count = 0;

	int ret = ppm_check_permissions(privileges, sizeof(privileges) / sizeof(privileges[0]), results);
    if (ret == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
		for (int it = 0; it < sizeof(privileges) / sizeof(privileges[0]); ++it)
		{
		    dlog_print(DLOG_ERROR, LOG_TAG, "PES: Checking Permission %i",it);

			switch (results[it]) {
				case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
				/* Update UI and start accessing protected functionality */


					break;
				case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
			   /* Show a message and terminate the application */
					break;
				case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
		                askable_privileges[askable_privileges_count++] = privileges[it];
		                /* Log and handle errors */
					break;
		      }
		}
		ret = ppm_request_permissions(askable_privileges, askable_privileges_count, app_request_multiple_response_cb, ad);
     }
     else {
			/* ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE */
			/* Handle errors */
	}

    dlog_print(DLOG_ERROR, LOG_TAG, "PES: Creating UI");
	create_base_gui(ad);


	int error;
	storage_type_e type;
	error = storage_get_type(internal_storage_id, &type);

	sqlite3_shutdown();
	sqlite3_config(SQLITE_CONFIG_URI, 1);
	sqlite3_initialize();
	char file_path[BUFLEN];
	char *document_path;
	int internal_storage_id = 0;
	storage_get_directory(internal_storage_id, STORAGE_DIRECTORY_DOCUMENTS, &document_path);
	snprintf(file_path, 128, "%s/test.db", document_path);
	dlog_print(DLOG_ERROR, LOG_TAG, "PES: Document Path %s", file_path);
	sqlite3_open(file_path, &db);

	CreateTable();
	dlog_print(DLOG_ERROR, LOG_TAG, "PES: Table Created");

	dlog_print(DLOG_ERROR, LOG_TAG, "PES: Showing Records");
	ShowRecords();

	create_location_service(ad);


	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
    dlog_print(DLOG_ERROR, LOG_TAG, "PES: Paused");
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	dlog_print(DLOG_ERROR, LOG_TAG, "PES: Terminated");
    appdata_s *ad = (appdata_s *)data;
    int ret;

    if (ad->location) {
        ret = location_manager_destroy(ad->location);
        if (ret != LOCATIONS_ERROR_NONE)
            dlog_print(DLOG_ERROR, LOG_TAG,
                       "location_manager_destroy() failed.(%d)", ret);
        else
            ad->location = NULL;
    }
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}

#include "pebble.h"

static Window *s_main_window;
static StatusBarLayer *s_status_bar;
static TextLayer *s_temperature_layer;
static TextLayer *s_city_layer;
static TextLayer *s_label_distance_layer;
static TextLayer *s_distance_layer;
//static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;

static AppSync s_sync;
static uint8_t s_sync_buffer[128];

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
  WEATHER_CITY_KEY = 0x2,         // TUPLE_CSTRING
  DISTANCE_KEY = 0x3,             // TUPLE_CSTRING
  MESSAGE_KEY = 0x4,              // TUPLE_CSTRING
};

enum MessageKey {
  REQUEST_LOCATION = 0x0,        // TUPLE_INT
};

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN, // 0
  RESOURCE_ID_IMAGE_CLOUD, // 1
  RESOURCE_ID_IMAGE_RAIN, // 2
  RESOURCE_ID_IMAGE_SNOW // 3
};

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case WEATHER_ICON_KEY:
      if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }

      s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
      //bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
      //bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
      break;

    case WEATHER_TEMPERATURE_KEY:
      // App Sync keeps new_tuple in s_sync_buffer, so we may use it directly
      text_layer_set_text(s_temperature_layer, new_tuple->value->cstring);
      break;

    case WEATHER_CITY_KEY:
      text_layer_set_text(s_city_layer, new_tuple->value->cstring);
      break;
    
    case DISTANCE_KEY:
      text_layer_set_text(s_distance_layer, new_tuple->value->cstring);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Distance key sent");
      break;
  }
}

static void request_weather(void) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    return;
  }

  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  //Get location every five minutes
  if(tick_time->tm_min % 1 == 0)
  {
    //Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    //Add key value pair
    //static const char *string = "Hello!";
    dict_write_cstring(iter, MESSAGE_KEY, "Hello");
    
    //Send the message
    app_message_outbox_send();
    
  }
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //s_icon_layer = bitmap_layer_create(GRect(0, 10, bounds.size.w, 80));
  //layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));
  
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

  s_label_distance_layer = text_layer_create(GRect(0, 20, bounds.size.w, 18));
  text_layer_set_text_color(s_label_distance_layer, GColorWhite);
  text_layer_set_background_color(s_label_distance_layer, GColorClear);
  text_layer_set_font(s_label_distance_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_label_distance_layer, GTextAlignmentCenter);
  text_layer_set_text(s_label_distance_layer, "Distance remaining:");
  layer_add_child(window_layer, text_layer_get_layer(s_label_distance_layer));
  
  s_distance_layer = text_layer_create(GRect(0, 50, bounds.size.w, 28));
  text_layer_set_text_color(s_distance_layer, GColorWhite);
  text_layer_set_background_color(s_distance_layer, GColorClear);
  text_layer_set_font(s_distance_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_distance_layer, GTextAlignmentCenter);
  /*text_layer_set_text(s_distance_layer, "10 miles");*/
  layer_add_child(window_layer, text_layer_get_layer(s_distance_layer));
  
  s_temperature_layer = text_layer_create(GRect(0, 90, bounds.size.w, 32));
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));

  s_city_layer = text_layer_create(GRect(0, 122, bounds.size.w, 32));
  text_layer_set_text_color(s_city_layer, GColorWhite);
  text_layer_set_background_color(s_city_layer, GColorClear);
  text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_city_layer));

  Tuplet initial_values[] = {
    TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    TupletCString(WEATHER_TEMPERATURE_KEY, "1234\u00B0C"),
    TupletCString(WEATHER_CITY_KEY, "St Pebblesburg"),
    TupletCString(DISTANCE_KEY, "Calc"),
  };

  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  request_weather();
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}


static void window_unload(Window *window) {
  if (s_icon_bitmap) {
    //gbitmap_destroy(s_icon_bitmap);
  }

  text_layer_destroy(s_city_layer);
  text_layer_destroy(s_temperature_layer);
  //bitmap_layer_destroy(s_icon_layer);
}

static void init(void) {
  s_main_window = window_create();
  window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorIndigo, GColorBlack));
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(s_main_window, true);

  app_message_open(64, 64);
}

static void deinit(void) {
  window_destroy(s_main_window);

  app_sync_deinit(&s_sync);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

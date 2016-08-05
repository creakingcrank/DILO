#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_dilo_layer;
static TextLayer *s_rem_layer;

typedef struct {
  char *activity;
  int start_h;
  int start_m;
}dilo_data;

/*

Maybe obvious, but format is: ACTIVITY,START HOUR,START MINS

*/

dilo_data d_data[] =
  {
    {"SLEEP",0,0},
    {"TEA",6,0},
    {"BREAKFAST",7,30},
    {"MAIL",8,30},
    {"PLAN",8,45},
    {"WORK",9,0},
    {"DOG",12,0},
    {"LUNCH",13,0},
    {"MAIL",13,30},  
    {"WORK",13,45},
    {"RUN",16,0},
    {"CLOSE",17,00},
    {"KITCHEN",17,15},
    {"SUPPER",18,0},
    {"FAMILY",19,0},
    {"TEETH",22,0},
    {"SLEEP",22,30},
    {"END",25,0}     //DON'T REMOVE, USED TO DETECT END OF LIST
  };



static int get_dilo_index(int h,int m) {
  
  int i = 0;
  int now_in_min;
  int start_in_min;
  int end_in_min;
  
  while(d_data[i].start_h<24) {
   
    now_in_min = 60*h+m;
    start_in_min = 60*d_data[i].start_h+d_data[i].start_m;
    end_in_min  = 60*d_data[i+1].start_h+d_data[i+1].start_m;
    if ( (now_in_min >= start_in_min) && (now_in_min < end_in_min)) return i;
    i++;
  }
  return 0;
}

static int mins_to_end(int i, int h, int m) {
  
  int now_in_min;
  int end_in_min;
  
  now_in_min = 60*h+m;
  end_in_min  = 60*d_data[i+1].start_h+d_data[i+1].start_m;
  
  return end_in_min - now_in_min;
}

static void update_dilo(struct tm *tick_time) {
  

  static char rem_text_to_display[24];
  
  static int dilo_index = 0;
  int new_dilo_index = get_dilo_index(tick_time->tm_hour, tick_time->tm_min);
  
  if (new_dilo_index != dilo_index) {
    if (( tick_time->tm_hour > 7 ) && ( tick_time->tm_hour < 22 )) vibes_short_pulse();
    dilo_index = new_dilo_index;
  }
 
  text_layer_set_text(s_dilo_layer, d_data[dilo_index].activity);
  
  snprintf(rem_text_to_display,sizeof(rem_text_to_display),"(%dm)",mins_to_end(dilo_index,tick_time->tm_hour, tick_time->tm_min));
  text_layer_set_text(s_rem_layer, rem_text_to_display);
  
}

static void update_time() {
 
  
  
  
   // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  update_dilo(tick_time);
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Create a TextLayer for the DILO information
  s_dilo_layer = text_layer_create( 
    GRect(0, PBL_IF_ROUND_ELSE(108, 102), bounds.size.w, 50));
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_dilo_layer, GColorClear);
  text_layer_set_text_color(s_dilo_layer, GColorBlack);
  text_layer_set_text(s_dilo_layer, "READY");
  text_layer_set_font(s_dilo_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_dilo_layer, GTextAlignmentCenter);
  
   // Create a TextLayer for the TIME REMAINING information
  s_rem_layer = text_layer_create( 
    GRect(0, PBL_IF_ROUND_ELSE(140, 134), bounds.size.w, 50));
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_rem_layer, GColorClear);
  text_layer_set_text_color(s_rem_layer, GColorBlack);
  text_layer_set_text(s_rem_layer, "(000m)");
  text_layer_set_font(s_rem_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_rem_layer, GTextAlignmentCenter);

  // Add them as  children layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_dilo_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_rem_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_dilo_layer);
  text_layer_destroy(s_rem_layer);
  
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
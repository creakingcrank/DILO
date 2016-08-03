#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_dilo_layer;

const char *dilo_activities[] = {    "SLEEP",  "TEA",  "BREAKFAST", "MAIL",  "PLAN", "WORK",  "DOG",  "LUNCH", "WORK",  "RUN",  "CLOSE", "KITCHEN",  "SUPPER",  "FAMILY",  "TEETH", "SLEEP"};
const int dilo_start_times[17][2] = {{0,0},    {6,0},  {7,30},      {8,30},  {9,0},  {9,30},  {12,0},  {13,0},  {13,30},{16,0},  {17,0},  {17,15},  {18,0},    {19,0},    {22,0},     {22,30}, {25,0}};

static int get_dilo_index(int h,int m) {
  
  int i =0;
  int now_in_min;
  int start_in_min;
  int end_in_min;
  
  while(dilo_start_times[i][0]<24) {
   
    now_in_min = 60*h+m;
    start_in_min = 60*dilo_start_times[i][0]+dilo_start_times[i][1];
    end_in_min  = 60*dilo_start_times[i+1][0]+dilo_start_times[i+1][1];
    if ( (now_in_min >= start_in_min) && (now_in_min < end_in_min)) return i;
    i++;
  }
  return 0;
}

static void update_dilo(struct tm *tick_time) {
  
  static int dilo_index = 0;
  int new_dilo_index = get_dilo_index(tick_time->tm_hour, tick_time->tm_min);
  
  if (new_dilo_index != dilo_index) {
    if (( tick_time->tm_hour > 7 ) && ( tick_time->tm_hour < 22 )) vibes_short_pulse();
    dilo_index = new_dilo_index;
  }
  
  text_layer_set_text(s_dilo_layer, dilo_activities[dilo_index]);
  
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

  // Add them as  children layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_dilo_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
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
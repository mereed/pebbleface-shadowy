#include "pebble.h"

static Window *window;
static TextLayer *date_layer; 
static TextLayer *battery_layer;
static InverterLayer *inv_layer;

static GBitmap *separator_image;
static BitmapLayer *separator_layer;

static GBitmap *separator_image2;
static BitmapLayer *separator_layer2;

static GBitmap *blockout_img;
static BitmapLayer *blockout;

static GBitmap *blockout_img2;
static BitmapLayer *blockout2;

static GBitmap *sun_img;
static BitmapLayer *sun_layer;

static GBitmap *moon_img;
static BitmapLayer *moon_layer;

InverterLayer *CityInv = NULL;

#define TOTAL_TIME_DIGITS 4
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

#define TOTAL_TIME_DIGITS2 4
static GBitmap *time_digits_images2[TOTAL_TIME_DIGITS2];
static BitmapLayer *time_digits_layers2[TOTAL_TIME_DIGITS2];

const int BIG_DIGIT2_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_02,
  RESOURCE_ID_IMAGE_NUM_12,
  RESOURCE_ID_IMAGE_NUM_22,
  RESOURCE_ID_IMAGE_NUM_32,
  RESOURCE_ID_IMAGE_NUM_42,
  RESOURCE_ID_IMAGE_NUM_52,
  RESOURCE_ID_IMAGE_NUM_62,
  RESOURCE_ID_IMAGE_NUM_72,
  RESOURCE_ID_IMAGE_NUM_82,
  RESOURCE_ID_IMAGE_NUM_92
};


static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;
  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = (*bmp_image)->bounds.size
  };
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);
  gbitmap_destroy(old_image);
}

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

static void update_hours(struct tm *tick_time) {
  
   unsigned short display_hour = get_display_hour(tick_time->tm_hour);

  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(18, 88));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(41, 88));

  set_container_image(&time_digits_images2[0], time_digits_layers2[0], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(3, 118));
  set_container_image(&time_digits_images2[1], time_digits_layers2[1], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(27, 118));
	
  if (!clock_is_24h_style()) {
	  
	 if (tick_time->tm_hour >= 18 || tick_time->tm_hour <= 6) {

		  layer_set_hidden(inverter_layer_get_layer( CityInv ), false);
 	 	  layer_set_hidden(bitmap_layer_get_layer( sun_layer ), true);
  		  layer_set_hidden(bitmap_layer_get_layer( moon_layer ), false);
	}	  
	    
    if (display_hour/10 == 0) {
		
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers2[0]), true);	  

	} else
		
	  layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers2[0]), false);
    
  }
	
  if (clock_is_24h_style()) {
	
	if (display_hour >= 18 || display_hour <= 6) {
		
		  layer_set_hidden(inverter_layer_get_layer( CityInv ), false);
 	 	  layer_set_hidden(bitmap_layer_get_layer( sun_layer ), true);
  		  layer_set_hidden(bitmap_layer_get_layer( moon_layer ), false);
	}
	  
  }

}


static void update_minutes(struct tm *tick_time) {
  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(82, 88));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(105, 88));

  set_container_image(&time_digits_images2[2], time_digits_layers2[2], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(67, 118));
  set_container_image(&time_digits_images2[3], time_digits_layers2[3], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(94, 118));

}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "+%d%%", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% ", charge_state.charge_percent);
  }
  inverter_layer_destroy(inv_layer);
  inv_layer = inverter_layer_create(GRect(0, 117, (charge_state.charge_percent/10)*15, 1));
  layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
  text_layer_set_text(battery_layer, battery_text);
}

static void handle_bluetooth(bool connected) {
  if (connected == 1){
	  layer_set_hidden(bitmap_layer_get_layer( blockout ), true);
	  layer_set_hidden(bitmap_layer_get_layer( blockout2 ), true);

  } else {	  
	  layer_set_hidden(bitmap_layer_get_layer( blockout ), false);
	  layer_set_hidden(bitmap_layer_get_layer( blockout2 ), false);
  }
}

static void handle_tick(struct tm* tick_time, TimeUnits units_changed) {

  if (units_changed & HOUR_UNIT) {
    update_hours(tick_time);
  }
  if (units_changed & MINUTE_UNIT) {
    update_minutes(tick_time);
  }	

	static char date_text[] = "September 99th, feb.";

switch(tick_time->tm_mday)
  {
    case 1 :
    case 21 :
    case 31 :
      strftime(date_text, sizeof(date_text), "%A %est, %b.", tick_time);
      break;
    case 2 :
    case 22 :
      strftime(date_text, sizeof(date_text), "%A %end, %b.", tick_time);
      break;
    case 3 :
    case 23 :
      strftime(date_text, sizeof(date_text), "%A %erd, %b.", tick_time);
      break;
    default :
      strftime(date_text, sizeof(date_text), "%A %eth, %b.", tick_time);
      break;
  }

  text_layer_set_text(date_layer, date_text);
  handle_battery(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());

}

static void do_init(void) {

  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));

  memset(&time_digits_layers2, 0, sizeof(time_digits_layers2));
  memset(&time_digits_images2, 0, sizeof(time_digits_images2));
	
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorWhite);

  Layer *root_layer = window_get_root_layer(window);
	
  /* Date block */
  date_layer = text_layer_create(GRect(1, 0, 143, 20));
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(date_layer, GTextAlignmentLeft);
  layer_add_child(root_layer, text_layer_get_layer(date_layer));

  /* Battery block */
  battery_layer = text_layer_create(GRect(104, 0, 40, 20 )); // 85 148 
  text_layer_set_text_color(battery_layer, GColorBlack);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentRight);
  text_layer_set_text(battery_layer, "100%");
  layer_add_child(root_layer, text_layer_get_layer(battery_layer));

  separator_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COLON);
  GRect frame = (GRect) {
    .origin = { .x = 68, .y = 92 },
    .size = separator_image->bounds.size
  };
  separator_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(separator_layer, separator_image);
  layer_add_child(root_layer, bitmap_layer_get_layer(separator_layer));   

  separator_image2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COLON2);
  GRect frame2 = (GRect) {
    .origin = { .x = 54, .y = 118 },
    .size = separator_image2->bounds.size
  };
  separator_layer2 = bitmap_layer_create(frame2);
  bitmap_layer_set_bitmap(separator_layer2, separator_image2);	
  GCompOp compositing_mode_sep = GCompOpAnd;
  bitmap_layer_set_compositing_mode(separator_layer2, compositing_mode_sep);	  
  layer_add_child(root_layer, bitmap_layer_get_layer(separator_layer2));   
	
	GRect dummy_frame = { {0, 0}, {0, 0} };

   for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(root_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
	
  for (int i = 0; i < TOTAL_TIME_DIGITS2; ++i) {
    time_digits_layers2[i] = bitmap_layer_create(dummy_frame);
	GCompOp compositing_mode = GCompOpAnd;
	bitmap_layer_set_compositing_mode(time_digits_layers2[i], compositing_mode);
    layer_add_child(root_layer, bitmap_layer_get_layer(time_digits_layers2[i]));
  }
	
  sun_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUN);
  GRect frame_sun = (GRect) {
    .origin = { .x = 27, .y = 15 },
    .size = sun_img->bounds.size
  };
  sun_layer = bitmap_layer_create(frame_sun);
  bitmap_layer_set_bitmap(sun_layer, sun_img);
  layer_add_child(root_layer, bitmap_layer_get_layer(sun_layer));   
  layer_set_hidden(bitmap_layer_get_layer( sun_layer ), true);
	
  moon_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MOON);
  GRect frame_moon = (GRect) {
    .origin = { .x = 100, .y = 17 },
    .size = moon_img->bounds.size
  };
  moon_layer = bitmap_layer_create(frame_moon);
  bitmap_layer_set_bitmap(moon_layer, moon_img);
  layer_add_child(root_layer, bitmap_layer_get_layer(moon_layer));   
  layer_set_hidden(bitmap_layer_get_layer( moon_layer ), true);
	
	
  blockout_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLOCKOUT);
  GRect frame_blockout = (GRect) {
    .origin = { .x = 0, .y = 117 },
    .size = blockout_img->bounds.size
  };
  blockout = bitmap_layer_create(frame_blockout);
  bitmap_layer_set_bitmap(blockout, blockout_img);
  layer_add_child(root_layer, bitmap_layer_get_layer(blockout));   
  layer_set_hidden(bitmap_layer_get_layer( blockout ), true);

  blockout_img2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLOCKOUT2);
  GRect frame_blockout2 = (GRect) {
    .origin = { .x = 66, .y = 17 },
    .size = blockout_img2->bounds.size
  };
  blockout2 = bitmap_layer_create(frame_blockout2);
  bitmap_layer_set_bitmap(blockout2, blockout_img2);
  layer_add_child(root_layer, bitmap_layer_get_layer(blockout2));   
  layer_set_hidden(bitmap_layer_get_layer( blockout2 ), true);	

  // Add inverter layer
	CityInv = inverter_layer_create(GRect(0, 0, 144, 168));
    layer_add_child(root_layer, inverter_layer_get_layer(CityInv));
	layer_set_hidden(inverter_layer_get_layer( CityInv ), true);
	
	
  /* Init blocks */
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_tick(current_time, HOUR_UNIT + MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_tick);
  battery_state_service_subscribe(&handle_battery);

  bool connected = bluetooth_connection_service_peek();
  handle_bluetooth(connected);
  bluetooth_connection_service_subscribe(&handle_bluetooth);

}

static void do_deinit(void) {
	
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
	
  inverter_layer_destroy(inv_layer);
  inverter_layer_destroy(CityInv);
	
  text_layer_destroy(date_layer);
  text_layer_destroy(battery_layer);
	
  layer_remove_from_parent(bitmap_layer_get_layer(separator_layer));
  bitmap_layer_destroy(separator_layer);
  gbitmap_destroy(separator_image);
	
  layer_remove_from_parent(bitmap_layer_get_layer(separator_layer2));
  bitmap_layer_destroy(separator_layer2);
  gbitmap_destroy(separator_image2);
	
  layer_remove_from_parent(bitmap_layer_get_layer(blockout));
  bitmap_layer_destroy(blockout);
  gbitmap_destroy(blockout_img);
	
  layer_remove_from_parent(bitmap_layer_get_layer(blockout2));
  bitmap_layer_destroy(blockout2);
  gbitmap_destroy(blockout_img2);
	
  layer_remove_from_parent(bitmap_layer_get_layer(moon_layer));
  bitmap_layer_destroy(moon_layer);
  gbitmap_destroy(moon_img);
	
  layer_remove_from_parent(bitmap_layer_get_layer(sun_layer));
  bitmap_layer_destroy(sun_layer);
  gbitmap_destroy(sun_img);
	
  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    bitmap_layer_destroy(time_digits_layers[i]);
  } 
	
	  for (int i = 0; i < TOTAL_TIME_DIGITS2; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers2[i]));
    gbitmap_destroy(time_digits_images2[i]);
    bitmap_layer_destroy(time_digits_layers2[i]);
  } 
  window_destroy(window);
}

int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}
//RADIATION ENGINE v1.0.1
//Mapping tools! Just working off our stable code base right now.
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>

using namespace std;

int tile_w, tile_h;

float FPS = 60;
int SCREEN_W = 640;
int SCREEN_H = 480;
enum MYKEYS {KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_LEFT, KEY_ENTER, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE, KEY_ZERO};
float pointer_x, pointer_y, enterheld, numheld;
string curnum;
stringstream convert;
int font_size = 32;
bool canenter = true;

ALLEGRO_DISPLAY *display = NULL; //Basic display pointer
ALLEGRO_EVENT_QUEUE *evt_q = NULL; //Basic event queue
ALLEGRO_TIMER *timer = NULL; //Basic timer pointer
ALLEGRO_CONFIG *config_ld = al_load_config_file("config.ini"); //Main config for engine
ALLEGRO_CONFIG *map_cfg = al_load_config_file("maps.mp"); //Loads map file
ALLEGRO_CONFIG *tile_cfg = al_load_config_file("tiles.tl"); //Load the tile loading configuration file
ALLEGRO_FONT *font = NULL;

struct tile{ //Basic tile declaration
	ALLEGRO_BITMAP* image; //The image referenced for each tile
	int id; //Image ref-id. Used for determing whether or not there are tile effects applied
};

struct tile_map{ //Basic map declaration
	string raw_data; //Really, this fucking vector thing is being a bitch. I'm not down with dynamic
	vector<int> formatted_data;
	int id; //Map ref-id. Used to determine which floor is being displayed
	string title; //Title of the map being displayed
};

struct level{ //Basic level declaration
	tile_map lvl[10][10]; //10x10 grid of maps which make up the entire level 
	int id; //Level ref-id. Used to determine which level is being displayed
	string title; //Title of level being used
};

//tile_map curmap; //Memory-loaded map and level variables, something static (roughly) so as to decrease lag and shit
//level curlvl;

tile tile_reg[100]; //There is space for 100 unique tile entities with this
tile_map mapparoo;

void save(){
	al_set_config_value(map_cfg, "map_data", "m", mapparoo.raw_data.c_str());
	al_save_config_file("map.mp", map_cfg);
}

void populate_map(){
	for(int i = 0; i < tile_w*tile_h; i++){
		mapparoo.formatted_data.push_back(0);
	}
}

double round(double d){
	return floor(d + 0.5);
}

ALLEGRO_COLOR color_tiles(int x, int y){
	if(x == round(pointer_x) && y == round(pointer_y)){
		return al_map_rgb(225,60,60);
	}
	else{ return al_map_rgb(225,225,225); }
}

void draw_map(){
	int temp_i;
	string temp_s;
	for(int x=0; x < tile_w; x++){
		for(int y=0; y < tile_h; y++){
			convert.clear();
			convert.str(""); //Clear the conversion buffer
			temp_i = mapparoo.formatted_data.at((y*tile_w)+x); //Grab our lovely vector thingy
			convert << temp_i; //Convert it to string
			temp_s = convert.str();

			//curnum = mapparoo.raw_data.at((y*tile_w)+x);
			al_draw_text(font, color_tiles(x, y), x*font_size + (font_size/2), y*font_size,ALLEGRO_ALIGN_CENTRE, temp_s.c_str());; //Minor fix here. Originally reliant on a variable, now actually reliant on a constant
		}
	}
}

string make_str(int input){
	string temp_s;
	convert.clear();
	convert.str("");
	convert << input;
	temp_s = convert.str();

	return temp_s;
}

void make_raw(){
	string temp_s;
	string convert_temp;
	int temp_i;

	for(int i = 0; i < tile_w*tile_h; i++){
		temp_s.push_back('-'); //Start delimit
		temp_i = mapparoo.formatted_data.at(i);
		for(int j = 0; j < make_str(temp_i).length(); j++){
			convert_temp = make_str(temp_i);
			temp_s.push_back(convert_temp.at(j)); //Data (in non-ASCII format)
		}
		//Okay, how do we do this? I'm thinking that maybe we try and get the distance between the delimiters,
		//then loop with that and add each digit individually. Because obviously, this isn't going to work.
		//Okay, so let's try it where we determine the length by calling make_str(), and then adding each digit individually.
		temp_s.push_back('+'); //End delimit
	}

	cout << temp_s;
	cout << "\n";
	for(int i = 0; i < temp_s.length(); i++){ //Set our map string equal to the temporary string (hopefully no memory access errors this time)
		mapparoo.raw_data.push_back(temp_s.at(i));
	}
	cout << mapparoo.raw_data;
	cout << "\n";
}

void edit_entry(int x, int y){
	cout << "Enter entry: ";
	
	string input;
	int temp_i;

	cin >> input;
	temp_i = atoi(input.c_str());

	mapparoo.formatted_data[y*tile_w + x] = temp_i;

	cout << "\n";
	cout << mapparoo.formatted_data.at((y*tile_w)+x);

	canenter = true;
}

/*
void edit(char id){
	string temp;
	int temp_i;
	temp.push_back(id);

	cin >> temp;
	temp_i = atoi(temp.c_str());

	mapparoo.formatted_data.push_back(temp_i);

	//mapparoo.raw_data[round(pointer_y)*tile_w + round(pointer_x)] = temp[0];
}*/

void apply_main_config(){
	//Setting the screen resolution as per the configuration file
	SCREEN_W = atoi(al_get_config_value(config_ld, "SCREENRES", "w"));
	SCREEN_H = atoi(al_get_config_value(config_ld, "SCREENRES", "h"));
	FPS = atoi(al_get_config_value(config_ld, "FPS", "f"));
	font_size = atoi(al_get_config_value(config_ld, "FONTSIZE", "size"));

	fprintf(stdout, "Screen width: ");
	fprintf(stdout, al_get_config_value(config_ld, "SCREENRES", "w"));
	fprintf(stdout, "\n");
	fprintf(stdout, "Screen height: ");
	fprintf(stdout, al_get_config_value(config_ld, "SCREENRES", "h"));
	fprintf(stdout, "\n");
	fprintf(stdout, "FPS: ");
	fprintf(stdout, al_get_config_value(config_ld, "FPS", "f"));
	fprintf(stdout, "\n");
}

int init_engine(){
	if(!al_init()) {
      fprintf(stderr, "failed to initialize allegro!\n");
      return -1;
   }
   else{ fprintf(stdout, "Allegro initialized!\n"); }

   if(!al_init_image_addon()){
	   fprintf(stderr, "failed to initialize image addon!\n");
	   return -2;
   }
   else{ fprintf(stdout, "Allegro Image addon initialized!\n"); }

   if(!al_install_keyboard()){
	   fprintf(stderr, "failed to install keyboard!\n");
	   return -3;
   }
   else{ fprintf(stdout, "Keyboard initialized!\n"); }

   if(!config_ld){
	   fprintf(stderr, "failed to load config file! setting to defaults...\n");
   }
   else{
	   apply_main_config(); fprintf(stdout, "Main config file loaded!\n");
   }

   if(!tile_cfg){
	   fprintf(stderr, "Failed to load tile config file!\n");
	   al_rest(5);
	   return -4;
   }
   else { fprintf(stdout, "Tile file loaded!\n"); }

   display = al_create_display(SCREEN_W, SCREEN_H);
   if(!display) {
      fprintf(stderr, "failed to create display!\n");
      return -6;
   }
   else{ fprintf(stdout, "Display initialized!\n"); }

   evt_q = al_create_event_queue();
   if(!evt_q){
	   al_destroy_display(display);
	   fprintf(stderr, "failed to initialize event queue!\n");
	   return -7;
   }
   else{ fprintf(stdout, "Event queue initialized!\n"); }

   timer = al_create_timer(1.0 / FPS);
   if(!timer){
	   al_destroy_display(display);
	   al_destroy_event_queue(evt_q);
	   fprintf(stderr, "failed to initialize timer!\n");
	   return -8;
   }
   else{ fprintf(stdout, "Timer initialized!\n"); }

   al_init_font_addon();
   al_init_ttf_addon();

   font = al_load_ttf_font("pirulen.ttf", font_size, 0);

   cout << "Enter width of map (in tiles): ";
   cin >> tile_w;
   cout << "\n";
   cout << "Enter height of map (in tiles): ";
   cin >> tile_h;
   cout << "\n";

   populate_map();

   pointer_x = 0;
   pointer_y = 0;
}

int main(int argc, char **argv)
{
	bool redraw = true;
	bool doexit = false;
	numheld = 0;
	bool key[15] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
   
	char error_status = init_engine();

   al_set_target_bitmap(al_get_backbuffer(display));

   al_register_event_source(evt_q, al_get_display_event_source(display)); //Register event source from display
   al_register_event_source(evt_q, al_get_timer_event_source(timer)); //Register event source from timer
   al_register_event_source(evt_q, al_get_keyboard_event_source()); //Register the keyboard for event sources
 
   al_clear_to_color(al_map_rgb(0,0,0));
 
   al_flip_display();

   al_start_timer(timer);

   while(!doexit){

	   ALLEGRO_EVENT ev; //Event variable for this scope
	   ALLEGRO_TIMEOUT timeout; //Timeout variable for refresh of event queue
	   al_init_timeout(&timeout, 0.06); //Set timeout to 60 milliseconds (60 Hz)

	   bool get_event = al_wait_for_event_until(evt_q, &ev, &timeout); //Shorten the al_wait_for_event_until(...) to something a bit more manageable

	   if(ev.type == ALLEGRO_EVENT_TIMER){ //Basic ping from timer
		   if(key[KEY_UP])
			   pointer_y -= 0.1;
		   if(key[KEY_DOWN])
			   pointer_y += 0.1;
		   if(key[KEY_LEFT])
			   pointer_x -= 0.1;
		   if(key[KEY_RIGHT])
			   pointer_x += 0.1;
		   if(key[KEY_ENTER])
			   enterheld += 0.1;
		   if(key[KEY_ZERO])
			   numheld += 0.3;
		   if(key[KEY_ONE])
			   numheld += 0.3;
		   if(key[KEY_TWO])
			   numheld += 0.3;
		   if(key[KEY_THREE])
			   numheld += 0.3;
		   if(key[KEY_FOUR])
			   numheld += 0.3;
		   if(key[KEY_FIVE])
			   numheld += 0.3;
		   if(key[KEY_SIX])
			   numheld += 0.3;
		   if(key[KEY_SEVEN])
			   numheld += 0.3;
		   if(key[KEY_EIGHT])
			   numheld += 0.3;
		   if(key[KEY_NINE])
			   numheld += 0.3;

		   if(pointer_x <= 0)
			   pointer_x = 0;
		   if(pointer_x >= tile_w)
			   pointer_x = tile_w;
		   if(pointer_y <= 0)
			   pointer_y = 0;
		   if(pointer_y >= tile_h)
			   pointer_y = tile_h;
		   if(enterheld >= 1){
			   enterheld = 0;
			   canenter=false;
			   edit_entry(round(pointer_x), round(pointer_y));
		   }
		   if(numheld >= 1){
				/*if(key[KEY_ZERO])
					edit('0');
				if(key[KEY_ONE])
					edit('1');
				if(key[KEY_TWO])
					edit('2');
				if(key[KEY_THREE])
					edit('3');
				if(key[KEY_FOUR])
					edit('4');
				if(key[KEY_FIVE])
					edit('5');
				if(key[KEY_SIX])
					edit('6');
				if(key[KEY_SEVEN])
					edit('7');
				if(key[KEY_EIGHT])
					edit('8');
				if(key[KEY_NINE])
					edit('9');
			   numheld = 0;*/
		   }

		   redraw = true;
	   }
	   else if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
		   switch(ev.keyboard.keycode){
		   case ALLEGRO_KEY_UP : key[KEY_UP] = true; break;
		   case ALLEGRO_KEY_DOWN : key[KEY_DOWN] = true; break;
		   case ALLEGRO_KEY_LEFT : key[KEY_LEFT] = true; break;
		   case ALLEGRO_KEY_RIGHT : key[KEY_RIGHT] = true; break;
		   case ALLEGRO_KEY_ENTER : key[KEY_ENTER] = canenter; break;
		   case ALLEGRO_KEY_0 : key[KEY_ZERO] = true; break;
		   case ALLEGRO_KEY_1 : key[KEY_ONE] = true; break;
		   case ALLEGRO_KEY_2 : key[KEY_TWO] = true; break;
		   case ALLEGRO_KEY_3 : key[KEY_THREE] = true; break;
		   case ALLEGRO_KEY_4 : key[KEY_FOUR] = true; break;
		   case ALLEGRO_KEY_5 : key[KEY_FIVE] = true; break;
		   case ALLEGRO_KEY_6 : key[KEY_SIX] = true; break;
           case ALLEGRO_KEY_7 : key[KEY_SEVEN] = true; break;
		   case ALLEGRO_KEY_8 : key[KEY_EIGHT] = true; break;
		   case ALLEGRO_KEY_9 : key[KEY_NINE] = true; break;
		   case ALLEGRO_KEY_S : save(); break;
		   case ALLEGRO_KEY_E : make_raw(); break;
		   default : ;
		   }
	   }
	   else if(ev.type == ALLEGRO_EVENT_KEY_UP){
		   switch(ev.keyboard.keycode){
		   case ALLEGRO_KEY_UP : key[KEY_UP] = false; break;
		   case ALLEGRO_KEY_DOWN : key[KEY_DOWN] = false; break;
		   case ALLEGRO_KEY_LEFT : key[KEY_LEFT] = false; break;
		   case ALLEGRO_KEY_RIGHT : key[KEY_RIGHT] = false; break;
		   case ALLEGRO_KEY_ENTER : key[KEY_ENTER] = false; break;
		   case ALLEGRO_KEY_ESCAPE : doexit = true; break;
		   case ALLEGRO_KEY_0 : key[KEY_ZERO] = false; break;
		   case ALLEGRO_KEY_1 : key[KEY_ONE] = false; break;
		   case ALLEGRO_KEY_2 : key[KEY_TWO] = false; break;
		   case ALLEGRO_KEY_3 : key[KEY_THREE] = false; break;
		   case ALLEGRO_KEY_4 : key[KEY_FOUR] = false; break;
		   case ALLEGRO_KEY_5 : key[KEY_FIVE] = false; break;
		   case ALLEGRO_KEY_6 : key[KEY_SIX] = false; break;
           case ALLEGRO_KEY_7 : key[KEY_SEVEN] = false; break;
		   case ALLEGRO_KEY_8 : key[KEY_EIGHT] = false; break;
		   case ALLEGRO_KEY_9 : key[KEY_NINE] = false; break;
		   default : ;
		   }
	   }
	   else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
		   doexit = true;
		   break;
	   }

	   if(redraw && al_is_event_queue_empty(evt_q)){
		al_clear_to_color(al_map_rgb(0,0,0)); //Clear the foreground (kinda expensive)
		redraw = false; //Make sure we don't redraw again till the next 60 frames
		draw_map(); //Call the custom map printing function
		al_flip_display(); //Bring buffer up
	   }


   }
 
   al_destroy_timer(timer);
   al_destroy_event_queue(evt_q);
   al_destroy_display(display);
   al_destroy_config(config_ld);

   al_uninstall_keyboard();
   al_uninstall_system();
 
   return 0;
}
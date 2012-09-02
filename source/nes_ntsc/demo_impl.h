/* Simple shell used by demos. Uses SDL multimedia library. */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SDL.h"

/* Image loader */
typedef struct image_t
{
	unsigned char const* byte_pixels;/* 8-bit pixels */
	unsigned short const* rgb_16;    /* 16-bit pixels */
	int width;
	int height;
	int row_width; /* number of pixels to get to next row (may be greater than width) */
} image_t;
/* if no palette, loads as 16-bit RGB */
void load_bmp( image_t* out, const char* path, SDL_Color palette [256] );
void save_bmp( const char* path );
void init_window( int width, int height );
int read_input( void );
void lock_pixels( void );
void double_output_height( void );
void display_output( void );
void fatal_error( const char* str );

static unsigned char* output_pixels; /* 16-bit RGB */
static long output_pitch;
static float mouse_x, mouse_y; /* -1.0 to 1.0 */
static int mouse_moved;
static int key_pressed;

/* implementation */

static SDL_Rect rect;
static SDL_Surface* screen;
static SDL_Surface* surface;
static unsigned long next_time;

void fatal_error( const char* str )
{
	fprintf( stderr, "Error: %s\n", str );
	exit( EXIT_FAILURE );
}

static void init_sdl_( void )
{
	static int initialized;
	if ( !initialized )
	{
		if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
			fatal_error( "SDL initialization failed" );
		atexit( SDL_Quit );
	}
}

void init_window( int width, int height )
{
	rect.w = width;
	rect.h = height;
	
	init_sdl_();
	
	screen = SDL_SetVideoMode( width, height, 0, 0 );
	surface = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 16, 0, 0, 0, 0 );
	if ( !screen || !surface )
		fatal_error( "SDL initialization failed" );
	
	SDL_WM_SetCaption( "NTSC Filter Demo", "NTSC Filter Demo" );
}

int read_input( void )
{
	SDL_Event e;
	
	/* limit to 60 calls per second */
	unsigned long start = SDL_GetTicks();
	if ( start < next_time && next_time - start > 10 )
		SDL_Delay( next_time - start );
	while ( SDL_GetTicks() < next_time ) { }
	next_time = start + 1000 / 60;
	
	mouse_moved = 0;
	key_pressed = 0;
	
	while ( SDL_PollEvent( &e ) )
	{
		if ( e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_QUIT )
			return 0;
		
		if ( e.type == SDL_KEYDOWN )
		{
			if ( e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q )
				return 0;
			key_pressed = e.key.keysym.sym;
		}
		
		if ( e.type == SDL_MOUSEMOTION )
		{
			int x, y;
			SDL_GetMouseState( &x, &y );
			mouse_moved = 1;
			mouse_x = x / (float) (SDL_GetVideoSurface()->w - 1) * 2 - 1;
			mouse_y = (1 - y / (float) (SDL_GetVideoSurface()->h - 1)) * 2 - 1;
		}
	}
	return 1;
}

void lock_pixels( void )
{
	if ( SDL_LockSurface( surface ) < 0 )
		fatal_error( "Couldn't lock surface" );
	SDL_FillRect( surface, 0, 0 );
	output_pitch = surface->pitch;
	output_pixels = (unsigned char*) surface->pixels;
}

void double_output_height( void )
{
	int y;
	for ( y = surface->h / 2; --y >= 0; )
	{
		unsigned char const* in = output_pixels + y * output_pitch;
		unsigned char* out = output_pixels + y * 2 * output_pitch;
		int n;
		for ( n = surface->w; n; --n )
		{
			unsigned prev = *(unsigned short*) in;
			unsigned next = *(unsigned short*) (in + output_pitch);
			/* mix 16-bit rgb without losing low bits */
			unsigned mixed = prev + next + ((prev ^ next) & 0x0821);
			/* darken by 12% */
			*(unsigned short*) out = prev;
			*(unsigned short*) (out + output_pitch) = (mixed >> 1) - (mixed >> 4 & 0x18E3);
			in += 2;
			out += 2;
		}
	}
}

void display_output( void )
{
	SDL_UnlockSurface( surface );
	if ( SDL_BlitSurface( surface, &rect, screen, &rect ) < 0 || SDL_Flip( screen ) < 0 )
		fatal_error( "SDL blit failed" );
}

void load_bmp( image_t* out, const char* path, SDL_Color palette [256] )
{
	SDL_PixelFormat fmt = { 0 }; /* clear fields */
	SDL_Palette pal = { 0 };
	SDL_Surface* bmp;
	SDL_Surface* conv;
	
	init_sdl_();
	bmp = SDL_LoadBMP( path );
	if ( !bmp )
		fatal_error( "Couldn't load BMP" );
	
	fmt.BitsPerPixel  = 16;
	fmt.BytesPerPixel = 2;
	if ( palette )
	{
		pal.ncolors = 256;
		pal.colors = palette;
		fmt.palette = &pal;
		fmt.BitsPerPixel  = 8;
		fmt.BytesPerPixel = 1;
	}
	conv = SDL_ConvertSurface( bmp, &fmt, SDL_SWSURFACE );
	if ( !conv )
		fatal_error( "Couldn't convert BMP" );
	SDL_FreeSurface( bmp );
	
	if ( SDL_LockSurface( conv ) < 0 )
		fatal_error( "Couldn't lock surface" );
	
	out->byte_pixels = (unsigned char *) conv->pixels;
	out->rgb_16      = (unsigned short*) conv->pixels;
	out->width       = conv->w;
	out->height      = conv->h;
	out->row_width   = conv->pitch / fmt.BytesPerPixel;
}

void save_bmp( const char* path )
{
	if ( SDL_SaveBMP( surface, path ) )
		fatal_error( "Couldn't save BMP" );
}

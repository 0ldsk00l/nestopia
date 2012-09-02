/* Displays and saves NTSC filtered image. Mouse controls sharpness and gamma.
Defaults to using "test.bmp" for input and "filtered.bmp" for output. Input
image must be an uncompressed BMP. Also writes "nes.pal" RGB color file on exit.

Usage: demo [in.bmp [out.bmp]]

Space   Toggle field merging
C       Composite video quality
S       S-video quality
R       RGB video quality
M       Monochrome video quality
D       Toggle between standard and Sony decoder matrix
*/

#include "nes_ntsc.h"

#include "demo_impl.h"

/* only used to convert input image to native palette format */
static SDL_Color palette [256] = {
	{102,102,102},{  0, 42,136},{ 20, 18,168},{ 59,  0,164},
	{ 92,  0,126},{110,  0, 64},{108,  7,  0},{ 87, 29,  0},
	{ 52, 53,  0},{ 12, 73,  0},{  0, 82,  0},{  0, 79,  8},
	{  0, 64, 78},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{174,174,174},{ 21, 95,218},{ 66, 64,254},{118, 39,255},
	{161, 27,205},{184, 30,124},{181, 50, 32},{153, 79,  0},
	{108,110,  0},{ 56,135,  0},{ 13,148,  0},{  0,144, 50},
	{  0,124,142},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{254,254,254},{100,176,254},{147,144,254},{199,119,254},
	{243,106,254},{254,110,205},{254,130,112},{235,159, 35},
	{189,191,  0},{137,217,  0},{ 93,229, 48},{ 69,225,130},
	{ 72,206,223},{ 79, 79, 79},{  0,  0,  0},{  0,  0,  0},
	{254,254,254},{193,224,254},{212,211,254},{233,200,254},
	{251,195,254},{254,197,235},{254,205,198},{247,217,166},
	{229,230,149},{208,240,151},{190,245,171},{180,243,205},
	{181,236,243},{184,184,184},{  0,  0,  0},{  0,  0,  0}
};

int main( int argc, char** argv )
{
	image_t image;
	int sony_decoder = 0;
	int merge_fields = 1;
	int burst_phase = 0;
	nes_ntsc_setup_t setup = nes_ntsc_composite;

	nes_ntsc_t* ntsc = (nes_ntsc_t*) malloc( sizeof (nes_ntsc_t) );
	if ( !ntsc )
		fatal_error( "Out of memory" );
	nes_ntsc_init( ntsc, &setup );
	
	load_bmp( &image, (argc > 1 ? argv [1] : "test.bmp"), palette );
	init_window( NES_NTSC_OUT_WIDTH( image.width ), image.height * 2 );
	
	while ( read_input() )
	{
		lock_pixels();
		
		burst_phase ^= 1;
		if ( setup.merge_fields )
			burst_phase = 0;
		
		nes_ntsc_blit( ntsc, image.byte_pixels, image.row_width, burst_phase,
				image.width, image.height, output_pixels, output_pitch );
		
		double_output_height();
		display_output();
		
		switch ( key_pressed )
		{
			case ' ': merge_fields = !merge_fields; break;
			case 'c': setup = nes_ntsc_composite; break;
			case 's': setup = nes_ntsc_svideo; break;
			case 'r': setup = nes_ntsc_rgb; break;
			case 'm': setup = nes_ntsc_monochrome; break;
			case 'd': sony_decoder = !sony_decoder; break;
		}
		
		if ( key_pressed || mouse_moved )
		{
			setup.merge_fields = merge_fields;
			
			/* available parameters: hue, saturation, contrast, brightness,
			sharpness, gamma, bleed, resolution, artifacts, fringing */
			setup.sharpness = mouse_x;
			setup.gamma     = mouse_y;
			
			setup.decoder_matrix = 0;
			if ( sony_decoder )
			{
				/* Sony CXA2025AS US */
				static float matrix [6] = { 1.630, 0.317, -0.378, -0.466, -1.089, 1.677 };
				setup.decoder_matrix = matrix;
			}
			
			nes_ntsc_init( ntsc, &setup );
		}
	}
	
	save_bmp( argc > 2 ? argv [2] : "filtered.bmp" );
	
	free( ntsc );
	
	/* write standard 192-byte NES palette */
	{
		FILE* out = fopen( "nes.pal", "wb" );
		if ( out )
		{
			unsigned char palette [nes_ntsc_palette_size * 3];
			setup.palette_out = palette;
			nes_ntsc_init( 0, &setup );
			fwrite( palette, 192, 1, out );
		}
	}
	
	return 0;
}

/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2013 R. Danbrook
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "main.h"
#include "cli.h"
#include "config.h"

extern settings *conf;

void cli_error(char *message) {
	cli_show_usage();
	fprintf(stderr, "%s\n", message);
	exit(1);
}

void cli_show_usage() {
	printf("Usage: nestopia [options] [FILE]\n");
	printf("\nOptions:\n");
	printf("  -d, --disablegui        Disable GTK+ GUI\n");
	printf("  -e, --enablegui         Enable GTK+ GUI\n\n");
	printf("  -f, --fullscreen        Fullscreen mode (-f 0 for windowed mode)\n\n");
	printf("  -l, --filter            Video Filter\n");
	printf("                          (0=none, 1=ntsc, 2=xbr, 3=hqx, 4=2xsai, 5=scalex)\n\n");
	printf("  -m, --maskoverscan      Mask Overscan areas (-m 0 to show overscan)\n\n");
	printf("  -s, --scalefactor       Video scale factor (1-4)\n\n");
	printf("  -t, --tvaspect          TV aspect ratio (-t 0 for normal aspect)\n\n");
	printf("  -u, --unlimitedsprites  Remove sprite limit (-u 0 to limit sprites)\n\n");
	printf("  -v, --version           Show version information\n\n");
	printf("More options can be set in the configuration file.\n");
	printf("Options are saved, and do not need to be set on future invocations.\n\n");
}

void cli_show_version() {
	printf("Nestopia UE %s\n", VERSION);
}

void cli_handle_command(int argc, char *argv[]) {
	int c;
	int optint;

	while (1) {
		static struct option long_options[] = {
			{"disablegui", no_argument, 0, 'd'},
			{"enablegui", no_argument, 0, 'e'},
			{"fullscreen", optional_argument, 0, 'f'},
			{"help", no_argument, 0, 'h'},
			{"filter", required_argument, 0, 'l'},
			{"maskoverscan", optional_argument, 0, 'm'},
			{"scalefactor", required_argument, 0, 's'},
			{"tvaspect", optional_argument, 0, 't'},
			{"unlimitedsprites", optional_argument, 0, 'u'},
			{"version", no_argument, 0, 'v'},
			{0, 0, 0, 0}
		};
		
		int option_index = 0;
		
		c = getopt_long(argc, argv, "defhl:m:s:t:u:v",
			long_options, &option_index);
		
		if (c == -1) { break; }
		
		switch(c) {
			case 'd':
				conf->misc_disable_gui = true;
				break;
			
			case 'e':
				conf->misc_disable_gui = false;
				break;
			
			case 'f':
				if (optarg && atoi(optarg) == 0) {
					conf->video_fullscreen = false;
				}
				else {
					conf->video_fullscreen = true;
				}
				break;
			
			case 'h':
				cli_show_usage();
				exit(0);
				break;
			
			case 'l':
				optint = atoi(optarg);
				if (optint < 6) {
					conf->video_filter = optint;
				}
				else {
					cli_error("error: invalid filter");
				}
				break;
			
			case 'm':
				if (optarg && atoi(optarg) == 0) {
					conf->video_mask_overscan = false;
				}
				else {
					conf->video_mask_overscan = true;
				}
				break;
			
			case 's':
				optint = atoi(optarg);
				if (optint < 5 && optint != 0) {
					conf->video_scale_factor = optint;
				}
				else {
					cli_error("error: invalid scale factor");
				}
				break;
			
			case 't':
				if (optarg && atoi(optarg) == 0) {
					conf->video_tv_aspect = false;
				}
				else {
					conf->video_tv_aspect = true;
				}
				break;
			
			case 'u':
				if (optarg && atoi(optarg) == 0) {
					conf->video_unlimited_sprites = false;
				}
				else {
					conf->video_unlimited_sprites = true;
				}
				break;
			
			case 'v':
				cli_show_version();
				exit(0);
				break;
			
			default:
				cli_error("error: invalid option");
				break;
		}
	}
}

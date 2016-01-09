/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2016 R. Danbrook
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

extern settings_t conf;

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
	printf("  -f, --fullscreen        Fullscreen mode\n");
	printf("  -w, --window            Window mode\n\n");
	printf("  -l, --filter            Video Filter\n");
	printf("                          (0=None, 1=NTSC, 2=xBR, 3=HqX, 4=2xSaI, 5=ScaleX)\n\n");
	printf("  -m, --maskoverscan      Mask overscan areas\n");
	printf("  -n, --no-maskoverscan   Disable overscan masking\n\n");
	printf("  -o, --stretchfs         Stretch to native resolution in fullscreen mode\n");
	printf("  -p, --preserveaspect    Preserve aspect ratio in fullscreen mode\n\n");
	printf("  -s, --scalefactor       Video scale factor (1-4)\n\n");
	printf("  -t, --tvaspect          TV aspect ratio\n");
	printf("  -r, --no-tvaspect       Regular aspect ratio\n\n");
	printf("  -u, --unlimitedsprites  Remove sprite limit\n");
	printf("  -q, --spritelimit       Enable sprite limit\n\n");
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
			{"fullscreen", no_argument, 0, 'f'},
			{"window", no_argument, 0, 'w'},
			{"help", no_argument, 0, 'h'},
			{"filter", required_argument, 0, 'l'},
			{"maskoverscan", no_argument, 0, 'm'},
			{"no-maskoverscan", no_argument, 0, 'n'},
			{"stretchfs", no_argument, 0, 'o'},
			{"preserveaspect", no_argument, 0, 'p'},
			{"scalefactor", required_argument, 0, 's'},
			{"tvaspect", no_argument, 0, 't'},
			{"no-tvaspect", no_argument, 0, 'r'},
			{"unlimitedsprites", no_argument, 0, 'u'},
			{"spritelimit", no_argument, 0, 'q'},
			{"version", no_argument, 0, 'v'},
			{0, 0, 0, 0}
		};
		
		int option_index = 0;
		
		c = getopt_long(argc, argv, "defhl:mnopqrs:tuvw",
			long_options, &option_index);
		
		if (c == -1) { break; }
		
		switch(c) {
			case 'd':
				conf.misc_disable_gui = true;
				break;
			
			case 'e':
				conf.misc_disable_gui = false;
				break;
			
			case 'f':
				conf.video_fullscreen = true;
				break;
			
			case 'w':
				conf.video_fullscreen = false;
				break;
			
			case 'h':
				cli_show_usage();
				exit(0);
				break;
			
			case 'l':
				optint = atoi(optarg);
				if (optint < 6) {
					conf.video_filter = optint;
				}
				else {
					cli_error("Error: Invalid filter");
				}
				break;
			
			case 'm':
				conf.video_unmask_overscan = false;
				break;
			
			case 'n':
				conf.video_unmask_overscan = true;
				break;
			
			case 'o':
				conf.video_stretch_aspect = true;
				break;
			
			case 'p':
				conf.video_stretch_aspect = false;
				break;
			
			case 's':
				optint = atoi(optarg);
				if (optint < 5 && optint != 0) {
					conf.video_scale_factor = optint;
				}
				else {
					cli_error("Error: Invalid scale factor");
				}
				break;
			
			case 't':
				conf.video_tv_aspect = true;
				break;
			
			case 'r':
				conf.video_tv_aspect = false;
				break;
			
			case 'u':
				conf.video_unlimited_sprites = true;
				break;
			
			case 'q':
				conf.video_unlimited_sprites = false;
				break;
			
			case 'v':
				cli_show_version();
				exit(0);
				break;
			
			default:
				cli_error("Error: Invalid option");
				break;
		}
	}
}

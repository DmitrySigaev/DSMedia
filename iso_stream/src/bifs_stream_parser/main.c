/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / command-line mp4 toolbox
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */


#ifndef _WIN32
#include <pwd.h>
#endif

#include <m4_isomedia.h>
#include <m4_render.h>
#include <m4_scenegraph.h>
#include <m4_bifs.h>
#include <intern/avilib.h>
#include <intern/m4_render_dev.h>


#ifdef WIN32
#define GPAC_CFG_FILE "GPAC.cfg"
#else
#define GPAC_CFG_FILE ".gpacrc"
#endif


void PrintVersion()
{
	printf ("MP42AVI - GPAC version %s\n", M4_VERSION);
}

void PrintUsage()
{
	printf ("MP42AVI [option] input\n"
			"Dumps BIFS media frames as AVI, BMP or raw\n\n"
			"Options\n"
			"-fps Framerate: specifies extraction framerate - if not set computed from track length\n"
			"-size WxH: forces output BIFS to the given resolution\n"
			"-raw [frame]: uses raw format for output - only dumps one frame if specified\n"
			"-bmp [frame]: uses BMP format for output - only dumps one frame if specified\n"
			"-outpath path: specifies where to dump frames/movie\n"
			"\n"
			"Note: when dumping a frame, either the frame number can be specified or the frame time\n"
			"in the format hh:mm:ss:xFz where hh, mm, ss are hours, minutes, seconds, x the number\n"
			"of the frame in the seconds and z the frame rate used to express the time\n"
			"\n"
			"-cfg: specifies path to GPAC config file (GPAC.cfg)\n"
			"-v: prints version\n"
			"-h: prints this message\n"
			"\nWritten by Jean Le Feuvre - (c) 2000-2003\n");
}

#ifndef _WIN32
typedef struct tagBITMAPFILEHEADER 
{
    u16	bfType;
    u32	bfSize;
    u16	bfReserved1;
    u16	bfReserved2;
    u32 bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	u32	biSize;
	s32	biWidth;
	s32	biHeight;
	u16	biPlanes;
	u16	biBitCount;
	u32	biCompression;
	u32	biSizeImage;
	s32	biXPelsPerMeter;
	s32	biYPelsPerMeter;
	u32	biClrUsed;
	u32	biClrImportant;
} BITMAPINFOHEADER;

#define BI_RGB        0L

#endif


typedef struct 
{
	LPSCENERENDER sr;
	LPSCENEGRAPH sg;
	LPBIFSDEC bifs;
	M4File *file;

	u32 track;
	u32 duration, cts;
} BIFSVID;

void node_init(void *cbk, SFNode *node)
{
	BIFSVID *b2v = cbk;
	switch (Node_GetTag(node)) {
	case TAG_MPEG4_Conditional:
	case TAG_MPEG4_QuantizationParameter:
		break;
	default:
		SR_NodeInit(b2v->sr, node);
		break;
	}
}

void node_modif(void *cbk, SFNode *node)
{
	BIFSVID *b2v = cbk;
	SR_Invalidate(b2v->sr, node);
}

Double get_scene_time(void *cbk)
{
	Double res;
	BIFSVID *b2v = cbk;
	res = b2v->cts;
	res /= b2v->duration;
	return res;
}

void write_bmp(M4VideoSurface *fb, char *rad_name, u32 img_num)
{
	char str[M4_MAX_PATH];
	BITMAPFILEHEADER fh;
	BITMAPINFOHEADER fi;
	FILE *fout;
	u32 j;
	char *ptr;

	if (img_num<10) {
		sprintf(str, "%s_00%d.bmp", rad_name, img_num);
	} else if (img_num<100) {
		sprintf(str, "%s_0%d.bmp", rad_name, img_num);
	} else {
		sprintf(str, "%s_%d.bmp", rad_name, img_num);
	}

	fout = fopen(str, "wb");
	if (!fout) return;

	memset(&fh, 0, sizeof(fh));
	fh.bfType = 19778;
	fh.bfOffBits = 14 + 40;

	memset(&fi, 0, sizeof(char)*40);
	fi.biSize = sizeof(char)*40;
	fi.biWidth = fb->width;
	fi.biHeight = fb->height;
	fi.biPlanes = 1;
	fi.biBitCount = 24;
	fi.biCompression = BI_RGB;
	fi.biSizeImage = fb->pitch * fb->height;

	/*NOT ALIGNED!!*/
    fwrite(&fh.bfType, 2, 1, fout);
    fwrite(&fh.bfSize, 4, 1, fout);
    fwrite(&fh.bfReserved1, 2, 1, fout);
    fwrite(&fh.bfReserved2, 2, 1, fout);
    fwrite(&fh.bfOffBits, 4, 1, fout);

	fwrite(&fi, 1, 40, fout);

	for (j=fb->height; j>0; j--) {
		ptr = fb->video_buffer + (j-1)*fb->pitch;
		fwrite(ptr, fb->width  * 3, 1, fout);
	}

	fclose(fout);
}


void write_raw(M4VideoSurface *fb, char *rad_name, u32 img_num)
{
	char str[M4_MAX_PATH];
	FILE *fout;
	if (img_num<10) {
		sprintf(str, "%s_00%d.raw", rad_name, img_num);
	} else if (img_num<100) {
		sprintf(str, "%s_0%d.raw", rad_name, img_num);
	} else {
		sprintf(str, "%s_%d.raw", rad_name, img_num);
	}

	fout = fopen(str, "wb");
	if (!fout) return;
	fwrite(fb->video_buffer , fb->height*fb->pitch, 1, fout);
	fclose(fout);
}

void dump_frame(BIFSVID b2v, char *conv_buf, char *out_path, u32 dump_type, avi_t *avi_out, u32 frameNum)
{
	u32 k;
	M4VideoSurface fb;

	/*lock it*/
	SR_GetScreenBuffer(b2v.sr, &fb);
	/*export frame*/
	switch (dump_type) {
	case 0:
		/*reverse frame*/
		for (k=0; k<fb.height; k++) {
			memcpy(conv_buf + k*fb.width*3, fb.video_buffer + (fb.height-k-1) * fb.pitch, sizeof(char) * fb.width  * 3);
		}
		if (AVI_write_frame(avi_out, conv_buf, fb.height*fb.width*3, 1) <0)
			printf("Error writing frame\n");
		break;
	case 2:
		write_raw(&fb, out_path, frameNum);
		break;
	case 1:
		write_bmp(&fb, out_path, frameNum);
		break;
	}
	/*unlock it*/
	SR_ReleaseScreenBuffer(b2v.sr, &fb);
}



LPINIFILE loadconfigfile()
{
	LPINIFILE cfg;
	char szPath[M4_MAX_PATH];

#ifdef WIN32
#ifdef _DEBUG
	strcpy(szPath, "d:\\CVS\\gpac\\bin\\debug\\");
#else
	strcpy(szPath, "d:\\CVS\\gpac\\bin\\release\\");
#endif
	cfg = NewIniFile(szPath, "GPAC.cfg");
	if (cfg) goto success;
	strcpy(szPath, ".");
	cfg = NewIniFile(szPath, "GPAC.cfg");
	if (cfg) goto success;
	strcpy(szPath, "C:\\Program Files\\GPAC");
	cfg = NewIniFile(szPath, "GPAC.cfg");
	if (cfg) goto success;
	strcpy(szPath, ".");
	cfg = NewIniFile(szPath, "GPAC.cfg");
#else
	/*linux*/
	char *cfg_dir = getenv("HOME");
	if (cfg_dir) {
		strcpy(szPath, cfg_dir);
	} else {
		fprintf(stdout, "WARNING: HOME env var not set - using current directory for config file\n");
		strcpy(szPath, ".");
	}
	cfg = NewIniFile(szPath, ".gpacrc");
	if (cfg) goto success;
#endif
	if (!cfg) {
	  fprintf(stdout, "cannot create config file in %s directory\n", szPath);
	  return NULL;
	}
 success:
	fprintf(stdout, "Using config file in %s directory\n", szPath);
	return cfg;
}


void bifs_to_vid(M4File *file, char *szConfigFile, u32 width, u32 height, char *rad_name, u32 dump_type, char *out_dir, Float fps, s32 frameID, s32 dump_time)
{
	M4User client;
	int N;
	BIFSVID b2v;
	u16 es_id;
	Bool first_dump, needs_raw;
	u32 i, j, di, count, timescale, frameNum;
	Float duration, cur_time;
	M4VideoSurface fb;
	M4Err e;
	char old_driv[1024], *test;
	char config_path[M4_MAX_PATH];
	avi_t *avi_out; 
	Bool reset_fps;
	ESDescriptor *esd;
	char comp[5];
	char *conv_buf;

	memset(config_path, 0, M4_MAX_PATH);
	memset(&client, 0, sizeof(M4User));
	if (szConfigFile && strlen(szConfigFile)) {
		client.config = NewIniFile(szConfigFile, GPAC_CFG_FILE);
	} else {
		client.config = loadconfigfile();
	}

	if (!client.config) {
		fprintf(stdout, "Configuration File \"GPAC.cfg\" not found\nPlease enter full path to config file:\n");
		scanf("%s", config_path);
		client.config = NewIniFile(config_path, GPAC_CFG_FILE);
		if (!client.config) {
			fprintf(stdout, "Error: Configuration File \"%s\" not found in %s\n", GPAC_CFG_FILE, config_path);
			return;
		}
	}

	avi_out = NULL;
	conv_buf = NULL;
	needs_raw = 0;
	test = IF_GetKey(client.config, "General", "PluginsDirectory");
	client.plugins = NewPluginManager((const unsigned char *) test, client.config);
	strcpy(old_driv, "raw_out");
	if (!PM_GetPluginsCount(client.plugins)) {
		printf("Error: no plugins found\n");
		goto err_exit;
	}

	/*switch driver to raw_driver*/
	test = IF_GetKey(client.config, "Video", "DriverName");
	if (test) strcpy(old_driv, test);

	test = IF_GetKey(client.config, "Rendering", "RendererName");
	/*since we only support RGB24 for MP42AVI force using RAW out with 2D driver*/
	if (test && strstr(test, "2D")) {
		IF_SetKey(client.config, "Video", "DriverName", "raw_out");
		needs_raw = 1;
	}

	b2v.sr = NewSceneRender(&client, 0, 1, NULL);
	SR_SetOption(b2v.sr, M4O_Visible, 0);

	b2v.sg = NewSceneGraph();
	SG_SetSceneTimeCallback(b2v.sg, get_scene_time, &b2v);
	SG_SetInitCallback(b2v.sg, node_init, &b2v);
	SG_SetModifiedCallback(b2v.sg, node_modif, &b2v);

	/*load config*/
	SR_SetOption(b2v.sr, M4O_ReloadConfig, 1);

	b2v.bifs = BIFS_NewDecoder(b2v.sg, 0);
	BIFS_SetClock(b2v.bifs, get_scene_time, &b2v);

	if (needs_raw) {
		test = IF_GetKey(client.config, "Video", "DriverName");
		if (stricmp(test, "raw_out") && stricmp(test, "Raw Video Output")) {
			printf("couldn't load raw output driver (%s used)\n", test);
			goto err_exit;
		}
	}

	strcpy(config_path, "");
	if (out_dir) {
		strcat(config_path, out_dir);
		if (config_path[strlen(config_path)-1] != '\\') strcat(config_path, "\\");
	}
	strcat(config_path, rad_name);
	strcat(config_path, "_bifs");
	if (!dump_type) {
		strcat(config_path, ".avi");
		avi_out = AVI_open_output_file(config_path);
		comp[0] = comp[1] = comp[2] = comp[3] = comp[4] = 0;
		if (!avi_out) goto err_exit;
	}


	for (i=0; i<M4_GetTrackCount(file); i++) {
		esd = M4_GetStreamDescriptor(file, i+1, 1);
		if (!esd) continue;
		if (!esd->dependsOnESID && (esd->decoderConfig->streamType == M4ST_SCENE)) break;
		OD_DeleteDescriptor((Descriptor **) &esd);
	}
	if (!esd) {
		printf("no bifs track found\n");
		goto err_exit;
	}

	b2v.duration = (u32) M4_GetMediaDuration(file, i+1);
	timescale = M4_GetMediaTimeScale(file, i+1);
	es_id = (u16) M4_GetTrackID(file, i+1);

	e = BIFS_ConfigureStream(b2v.bifs, es_id, esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, esd->decoderConfig->objectTypeIndication);

	if (e) {
		printf("BIFS init error %s\n", M4ErrToString(e));
		OD_DeleteDescriptor((Descriptor **) &esd);
		goto err_exit;
	}
	if (dump_time>=0) dump_time = dump_time *1000 / timescale;

	SR_SetSceneGraph(b2v.sr, b2v.sg);
	count = M4_GetSampleCount(file, i+1);

	reset_fps = 0;
	if (!fps) { 
		fps = (Float) (count * timescale);
		fps /= b2v.duration;
		printf("Estimated BIFS FrameRate %g\n", fps);
		reset_fps = 1;
	}

	if (!width || !height) {
		SG_GetSizeInfo(b2v.sg, &width, &height);
	}
	/*we work in RGB24, and we must make sure the pitch is %4*/
	if ((width*3)%4) {
		printf("Adjusting width (%d) to have a stride multiple of 4\n", width);
		while ((width*3)%4) width--;
	}

	SR_SetSize(b2v.sr, width, height);

	SR_GetScreenBuffer(b2v.sr, &fb);
	width = fb.width;
	height = fb.height;
	if (avi_out) {
		AVI_set_video(avi_out, width, height, fps, comp);
		conv_buf = malloc(sizeof(char) * width * height * 3);
	}
	printf("Dumping at BIFS resolution %d x %d\n\n", width, height);
	SR_ReleaseScreenBuffer(b2v.sr, &fb);

	cur_time = 0;

	duration = 1 / fps;
	if (reset_fps) fps = 0;

	frameNum = 1;
	first_dump = 1;
	N = 0;
	for (j=0; j<count+N; j++) {
		if(j < count)
		{
			M4Sample *samp = M4_GetSample(file, i+1, j+1, &di);

			b2v.cts = samp->DTS + samp->CTS_Offset;
			/*apply command*/
			BIFS_DecodeAU(b2v.bifs, es_id, samp->data, samp->dataLength);
			M4_DeleteSample(&samp);

			if ((frameID>=0) && (j<(u32)frameID)) continue;
			if ((dump_time>=0) && ((u32) dump_time>b2v.cts)) continue;
		}
		/*render frame*/
		SR_RenderFrame(b2v.sr);
		/*needed for background2D !!*/
		if (first_dump) {
			SR_RenderFrame(b2v.sr);
			first_dump = 0;
		}

		if (fps) {
			Float bifs_time = (Float) b2v.cts;
			bifs_time /= timescale;
			if (cur_time > bifs_time) continue;

			while (1) {
				printf("dumped frame time %g (frame %d - sample %d)\r", cur_time, frameNum, j+1);
				dump_frame(b2v, conv_buf, config_path, dump_type, avi_out, frameNum);
				frameNum++;
				cur_time += duration;
				if (cur_time > bifs_time) break;
			}
		} else {
			dump_frame(b2v, conv_buf, config_path, dump_type, avi_out, (frameID>=0) ? frameID : frameNum);
			if (frameID>=0 || dump_time>=0) break;
			frameNum++;
			printf("dumped frame %d / %d\r", j+1, count);
		}

	}
	OD_DeleteDescriptor((Descriptor **) &esd);

	/*destroy everything*/
	BIFS_DeleteDecoder(b2v.bifs);
	SG_Delete(b2v.sg);
	SR_SetSceneGraph(b2v.sr, NULL);
	SR_Delete(b2v.sr);

err_exit:
	if (avi_out) AVI_close(avi_out);
	if (conv_buf) free(conv_buf);
	if (client.plugins) PM_Delete(client.plugins);
	if (needs_raw) IF_SetKey(client.config, "Video", "DriverName", old_driv);
	IF_Delete(client.config);
}

int main (int argc, char **argv)
{
	Float fps_dump;
	u32 i;
	char rad[500];
	s32 frameID, h, m, s, f;
	Float fps;
	u32 dump_type;
	s32 dump_time;
	u32 dump_w, dump_h;
	Bool copy;
	char szConfigFile[4096];
	char *dump_out;
	char *inName, *arg;
	M4File *file;

	if (argc < 2) {
		PrintUsage();
		return 0;
	}

	dump_type = 0;
	fps_dump = 0.0f;
	dump_w = dump_h = 0;
	dump_out = NULL;
	inName = NULL;
	frameID = -1;
	dump_time = -1;
	szConfigFile[0] = 0;

	for (i = 1; i < (u32) argc ; i++) {
		arg = argv[i];
		if (arg[0] != '-') {
			inName = arg;
			break;
		}		
		if (!stricmp(arg, "-h")) {
			PrintUsage();
			return 0;
		} else if (!stricmp(arg, "-version")) {
			PrintVersion();
			return 0;
		} else if (!stricmp(arg, "-size")) {
			sscanf(argv[i+1], "%dx%d", &dump_w, &dump_h);
			i++;
		} else if (!stricmp(arg, "-raw")) {
			dump_type = 2;
			if ((i+1<(u32)argc) && (argv[i+1][0]!='-')) {
				if (strstr(argv[i+1], "T")) {
					if (strstr(argv[i+1], "F")) {
						sscanf(argv[i+1], "T%d:%d:%d:%dF%f", &h, &m, &s, &f, &fps);
						dump_time = (s32) ((3600*h + 60*m + s)*1000 + 1000*f/fps);
					} else {
						sscanf(argv[i+1], "T%d:%d:%d", &h, &m, &s);
						dump_time = (s32) ((3600*h + 60*m + s)*1000);
					}
				} else {
					frameID = atoi(argv[i+1]);
				}
				i++;
			}
		} else if (!stricmp(arg, "-bmp")) {
			dump_type = 1;
			if ((i+1<(u32)argc) && (argv[i+1][0]!='-')) {
				if (strstr(argv[i+1], "T")) {
					if (strstr(argv[i+1], "F")) {
						sscanf(argv[i+1], "T%d:%d:%d:%dF%f", &h, &m, &s, &f, &fps);
						dump_time = (s32) ((3600*h + 60*m + s)*1000 + 1000*f/fps);
					} else {
						sscanf(argv[i+1], "T%d:%d:%d", &h, &m, &s);
						dump_time = (s32) ((3600*h + 60*m + s)*1000);
					}
				} else {
					frameID = atoi(argv[i+1]);
				}
				i++;
			}
		} else if (!stricmp(arg, "-outpath")) {
			dump_out = argv[i+1];
			i++;
		} else if (!stricmp(arg, "-fps")) {
			sscanf(argv[i+1], "%f", &fps_dump);
			i++;
		} else if (!stricmp(arg, "-copy")) {
			copy = 1;
		} else if (!stricmp(arg, "-cfg")) {
			strcpy(szConfigFile, argv[i+1]);
			i += 1;
		} else {
			PrintUsage();
			return (0);
		}
	}
	if (!inName) {
		PrintUsage();
		return 0;
	}
	file = M4_MovieOpen(inName, M4_OPEN_READ);
	if (!file) {
		printf("Error opening file: %s\n", M4ErrToString(M4_GetLastError(NULL)));
		return 0;
	}

	if (dump_out) {
		arg = strrchr(inName, M4_PATH_SEPARATOR);
		if (arg) {
			strcpy(rad, arg + 1);
		} else {
			strcpy(rad, inName);
		}
	} else {
		strcpy(rad, inName);
	}
	while (rad[strlen(rad)-1] != '.') rad[strlen(rad)-1] = 0;
	rad[strlen(rad)-1] = 0;
	bifs_to_vid(file, szConfigFile, dump_w, dump_h, rad, dump_type, dump_out, fps_dump, frameID, dump_time);
	printf("\ndone\n");
	M4_MovieDelete(file);
	return 0;

}


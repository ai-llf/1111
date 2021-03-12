//main_affineTPS.cpp
//affine+TPS transform image based on a pair of given control pointsets
//
// by Lei Qu
//2010-10-27

#include <stdio.h>
#include "getopt.h"

#include "stackutil.h"
#include "basic_surf_objs.h"
#include "q_warp_affine_tps.h"

void printHelp();

//int main(int argc, char *argv[])
int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp();
		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//read arguments
	//input
	QString qs_filename_marker_tar			=NULL;
	QString qs_filename_marker_sub			=NULL;
	QString qs_filename_sub					=NULL;
	long long sz_img_resize[4]				= { 0, 0, 0, 0 };
	//output
	QString qs_filename_sub2tar_affine 		=NULL;
	QString qs_filename_sub2tar_tps 		=NULL;
	//////input
	//QString qs_filename_marker_tar = "D:/Allen/Data/warp/Brats18_CBICA_AQT_1_new.marker";
	//QString qs_filename_marker_sub = "D:/Allen/Data/warp/Brats18_CBICA_AQT_1.marker";
	//QString qs_filename_sub = "D:/Allen/Data/warp/Brats18_CBICA_AQT_1_new.v3draw_processed.v3draw";
	//long long sz_img_resize[4] = { 240, 240, 155, 1 };//xyzc
	////output
	////QString qs_filename_sub2tar_affine = "D:/Allen/Data/warp/affine.v3draw";
	//QString qs_filename_sub2tar_tps = "D:/Allen/Data/warp/Brats18_CBICA_AQT_1_tps.v3draw";

	int c;
	static char optstring[] = "ht:s:S:a:o:f:x:y:z:";
	opterr = 0;
	while ((c = getopt(argc, argv, optstring)) != -1)
	{
		switch (c)
		{
		case 'h':
			printHelp();
			return 0;
			break;
		case 't':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
				return 1;
			}
			qs_filename_marker_tar.append(optarg);
			break;
		case 's':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
				return 1;
			}
			qs_filename_marker_sub.append(optarg);
			break;
		case 'S':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -S.\n");
				return 1;
			}
			qs_filename_sub.append(optarg);
			break;
		case 'a':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -a.\n");
				return 1;
			}
			qs_filename_sub2tar_affine.append(optarg);
			break;
		case 'o':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
				return 1;
			}
			qs_filename_sub2tar_tps.append(optarg);
			break;
		case 'x':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -x.\n");
				return 1;
			}
			sz_img_resize[0] = atoi(optarg);
			break;
		case 'y':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -y.\n");
				return 1;
			}
			sz_img_resize[1] = atoi(optarg);
			break;
		case 'z':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -z.\n");
				return 1;
			}
			sz_img_resize[2] = atoi(optarg);
			break;
		case '?':
			fprintf(stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
			return 1;
			break;
		}
	}

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>Image affine transformation:\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input target  marker:          %s\n", qPrintable(qs_filename_marker_tar));
	printf(">>  input subject marker:          %s\n", qPrintable(qs_filename_marker_sub));
	printf(">>  input subject data:            %s\n", qPrintable(qs_filename_sub));
	printf(">>  output image  size:            [%ld,%ld,%ld]\n", sz_img_resize[0], sz_img_resize[1], sz_img_resize[2]);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output sub2tar_affine data:       %s\n", qPrintable(qs_filename_sub2tar_affine));
	printf(">>  output sub2tar_affine_tps data:   %s\n", qPrintable(qs_filename_sub2tar_tps));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	bool b_saveaffine = 0, b_savetps = 0;
	if (!qs_filename_sub2tar_affine.isEmpty()) 		b_saveaffine = 1;
	if (!qs_filename_sub2tar_tps.isEmpty()) 		b_savetps = 1;

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read target and subject marker files. \n");
	QList<ImageMarker> ql_marker_tar, ql_marker_sub;
	if (qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
	{
		ql_marker_tar = readMarker_file(qs_filename_marker_tar);
		ql_marker_sub = readMarker_file(qs_filename_marker_sub);
		printf("\t>>Target: read %d markers from [%s]\n", ql_marker_tar.size(), qPrintable(qs_filename_marker_tar));
		printf("\t>>Subject:read %d markers from [%s]\n", ql_marker_sub.size(), qPrintable(qs_filename_marker_sub));
	}
	//else
	//{
	//	printf("ERROR: at least one marker file is invalid.\n");
	//	return false;
	//}

	//re-formate to vector
	vector<Coord3D_PCM> vec_tar, vec_sub;
	long l_minlength = min(ql_marker_tar.size(), ql_marker_sub.size());
	for (long i = 0; i < l_minlength; i++)
	{
		vec_tar.push_back(Coord3D_PCM(ql_marker_tar[i].x, ql_marker_tar[i].y, ql_marker_tar[i].z));
		vec_sub.push_back(Coord3D_PCM(ql_marker_sub[i].x, ql_marker_sub[i].y, ql_marker_sub[i].z));
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Read subject image. \n");
	unsigned char *p_img_sub = 0;
	long long *sz_img_sub = 0;
	int datatype_sub = 0;
	if (!loadImage((char *)qPrintable(qs_filename_sub), p_img_sub, sz_img_sub, datatype_sub))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_sub));
		return false;
	}
	printf("\t>>read image file [%s] complete.\n", qPrintable(qs_filename_sub));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]);
	printf("\t\tdatatype: %d\n", datatype_sub);

	//if (datatype_sub != 1)
	//{
	//	printf("ERROR: Input image datatype is not UINT8.\n");
	//	return false;
	//}
	if (sz_img_resize[0] == 0)//do not assigned the output image size (output size = input size)
	{ 
		sz_img_resize[0] = sz_img_sub[0];
		sz_img_resize[1] = sz_img_sub[1];
		sz_img_resize[2] = sz_img_sub[2];
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. Affine or TPS transform the subject image to target. \n");
	unsigned char *p_img_sub2tar_affine = 0, *p_img_sub2tar_tps = 0;
	sz_img_resize[3] = sz_img_sub[3];
	if (b_saveaffine || b_savetps)
	{
		//affine
		if (!q_imagewarp_affine(vec_tar, vec_sub,
			p_img_sub, sz_img_sub, sz_img_resize,
			p_img_sub2tar_affine))
		{
			printf("ERROR: q_imagewarp_affine return false!\n");
			if (p_img_sub) 				{ delete[]p_img_sub;			p_img_sub = 0; }
			if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }
			return false;
		}
		//save image
		saveImage(qPrintable(qs_filename_sub2tar_affine), p_img_sub2tar_affine, sz_img_resize, 1);

		if (b_savetps)
		{
			//affine warp the sub control points
			vector<Coord3D_PCM> vec_sub2tar_affine;
			if (!q_ptswarp_affine(vec_tar, vec_sub, vec_sub2tar_affine))
			{
				printf("ERROR: q_ptswarp_affine() return false!\n");
				if (p_img_sub) 				{ delete[]p_img_sub;			p_img_sub = 0; }
				if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }
				if (p_img_sub2tar_affine) 	{ delete[]p_img_sub2tar_affine;	p_img_sub2tar_affine = 0; }
				return false;
			}

			//affine+tps
			if (!q_imagewarp_tps(vec_tar, vec_sub2tar_affine,
				p_img_sub2tar_affine, sz_img_resize, sz_img_resize,
				p_img_sub2tar_tps))
			{
				printf("ERROR: q_imagewarp_tps() return false!\n");
				if (p_img_sub) 				{ delete[]p_img_sub;			p_img_sub = 0; }
				if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }
				if (p_img_sub2tar_affine) 	{ delete[]p_img_sub2tar_affine;	p_img_sub2tar_affine = 0; }
				return false;
			}
			//save image
			saveImage(qPrintable(qs_filename_sub2tar_tps), p_img_sub2tar_tps, sz_img_resize, 1);
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. free memory. \n");
	if (p_img_sub2tar_tps) 		{ delete[]p_img_sub2tar_tps;	p_img_sub2tar_tps = 0; }
	if (p_img_sub2tar_affine) 	{ delete[]p_img_sub2tar_affine;	p_img_sub2tar_affine = 0; }
	if (p_img_sub) 				{ delete[]p_img_sub;			p_img_sub = 0; }
	if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }

	printf("Program exit success.\n");
	return true;
}

//Printing Help of Usage of this Program
void printHelp()
{
	//pirnt help messages
	printf("\nUsage: main_warp_affine_tps\n");
	printf("Input paras:\n");
	printf("\t  -t   <markerFilename_target>	input target marker file full name.\n");
	printf("\t  -s   <markerFilename_subject>	input subject marker file full name.\n");
	printf("\t  -S   <filename_sub>	            input subject (image) file full name.\n");
	printf("Output paras:\n");
	printf("\t [-a]  <filename_sub2tar_affine>  output sub2tar affine warped (image) file full name.\n");
	printf("\t [-o]  <filename_sub2tar_tps>     output sub2tar tps warped (image) file full name.\n");
	printf("\t [-x]  <output size x>            output image size in x dim.\n");
	printf("\t [-y]  <output size y>            output image size in y dim.\n");
	printf("\t [-z]  <output size z>            output image size in z dim.\n");
	printf("\n");
	printf("\t [-h]	print this message.\n");
	return;
}


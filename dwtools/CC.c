/* CC.c
 * 
 * Copyright (C) 1993-2006 David Weenink
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 djmw 20011016 removed some causes for compiler warnings
 djmw 20020315 GPL header
 djmw 20061212 Changed info to Melder_writeLine<x> format.
 */

#include "CC.h"
#include "NUM2.h"

#include "oo_DESTROY.h"
#include "CC_def.h"
#include "oo_COPY.h"
#include "CC_def.h"
#include "oo_EQUAL.h"
#include "CC_def.h"
#include "oo_WRITE_ASCII.h"
#include "CC_def.h"
#include "oo_WRITE_BINARY.h"
#include "CC_def.h"
#include "oo_READ_ASCII.h"
#include "CC_def.h"
#include "oo_READ_BINARY.h"
#include "CC_def.h"
#include "oo_DESCRIPTION.h"
#include "CC_def.h"

static void info (I)
{
	iam (CC);
	classData -> info (me);
	MelderInfo_writeLine5 ("Time domain:", Melder_double (my xmin), " to ", Melder_double (my xmax),
		" seconds.");
	MelderInfo_writeLine2 ("Number of frames: ", Melder_integer (my nx));
	MelderInfo_writeLine3 ("Time step: ", Melder_double (my dx), " (s).");
	MelderInfo_writeLine3 ("First frame at: ", Melder_double (my x1), " (s).");
	MelderInfo_writeLine2 ("Number of coefficients: ", Melder_integer (my maximumNumberOfCoefficients));
	MelderInfo_writeLine3 ("Minimum frequency: ", Melder_double (my fmin), " Hz.");
	MelderInfo_writeLine3 ("Maximum frequency: ", Melder_double (my fmax), " Hz.");
}

class_methods (CC, Sampled)
	class_method_local (CC, destroy)
	class_method_local (CC, equal)
	class_method_local (CC, copy)
	class_method (info)
	class_method_local (CC, readAscii)
	class_method_local (CC, readBinary)
	class_method_local (CC, writeAscii)
	class_method_local (CC, writeBinary)
	class_method_local (CC, description)
class_methods_end

int CC_Frame_init (CC_Frame me, long numberOfCoefficients)
{
	if ((my c = NUMfvector (1, numberOfCoefficients)) == NULL) return 0;
	my numberOfCoefficients = numberOfCoefficients;
	return 1;
}

int CC_init (I, double tmin, double tmax, long nt, double dt, double t1,
	long maximumNumberOfCoefficients, double fmin, double fmax)
{
	iam (CC);
	my fmin = fmin;
	my fmax = fmax;
	my maximumNumberOfCoefficients = maximumNumberOfCoefficients;
	return Sampled_init (me, tmin, tmax, nt, dt, t1) &&
		(my frame = NUMstructvector (CC_Frame, 1, nt));
}

Matrix CC_to_Matrix (I)
{
	iam (CC);
	Matrix thee;
	long i, j;
	
	if ((thee = Matrix_create (my xmin, my xmax, my nx, my dx, my x1,
		1, my maximumNumberOfCoefficients, my maximumNumberOfCoefficients,
			1, 1)) == NULL) return thee;

	for (i = 1; i <= my nx; i++)
	{
		CC_Frame cf = & my frame[i];
		for (j = 1; j <= cf -> numberOfCoefficients; j++)
		{
			thy z[j][i] = cf -> c[j];
		}
	}
	return thee;
}

void CC_paint (I, Graphics g, double xmin, double xmax, long cmin,
	long cmax, double minimum, double maximum, int garnish)
{
	iam (CC);
	Matrix thee = CC_to_Matrix (me);
	
	if (thee == NULL) return;
	
	Matrix_paintCells (thee, g, xmin, xmax, cmin, cmax, minimum, maximum);
	
	if (garnish)
	{
		Graphics_marksBottom (g, 2, 1, 1, 0);
		Graphics_textBottom (g, 1, "Time (s)");
		Graphics_marksLeft (g, 2, 1, 1, 0);
		Graphics_textLeft (g, 1, "Coefficients");
	}
	
	forget (thee);
}

void CC_drawC0 (I, Graphics g, double xmin, double xmax, double ymin,
	double ymax, int garnish)
{
	iam (CC);
	float *c;
	long i, bframe, eframe, nframes;
	(void) garnish;
	
	if (xmin >= xmax)
	{
		xmin = my xmin; xmax = my xmax;
	}
	
	nframes = Sampled_getWindowSamples (me, xmin, xmax, &bframe, &eframe);
	
	if ((c = NUMfvector (bframe, eframe)) == NULL) return;
	
	for (i = bframe; i <= eframe; i++)
	{
		CC_Frame cf = & my frame[i];
		c[i] = cf -> c0;
	}
	if (ymin >= ymax)
	{
		NUMfvector_extrema (c, bframe, eframe, &ymin, &ymax);
		if (ymax <= ymin) { ymin -= 1.0; ymax += 1.0; }
	}
	else
	{
		NUMfvector_clip (c, bframe, eframe, ymin, ymax);
	}
	Graphics_setInner (g);
	Graphics_setWindow (g, xmin, xmax, ymin, ymax);
		
	Graphics_function (g, c, bframe, eframe, xmin, xmax);
	Graphics_unsetInner (g);
	
	NUMfvector_free (c, bframe);
}

void CC_getNumberOfCoefficients_extrema (I, long startframe, 
	long endframe, long *min, long *max)
{
	iam (CC);
	long i;
	
	Melder_assert (startframe <= endframe);
	
	if (startframe == 0 && endframe == 0)
	{
		startframe = 1; endframe = my nx;
	}
	if (startframe < 1) startframe = 1;
	if (endframe > my nx) endframe = my nx;
	
	*min = my maximumNumberOfCoefficients;
	*max = 0;
	
	for (i = startframe; i <= endframe; i++)
	{
		CC_Frame f = & my frame[i];
		long nc = f -> numberOfCoefficients;
		
		if (nc < *min) *min = nc;
		else if (nc > *max) *max = nc;
	}
}

long CC_getMinimumNumberOfCoefficients (I, long startframe, long endframe)
{
	iam (CC);
	long min, max;
	
	CC_getNumberOfCoefficients_extrema (me, startframe, endframe, &min, &max);

	return min;
}

long CC_getMaximumNumberOfCoefficients (I, long startframe, long endframe)
{
	iam (CC);
	long min, max;
	
	CC_getNumberOfCoefficients_extrema (me, startframe, endframe, &min, &max);

	return max;
}

/* End of file CC.c */

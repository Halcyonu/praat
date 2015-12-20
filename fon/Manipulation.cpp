/* Manipulation.cpp
 *
 * Copyright (C) 1992-2012,2015 Paul Boersma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Manipulation.h"
#include "Sound_to_Pitch.h"
#include "Pitch_to_PitchTier.h"
#include "Pitch_AnyTier_to_PitchTier.h"
#include "PitchTier_to_PointProcess.h"
#include "Pitch_to_PointProcess.h"
#include "PointProcess_and_Sound.h"
#include "Sound_and_LPC.h"

#define MAX_T  0.02000000001   /* Maximum interval between two voice pulses (otherwise voiceless). */

#include "oo_DESTROY.h"
#include "Manipulation_def.h"
#include "oo_COPY.h"
#include "Manipulation_def.h"
#include "oo_EQUAL.h"
#include "Manipulation_def.h"
#include "oo_CAN_WRITE_AS_ENCODING.h"
#include "Manipulation_def.h"
#include "oo_WRITE_TEXT.h"
#include "Manipulation_def.h"
#include "oo_READ_TEXT.h"
#include "Manipulation_def.h"
#include "oo_WRITE_BINARY.h"
#include "Manipulation_def.h"
#include "oo_READ_BINARY.h"
#include "Manipulation_def.h"
#include "oo_DESCRIPTION.h"
#include "Manipulation_def.h"

Thing_implement (Manipulation, Function, 5);

void structManipulation :: v_shiftX (double xfrom, double xto) {
	Manipulation_Parent :: v_shiftX (xfrom, xto);
	if (our sound   )  Function_shiftXTo (our sound.get(),    xfrom, xto);
	if (our pulses  )  Function_shiftXTo (our pulses.get(),   xfrom, xto);
	if (our pitch   )  Function_shiftXTo (our pitch.get(),    xfrom, xto);
	if (our duration)  Function_shiftXTo (our duration.get(), xfrom, xto);
	if (our lpc     )  Function_shiftXTo (our lpc.get(),      xfrom, xto);
}

void structManipulation :: v_scaleX (double xminfrom, double xmaxfrom, double xminto, double xmaxto) {
	Manipulation_Parent :: v_scaleX (xminfrom, xmaxfrom, xminto, xmaxto);
	if (our sound   )  our sound    -> v_scaleX (xminfrom, xmaxfrom, xminto, xmaxto);
	if (our pulses  )  our pulses   -> v_scaleX (xminfrom, xmaxfrom, xminto, xmaxto);
	if (our pitch   )  our pitch    -> v_scaleX (xminfrom, xmaxfrom, xminto, xmaxto);
	if (our duration)  our duration -> v_scaleX (xminfrom, xmaxfrom, xminto, xmaxto);
	if (our lpc     )  our lpc      -> v_scaleX (xminfrom, xmaxfrom, xminto, xmaxto);
}

autoManipulation Manipulation_create (double tmin, double tmax) {
	try {
		autoManipulation me = Thing_new (Manipulation);
		my xmin = tmin, my xmax = tmax;
		my duration = DurationTier_create (tmin, tmax);
		return me;
	} catch (MelderError) {
		Melder_throw (U"Manipulation object not created.");
	}
}

void Manipulation_replaceOriginalSound (Manipulation me, Sound sound) {
	try {
		my sound = Sound_convertToMono (sound);
		Vector_subtractMean (my sound.peek());
		my lpc = autoLPC();
	} catch (MelderError) {
		Melder_throw (me, U": original Sound not replaced with ", sound, U".");
	}
}

void Manipulation_replacePulses (Manipulation me, PointProcess pulses) {
	try {
		my pulses = Data_copy (pulses);
	} catch (MelderError) {
		Melder_throw (me, U": pulses not replaced with ", pulses, U".");
	}
}

void Manipulation_replacePitchTier (Manipulation me, PitchTier pitch) {
	try {
		my pitch = Data_copy (pitch);
	} catch (MelderError) {
		Melder_throw (me, U": pitch tier not replaced with ", pitch, U".");
	}
}

void Manipulation_replaceDurationTier (Manipulation me, DurationTier duration) {
	try {
		my duration = Data_copy (duration);
	} catch (MelderError) {
		Melder_throw (me, U": duration tier not replaced with ", duration, U".");
	}
}

autoManipulation Sound_to_Manipulation (Sound me, double timeStep, double minimumPitch, double maximumPitch) {
	try {
		autoManipulation thee = Manipulation_create (my xmin, my xmax);
		thy sound = Sound_convertToMono (me);
		Vector_subtractMean (thy sound.get());
		autoPitch pitch = Sound_to_Pitch (thy sound.get(), timeStep, minimumPitch, maximumPitch);
		thy pulses = Sound_Pitch_to_PointProcess_cc (thy sound.get(), pitch.peek());
		thy pitch = Pitch_to_PitchTier (pitch.peek());
		/* (DurationTier has been done at creation time) */
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": not converted to Manipulation.");
	}
}

autoManipulation Sound_Pitch_to_Manipulation (Sound sound, Pitch pitch) {
	try {
		autoManipulation me = Manipulation_create (sound -> xmin, sound -> xmax);
		my sound = Sound_convertToMono (sound);
		Vector_subtractMean (my sound.get());
		my pulses = Sound_Pitch_to_PointProcess_cc (my sound.get(), pitch);
		my pitch = Pitch_to_PitchTier (pitch);
		return me;
	} catch (MelderError) {
		Melder_throw (sound, U" & ", pitch, U": not converted to Manipulation.");
	}
}

autoManipulation Sound_PointProcess_to_Manipulation (Sound sound, PointProcess point) {
	try {
		autoManipulation me = Manipulation_create (sound -> xmin, sound -> xmax);
		my sound = Sound_convertToMono (sound);
		Vector_subtractMean (my sound.get());
		my pulses = Data_copy (point);
		my pitch = PointProcess_to_PitchTier (point, MAX_T);
		return me;
	} catch (MelderError) {
		Melder_throw (sound, U" & ", point, U": not converted to Manipulation.");
	}
}

int Manipulation_playPart (Manipulation me, double tmin, double tmax, int method) {
	try {
		if (method == Manipulation_OVERLAPADD) {
			if (! my sound)
				Melder_throw (U"Cannot synthesize overlap-add without a sound.");
			autoSound part = Data_copy (my sound.get());
			long imin = Sampled_xToLowIndex (part.peek(), tmin), imax = Sampled_xToHighIndex (part.peek(), tmax);
			double *amp = part -> z [1];
			for (long i = 1; i <= imin; i ++) amp [i] = 0.0;
			for (long i = imax; i <= part -> nx; i ++) amp [i] = 0.0;
			autoSound saved = my sound.move();
			my sound = part.move();
			try {
				autoSound played = Manipulation_to_Sound (me, Manipulation_OVERLAPADD);
				my sound = saved.move();
				amp = played -> z [1];
				for (imin = 1; imin <= played -> nx; imin ++)
					if (amp [imin] != 0.0) break;
				for (imax = played -> nx; imax >= 1; imax --)
					if (amp [imax] != 0.0) break;
				Sound_playPart (played.peek(), played -> x1 + (imin - 1.5) * played -> dx, played -> x1 + (imax - 0.5) * played -> dx, nullptr, nullptr);
			} catch (MelderError) {
				my sound = saved.move();
				throw;
			}
		} else {
			autoSound sound = Manipulation_to_Sound (me, method);
			Sound_playPart (sound.peek(), tmin, tmax, nullptr, nullptr);
		}
		return 1;
	} catch (MelderError) {
		Melder_throw (me, U": not played.");
	}
}

int Manipulation_play (Manipulation me, int method) {
	try {
		autoSound sound = Manipulation_to_Sound (me, method);
		Sound_play (sound.peek(), nullptr, nullptr);
		return 1;
	} catch (MelderError) {
		Melder_throw (me, U": not played.");
	}
}

static long PointProcess_getFirstVoicedPoint (PointProcess me, double maxT) {
	for (long i = 1; i < my nt; i ++) if (my t [i + 1] - my t [i] <= maxT) return i;
	return 0;
}

static void copyRise (Sound me, double tmin, double tmax, Sound thee, double tmaxTarget) {
	long imin = Sampled_xToHighIndex (me, tmin);
	if (imin < 1) imin = 1;
	long imax = Sampled_xToHighIndex (me, tmax) - 1;   // not xToLowIndex: ensure separation of subsequent calls
	if (imax > my nx) imax = my nx;
	if (imax < imin) return;
	long imaxTarget = Sampled_xToHighIndex (thee, tmaxTarget) - 1;
	long distance = imaxTarget - imax;
	double dphase = NUMpi / (imax - imin + 1);
	for (long i = imin; i <= imax; i ++) {
		long iTarget = i + distance;
		if (iTarget >= 1 && iTarget <= thy nx)
			thy z [1] [iTarget] += my z [1] [i] * 0.5 * (1.0 - cos (dphase * (i - imin + 0.5)));
	}
}

static void copyFall (Sound me, double tmin, double tmax, Sound thee, double tminTarget) {
	long imin = Sampled_xToHighIndex (me, tmin);
	if (imin < 1) imin = 1;
	long imax = Sampled_xToHighIndex (me, tmax) - 1;   // not xToLowIndex: ensure separation of subsequent calls
	if (imax > my nx) imax = my nx;
	if (imax < imin) return;
	long iminTarget = Sampled_xToHighIndex (thee, tminTarget);
	long distance = iminTarget - imin;
	double dphase = NUMpi / (imax - imin + 1);
	for (long i = imin; i <= imax; i ++) {
		long iTarget = i + distance;
		if (iTarget >= 1 && iTarget <= thy nx)
			thy z [1] [iTarget] += my z [1] [i] * 0.5 * (1.0 + cos (dphase * (i - imin + 0.5)));
	}
}

static void copyBell (Sound me, double tmid, double leftWidth, double rightWidth, Sound thee, double tmidTarget) {
	copyRise (me, tmid - leftWidth, tmid, thee, tmidTarget);
	copyFall (me, tmid, tmid + rightWidth, thee, tmidTarget);
}

static void copyBell2 (Sound me, PointProcess source, long isource, double leftWidth, double rightWidth,
	Sound thee, double tmidTarget, double maxT)
{
	/*
	 * Replace 'leftWidth' and 'rightWidth' by the lengths of the intervals in the source (instead of target),
	 * if these are shorter.
	 */
	double tmid = source -> t [isource];
	if (isource > 1 && tmid - source -> t [isource - 1] <= maxT) {
		double sourceLeftWidth = tmid - source -> t [isource - 1];
		if (sourceLeftWidth < leftWidth) leftWidth = sourceLeftWidth;
	}
	if (isource < source -> nt && source -> t [isource + 1] - tmid <= maxT) {
		double sourceRightWidth = source -> t [isource + 1] - tmid;
		if (sourceRightWidth < rightWidth) rightWidth = sourceRightWidth;
	}
	copyBell (me, tmid, leftWidth, rightWidth, thee, tmidTarget);
}

static void copyFlat (Sound me, double tmin, double tmax, Sound thee, double tminTarget) {
	long imin = Sampled_xToHighIndex (me, tmin);
	if (imin < 1) imin = 1;
	long imax = Sampled_xToHighIndex (me, tmax) - 1;   /* Not xToLowIndex: ensure separation of subsequent calls. */
	if (imax > my nx) imax = my nx;
	if (imax < imin) return;
	long iminTarget = Sampled_xToHighIndex (thee, tminTarget);
	if (iminTarget < 1) iminTarget = 1;
	trace (tmin, U" ", tmax, U" ", tminTarget, U" ", imin, U" ", imax, U" ", iminTarget);
	Melder_assert (iminTarget + imax - imin <= thy nx);
	NUMvector_copyElements (my z [1] + imin, thy z [1] + iminTarget, 0, imax - imin);
}

autoSound Sound_Point_Point_to_Sound (Sound me, PointProcess source, PointProcess target, double maxT) {
	try {
		autoSound thee = Sound_create (1, my xmin, my xmax, my nx, my dx, my x1);
		if (source -> nt < 2 || target -> nt < 2) {   /* Almost completely voiceless? */
			NUMvector_copyElements (my z [1], thy z [1], 1, my nx);
			return thee;
		}
		for (long i = 1; i <= target -> nt; i ++) {
			double tmid = target -> t [i];
			double tleft = i > 1 ? target -> t [i - 1] : my xmin;
			double tright = i < target -> nt ? target -> t [i + 1] : my xmax;
			double leftWidth = tmid - tleft, rightWidth = tright - tmid;
			int leftVoiced = i > 1 && leftWidth <= maxT;
			int rightVoiced = i < target -> nt && rightWidth <= maxT;
			long isource = PointProcess_getNearestIndex (source, tmid);
			if (! leftVoiced) leftWidth = rightWidth;   /* Symmetric bell. */
			if (! rightVoiced) rightWidth = leftWidth;   /* Symmetric bell. */
			if (leftVoiced || rightVoiced) {
				copyBell2 (me, source, isource, leftWidth, rightWidth, thee.peek(), tmid, maxT);
				if (! leftVoiced) {
					double startOfFlat = i == 1 ? tleft : (tleft + tmid) / 2;
					double endOfFlat = tmid - leftWidth;
					copyFlat (me, startOfFlat, endOfFlat, thee.peek(), startOfFlat);
					copyFall (me, endOfFlat, tmid, thee.peek(), endOfFlat);
				} else if (! rightVoiced) {
					double startOfFlat = tmid + rightWidth;
					double endOfFlat = i == target -> nt ? tright : (tmid + tright) / 2;
					copyRise (me, tmid, startOfFlat, thee.peek(), startOfFlat);
					copyFlat (me, startOfFlat, endOfFlat, thee.peek(), startOfFlat);
				}
			} else {
				double startOfFlat = i == 1 ? tleft : (tleft + tmid) / 2;
				double endOfFlat = i == target -> nt ? tright : (tmid + tright) / 2;
				copyFlat (me, startOfFlat, endOfFlat, thee.peek(), startOfFlat);
			}
		}
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": not manipulated.");
	}
}

autoSound Sound_Point_Pitch_Duration_to_Sound (Sound me, PointProcess pulses,
	PitchTier pitch, DurationTier duration, double maxT)
{
	try {
		long ipointleft, ipointright;
		double deltat = 0, handledTime = my xmin;
		double startOfSourceNoise, endOfSourceNoise, startOfTargetNoise, endOfTargetNoise;
		double durationOfSourceNoise, durationOfTargetNoise;
		double startOfSourceVoice, endOfSourceVoice, startOfTargetVoice, endOfTargetVoice;
		double durationOfSourceVoice, durationOfTargetVoice;
		double startingPeriod, finishingPeriod, ttarget, voicelessPeriod;
		if (duration -> points.size() == 0)
			Melder_throw (U"No duration points.");

		/*
		 * Create a Sound long enough to hold the longest possible duration-manipulated sound.
		 */
		autoSound thee = Sound_create (1, my xmin, my xmin + 3 * (my xmax - my xmin), 3 * my nx, my dx, my x1);

		/*
		 * Below, I'll abbreviate the voiced interval as "voice" and the voiceless interval as "noise".
		 */
		if (pitch && pitch -> points.size()) for (ipointleft = 1; ipointleft <= pulses -> nt; ipointleft = ipointright + 1) {
			/*
			 * Find the beginning of the voice.
			 */
			startOfSourceVoice = pulses -> t [ipointleft];   /* The first pulse of the voice. */
			startingPeriod = 1.0 / RealTier_getValueAtTime (pitch, startOfSourceVoice);
			startOfSourceVoice -= 0.5 * startingPeriod;   /* The first pulse is in the middle of a period. */

			/*
			 * Measure one noise.
			 */
			startOfSourceNoise = handledTime;
			endOfSourceNoise = startOfSourceVoice;
			durationOfSourceNoise = endOfSourceNoise - startOfSourceNoise;
			startOfTargetNoise = startOfSourceNoise + deltat;
			endOfTargetNoise = startOfTargetNoise + RealTier_getArea (duration, startOfSourceNoise, endOfSourceNoise);
			durationOfTargetNoise = endOfTargetNoise - startOfTargetNoise;

			/*
			 * Copy the noise.
			 */
			voicelessPeriod = NUMrandomUniform (0.008, 0.012);
			ttarget = startOfTargetNoise + 0.5 * voicelessPeriod;
			while (ttarget < endOfTargetNoise) {
				double tsource;
				double tleft = startOfSourceNoise, tright = endOfSourceNoise;
				int i;
				for (i = 1; i <= 15; i ++) {
					double tsourcemid = 0.5 * (tleft + tright);
					double ttargetmid = startOfTargetNoise + RealTier_getArea (duration,
						startOfSourceNoise, tsourcemid);
					if (ttargetmid < ttarget) tleft = tsourcemid; else tright = tsourcemid;
				}
				tsource = 0.5 * (tleft + tright);
				copyBell (me, tsource, voicelessPeriod, voicelessPeriod, thee.peek(), ttarget);
				voicelessPeriod = NUMrandomUniform (0.008, 0.012);
				ttarget += voicelessPeriod;
			}
			deltat += durationOfTargetNoise - durationOfSourceNoise;

			/*
			 * Find the end of the voice.
			 */
			for (ipointright = ipointleft + 1; ipointright <= pulses -> nt; ipointright ++)
				if (pulses -> t [ipointright] - pulses -> t [ipointright - 1] > maxT)
					break;
			ipointright --;
			endOfSourceVoice = pulses -> t [ipointright];   /* The last pulse of the voice. */
			finishingPeriod = 1.0 / RealTier_getValueAtTime (pitch, endOfSourceVoice);
			endOfSourceVoice += 0.5 * finishingPeriod;   /* The last pulse is in the middle of a period. */
			/*
			 * Measure one voice.
			 */
			durationOfSourceVoice = endOfSourceVoice - startOfSourceVoice;

			/*
			 * This will be copied to an interval with a different location and duration.
			 */
			startOfTargetVoice = startOfSourceVoice + deltat;
			endOfTargetVoice = startOfTargetVoice +
				RealTier_getArea (duration, startOfSourceVoice, endOfSourceVoice);
			durationOfTargetVoice = endOfTargetVoice - startOfTargetVoice;

			/*
			 * Copy the voiced part.
			 */
			ttarget = startOfTargetVoice + 0.5 * startingPeriod;
			while (ttarget < endOfTargetVoice) {
				double tsource, period;
				long isourcepulse;
				double tleft = startOfSourceVoice, tright = endOfSourceVoice;
				int i;
				for (i = 1; i <= 15; i ++) {
					double tsourcemid = 0.5 * (tleft + tright);
					double ttargetmid = startOfTargetVoice + RealTier_getArea (duration,
						startOfSourceVoice, tsourcemid);
					if (ttargetmid < ttarget) tleft = tsourcemid; else tright = tsourcemid;
				}
				tsource = 0.5 * (tleft + tright);
				period = 1.0 / RealTier_getValueAtTime (pitch, tsource);
				isourcepulse = PointProcess_getNearestIndex (pulses, tsource);
				copyBell2 (me, pulses, isourcepulse, period, period, thee.peek(), ttarget, maxT);
				ttarget += period;
			}
			deltat += durationOfTargetVoice - durationOfSourceVoice;
			handledTime = endOfSourceVoice;
		}

		/*
		 * Copy the remaining unvoiced part, if we are at the end.
		 */
		startOfSourceNoise = handledTime;
		endOfSourceNoise = my xmax;
		durationOfSourceNoise = endOfSourceNoise - startOfSourceNoise;
		startOfTargetNoise = startOfSourceNoise + deltat;
		endOfTargetNoise = startOfTargetNoise + RealTier_getArea (duration, startOfSourceNoise, endOfSourceNoise);
		durationOfTargetNoise = endOfTargetNoise - startOfTargetNoise;
		voicelessPeriod = NUMrandomUniform (0.008, 0.012);
		ttarget = startOfTargetNoise + 0.5 * voicelessPeriod;
		while (ttarget < endOfTargetNoise) {
			double tsource;
			double tleft = startOfSourceNoise, tright = endOfSourceNoise;
			for (int i = 1; i <= 15; i ++) {
				double tsourcemid = 0.5 * (tleft + tright);
				double ttargetmid = startOfTargetNoise + RealTier_getArea (duration,
					startOfSourceNoise, tsourcemid);
				if (ttargetmid < ttarget) tleft = tsourcemid; else tright = tsourcemid;
			}
			tsource = 0.5 * (tleft + tright);
			copyBell (me, tsource, voicelessPeriod, voicelessPeriod, thee.peek(), ttarget);
			voicelessPeriod = NUMrandomUniform (0.008, 0.012);
			ttarget += voicelessPeriod;
		}

		/*
		 * Find the number of trailing zeroes and hack the sound's time domain.
		 */
		thy xmax = thy xmin + RealTier_getArea (duration, my xmin, my xmax);
		if (fabs (thy xmax - my xmax) < 1e-12) thy xmax = my xmax;   /* Common situation. */
		thy nx = Sampled_xToLowIndex (thee.peek(), thy xmax);
		if (thy nx > 3 * my nx) thy nx = 3 * my nx;

		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": not manipulated.");
	}
}

static autoSound synthesize_overlapAdd_nodur (Manipulation me) {
	try {
		if (! my sound)  Melder_throw (U"Missing original sound.");
		if (! my pulses) Melder_throw (U"Missing pulses analysis.");
		if (! my pitch)  Melder_throw (U"Missing pitch manipulation.");
		autoPointProcess targetPulses = PitchTier_Point_to_PointProcess (my pitch.get(), my pulses.get(), MAX_T);
		return Sound_Point_Point_to_Sound (my sound.get(), my pulses.get(), targetPulses.peek(), MAX_T);
	} catch (MelderError) {
		Melder_throw (me, U": overlap-add synthesis (without duration) not performed.");
	}
}

static autoSound synthesize_overlapAdd (Manipulation me) {
	if (! my duration || my duration -> points.size() == 0) return synthesize_overlapAdd_nodur (me);
	try {
		if (! my sound)  Melder_throw (U"Missing original sound.");
		if (! my pulses) Melder_throw (U"Missing pulses analysis.");
		if (! my pitch)  Melder_throw (U"Missing pitch manipulation.");
		return Sound_Point_Pitch_Duration_to_Sound (my sound.get(), my pulses.get(), my pitch.get(), my duration.get(), MAX_T);
	} catch (MelderError) {
		Melder_throw (me, U": overlap-add synthesis not performed.");
	}
}

static autoSound synthesize_pulses (Manipulation me) {
	try {
		if (! my pulses) Melder_throw (U"Missing pulses analysis.");
		return PointProcess_to_Sound_pulseTrain (my pulses.get(), 44100, 0.7, 0.05, 30);
	} catch (MelderError) {
		Melder_throw (me, U": pulses synthesis not performed.");
	}
}

static autoSound synthesize_pulses_hum (Manipulation me) {
	try {
		if (! my pulses) Melder_throw (U"Missing pulses analysis.");
		return PointProcess_to_Sound_hum (my pulses.get());
	} catch (MelderError) {
		Melder_throw (me, U": pulses hum synthesis not performed.");
	}
}

static autoSound synthesize_pitch (Manipulation me) {
	try {
		if (! my pitch) Melder_throw (U"Missing pitch tier.");
		autoPointProcess pulses = PitchTier_to_PointProcess (my pitch.get());
		return PointProcess_to_Sound_pulseTrain (pulses.peek(), 44100, 0.7, 0.05, 30);
	} catch (MelderError) {
		Melder_throw (me, U": pitch manipulation not synthesized.");
	}
}

static autoSound synthesize_pitch_hum (Manipulation me) {
	try {
		if (! my pitch) Melder_throw (U"Missing pitch tier.");
		autoPointProcess pulses = PitchTier_to_PointProcess (my pitch.get());
		return PointProcess_to_Sound_hum (pulses.peek());
	} catch (MelderError) {
		Melder_throw (me, U": pitch hum manipulation not synthesized.");
	}
}

static autoSound synthesize_pulses_pitch (Manipulation me) {
	try {
		if (! my pulses) Melder_throw (U"Missing pulses analysis.");
		if (! my pitch)  Melder_throw (U"Missing pitch tier.");
		autoPointProcess pulses = PitchTier_Point_to_PointProcess (my pitch.get(), my pulses.get(), MAX_T);
		return PointProcess_to_Sound_pulseTrain (pulses.peek(), 44100, 0.7, 0.05, 30);
	} catch (MelderError) {
		Melder_throw (me, U": pitch pulses manipulation not synthesized.");
	}
}

static autoSound synthesize_pulses_pitch_hum (Manipulation me) {
	try {
		if (! my pulses) Melder_throw (U"Missing pulses analysis.");
		if (! my pitch)  Melder_throw (U"Missing pitch tier.");
		autoPointProcess pulses = PitchTier_Point_to_PointProcess (my pitch.get(), my pulses.get(), MAX_T);
		return PointProcess_to_Sound_hum (pulses.peek());
	} catch (MelderError) {
		Melder_throw (me, U": pitch pulses hum manipulation not synthesized.");
	}
}

/*
void Sound_Formant_Intensity_filter (Sound me, FormantTier formantTier, IntensityTier intensity) {
	Sound_FormantTier_filter_inline (me, formantTier);
	if (intensity) Sound_IntensityTier_multiply_inline (me, intensity);
	NUMdeemphasize_f (my z [1], my nx, my dx, 50.0);
	Vector_scale (me, 0.99);
}

static Sound synthesize_pulses_formant (Manipulation me, int useIntensity) {
	try {
		if (! my pulses)  Melder_throw (U"Missing pulses analysis.");
		if (! my formant) Melder_throw (U"Missing formant information.");
		autoSound thee = PointProcess_to_Sound (my pulses, 44100, 0.7, 0.05, 30);
		Sound_Formant_Intensity_filter (thee.peek(), my formant, useIntensity ? my intensity : nullptr);
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": formant and pulses manipulation not synthesized.");
	}
}
*/

static void Sound_PointProcess_fillVoiceless (Sound me, PointProcess pulses) {
	long ipointleft, ipointright;
	double beginVoiceless = my xmin, endVoiceless;
	for (ipointleft = 1; ipointleft <= pulses -> nt; ipointleft = ipointright + 1) {
		long i1, i2, i;
		endVoiceless = pulses -> t [ipointleft] - 0.005;
		i1 = Sampled_xToHighIndex (me, beginVoiceless);
		if (i1 < 1) i1 = 1; if (i1 > my nx) i1 = my nx;
		i2 = Sampled_xToLowIndex (me, endVoiceless);
		if (i2 < 1) i2 = 1; if (i2 > my nx) i2 = my nx;
		if (i2 - i1 > 10) for (i = i1; i <= i2; i ++)
			my z [1] [i] = NUMrandomGauss (0.0, 0.3);
		for (ipointright = ipointleft + 1; ipointright <= pulses -> nt; ipointright ++)
			if (pulses -> t [ipointright] - pulses -> t [ipointright - 1] > MAX_T)
				break;
		ipointright --;
		beginVoiceless = pulses -> t [ipointright] + 0.005;
	}
	endVoiceless = my xmax;
	{
		long i1, i2, i;
		i1 = Sampled_xToHighIndex (me, beginVoiceless);
		if (i1 < 1) i1 = 1; if (i1 > my nx) i1 = my nx;
		i2 = Sampled_xToLowIndex (me, endVoiceless);
		if (i2 < 1) i2 = 1; if (i2 > my nx) i2 = my nx;
		if (i2 - i1 > 10) for (i = i1; i <= i2; i ++)
			my z [1] [i] = NUMrandomGauss (0.0, 0.3);
	}
}

static autoSound synthesize_pulses_lpc (Manipulation me) {
	try {
		if (! my lpc) {
			if (! my sound) Melder_throw (U"Missing original sound.");
			autoSound sound10k = Sound_resample (my sound.get(), 10000.0, 50);
			my lpc = Sound_to_LPC_burg (sound10k.peek(), 20, 0.025, 0.01, 50.0);
		}
		if (! my pulses) Melder_throw (U"Missing pulses analysis.");
		autoSound train = PointProcess_to_Sound_pulseTrain (my pulses.get(), 1.0 / my lpc -> samplingPeriod, 0.7, 0.05, 30);
		train -> dx = my lpc -> samplingPeriod;   // to be exact
		Sound_PointProcess_fillVoiceless (train.peek(), my pulses.get());
		autoSound result = LPC_and_Sound_filter (my lpc.get(), train.peek(), true);
		NUMdeemphasize_f (result -> z [1], result -> nx, result -> dx, 50.0);
		Vector_scale (result.peek(), 0.99);
		return result;
	} catch (MelderError) {
		Melder_throw (me, U": LPC synthesis not performed.");
	}
}

static autoSound synthesize_pitch_lpc (Manipulation me) {
	try {
		if (! my lpc) {
			if (! my sound) Melder_throw (U"Missing original sound.");
			autoSound sound10k = Sound_resample (my sound.get(), 10000.0, 50);
			my lpc = Sound_to_LPC_burg (sound10k.peek(), 20, 0.025, 0.01, 50.0);
		}
		if (! my pitch)  Melder_throw (U"Missing pitch manipulation.");
		if (! my pulses) Melder_throw (U"Missing pulses analysis.");
		autoPointProcess pulses = PitchTier_Point_to_PointProcess (my pitch.get(), my pulses.get(), MAX_T);
		autoSound train = PointProcess_to_Sound_pulseTrain (pulses.peek(), 1 / my lpc -> samplingPeriod, 0.7, 0.05, 30);
		train -> dx = my lpc -> samplingPeriod;   // to be exact
		Sound_PointProcess_fillVoiceless (train.peek(), my pulses.get());
		autoSound result = LPC_and_Sound_filter (my lpc.get(), train.peek(), true);
		NUMdeemphasize_f (result -> z [1], result -> nx, result -> dx, 50.0);
		Vector_scale (result.peek(), 0.99);
		return result;
	} catch (MelderError) {
		Melder_throw (me, U": pitch LPC synthesis not performed.");
	}
}

autoSound Manipulation_to_Sound (Manipulation me, int method) {
	switch (method) {
		case Manipulation_OVERLAPADD: return synthesize_overlapAdd (me);
		case Manipulation_PULSES: return synthesize_pulses (me);
		case Manipulation_PULSES_HUM: return synthesize_pulses_hum (me);
		case Manipulation_PITCH: return synthesize_pitch (me);
		case Manipulation_PITCH_HUM: return synthesize_pitch_hum (me);
		case Manipulation_PULSES_PITCH: return synthesize_pulses_pitch (me);
		case Manipulation_PULSES_PITCH_HUM: return synthesize_pulses_pitch_hum (me);
		case Manipulation_OVERLAPADD_NODUR: return synthesize_overlapAdd_nodur (me);
		case Manipulation_PULSES_FORMANT: return autoSound();
		case Manipulation_PULSES_FORMANT_INTENSITY: return autoSound();
		case Manipulation_PULSES_LPC: return synthesize_pulses_lpc (me);
		case Manipulation_PULSES_LPC_INTENSITY: return autoSound();
		case Manipulation_PITCH_LPC: return synthesize_pitch_lpc (me);
		case Manipulation_PITCH_LPC_INTENSITY: return autoSound();
		default: return synthesize_overlapAdd (me);
	}
}

autoManipulation Manipulation_AnyTier_to_Manipulation (Manipulation me, AnyTier tier) {
	try {
		if (! my pitch) Melder_throw (U"Missing pitch manipulation.");
		autoManipulation result = Data_copy (me);
		result -> pitch = PitchTier_AnyTier_to_PitchTier (my pitch.get(), tier);
		return result;
	} catch (MelderError) {
		Melder_throw (me, U": not converted.");
	}
}

void Manipulation_writeToTextFileWithoutSound (Manipulation me, MelderFile file) {
	autoSound saved = my sound.move();
	try {
		Data_writeToTextFile (me, file);
		my sound = saved.move();
	} catch (MelderError) {
		my sound = saved.move();
		Melder_throw (me, U": not saved to text file.");
	}
}

void Manipulation_writeToBinaryFileWithoutSound (Manipulation me, MelderFile file) {
	autoSound saved = my sound.move();
	try {
		Data_writeToBinaryFile (me, file);
		my sound = saved.move();
	} catch (MelderError) {
		my sound = saved.move();
		Melder_throw (me, U": not saved to binary file.");
	}
}

/* End of file Manipulation.cpp */

/* melder_tensorio.cpp
 *
 * Copyright (C) 1992-2018 Paul Boersma
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work. If not, see <http://www.gnu.org/licenses/>.
 */

#include "melder.h"

/*** Typed I/O functions for vectors and matrices. ***/

#define FUNCTION(T,storage)  \
	void NUMvector_writeText_##storage (const T *v, integer lo, integer hi, MelderFile file, conststring32 name) { \
		texputintro (file, name, U" []: ", hi >= lo ? nullptr : U"(empty)", 0,0,0); \
		for (integer i = lo; i <= hi; i ++) \
			texput##storage (file, v [i], name, U" [", Melder_integer (i), U"]", 0,0); \
		texexdent (file); \
		if (feof (file -> filePointer) || ferror (file -> filePointer)) Melder_throw (U"Write error."); \
	} \
	void vector_writeText_##storage (const constvector<T>& vec, MelderFile file, conststring32 name) { \
		texputintro (file, name, U" []: ", vec.size >= 1 ? nullptr : U"(empty)", 0,0,0); \
		for (integer i = 1; i <= vec.size; i ++) \
			texput##storage (file, vec [i], name, U" [", Melder_integer (i), U"]", 0,0); \
		texexdent (file); \
		if (feof (file -> filePointer) || ferror (file -> filePointer)) Melder_throw (U"Write error."); \
	} \
	void NUMvector_writeBinary_##storage (const T *v, integer lo, integer hi, FILE *f) { \
		for (integer i = lo; i <= hi; i ++) \
			binput##storage (v [i], f); \
		if (feof (f) || ferror (f)) Melder_throw (U"Write error."); \
	} \
	void vector_writeBinary_##storage (const constvector<T>& vec, FILE *f) { \
		for (integer i = 1; i <= vec.size; i ++) \
			binput##storage (vec [i], f); \
		if (feof (f) || ferror (f)) Melder_throw (U"Write error."); \
	} \
	T * NUMvector_readText_##storage (integer lo, integer hi, MelderReadText text, const char *name) { \
		T *result = nullptr; \
		try { \
			result = NUMvector <T> (lo, hi); \
			for (integer i = lo; i <= hi; i ++) { \
				try { \
					result [i] = texget##storage (text); \
				} catch (MelderError) { \
					Melder_throw (U"Could not read ", Melder_peek8to32 (name), U" [", i, U"]."); \
				} \
			} \
			return result; \
		} catch (MelderError) { \
			NUMvector_free (result, lo); \
			throw; \
		} \
	} \
	autovector<T> vector_readText_##storage (integer size, MelderReadText text, const char *name) { \
		autovector<T> result = newvectorzero<T> (size); \
		for (integer i = 1; i <= size; i ++) { \
			try { \
				result [i] = texget##storage (text); \
			} catch (MelderError) { \
				Melder_throw (U"Could not read ", Melder_peek8to32 (name), U" [", i, U"]."); \
			} \
		} \
		return result; \
	} \
	T * NUMvector_readBinary_##storage (integer lo, integer hi, FILE *f) { \
		T *result = nullptr; \
		try { \
			result = NUMvector <T> (lo, hi); \
			for (integer i = lo; i <= hi; i ++) { \
				result [i] = binget##storage (f); \
			} \
			return result; \
		} catch (MelderError) { \
			NUMvector_free (result, lo); \
			throw; \
		} \
	} \
	autovector<T> vector_readBinary_##storage (integer size, FILE *f) { \
		autovector<T> result = newvectorzero<T> (size); \
		for (integer i = 1; i <= size; i ++) { \
			result [i] = binget##storage (f); \
		} \
		return result; \
	} \
	void NUMmatrix_writeText_##storage (T **m, integer row1, integer row2, integer col1, integer col2, MelderFile file, conststring32 name) { \
		texputintro (file, name, U" [] []: ", row2 >= row1 ? nullptr : U"(empty)", 0,0,0); \
		if (row2 >= row1) { \
			for (integer irow = row1; irow <= row2; irow ++) { \
				texputintro (file, name, U" [", Melder_integer (irow), U"]:", 0,0); \
				for (integer icol = col1; icol <= col2; icol ++) { \
					texput##storage (file, m [irow] [icol], name, U" [", Melder_integer (irow), U"] [", Melder_integer (icol), U"]"); \
				} \
				texexdent (file); \
			} \
		} \
		texexdent (file); \
		if (feof (file -> filePointer) || ferror (file -> filePointer)) Melder_throw (U"Write error."); \
	} \
	void matrix_writeText_##storage (const constmatrix<T>& mat, MelderFile file, conststring32 name) { \
		texputintro (file, name, U" [] []: ", mat.nrow >= 1 ? nullptr : U"(empty)", 0,0,0); \
		for (integer irow = 1; irow <= mat.nrow; irow ++) { \
			texputintro (file, name, U" [", Melder_integer (irow), U"]:", 0,0); \
			for (integer icol = 1; icol <= mat.ncol; icol ++) { \
				texput##storage (file, mat [irow] [icol], name, U" [", Melder_integer (irow), U"] [", Melder_integer (icol), U"]"); \
			} \
			texexdent (file); \
		} \
		texexdent (file); \
		if (feof (file -> filePointer) || ferror (file -> filePointer)) Melder_throw (U"Write error."); \
	} \
	void NUMmatrix_writeBinary_##storage (T **m, integer row1, integer row2, integer col1, integer col2, FILE *f) { \
		if (row2 >= row1) { \
			for (integer irow = row1; irow <= row2; irow ++) { \
				for (integer icol = col1; icol <= col2; icol ++) \
					binput##storage (m [irow] [icol], f); \
			} \
		} \
		if (feof (f) || ferror (f)) Melder_throw (U"Write error."); \
	} \
	void matrix_writeBinary_##storage (const constmatrix<T>& mat, FILE *f) { \
		for (integer irow = 1; irow <= mat.nrow; irow ++) { \
			for (integer icol = 1; icol <= mat.ncol; icol ++) \
				binput##storage (mat [irow] [icol], f); \
		} \
		if (feof (f) || ferror (f)) Melder_throw (U"Write error."); \
	} \
	T ** NUMmatrix_readText_##storage (integer row1, integer row2, integer col1, integer col2, MelderReadText text, const char *name) { \
		T **result = nullptr; \
		try { \
			result = NUMmatrix <T> (row1, row2, col1, col2); \
			for (integer irow = row1; irow <= row2; irow ++) for (integer icol = col1; icol <= col2; icol ++) { \
				try { \
					result [irow] [icol] = texget##storage (text); \
				} catch (MelderError) { \
					Melder_throw (U"Could not read ", Melder_peek8to32 (name), U" [", irow, U"] [", icol, U"]."); \
				} \
			} \
			return result; \
		} catch (MelderError) { \
			NUMmatrix_free (result, row1, col1); \
			throw; \
		} \
	} \
	automatrix<T> matrix_readText_##storage (integer nrow, integer ncol, MelderReadText text, const char *name) { \
		automatrix<T> result = newmatrixzero<T> (nrow, ncol); \
		for (integer irow = 1; irow <= nrow; irow ++) for (integer icol = 1; icol <= ncol; icol ++) { \
			try { \
				result [irow] [icol] = texget##storage (text); \
			} catch (MelderError) { \
				Melder_throw (U"Could not read ", Melder_peek8to32 (name), U" [", irow, U"] [", icol, U"]."); \
			} \
		} \
		return result; \
	} \
	T ** NUMmatrix_readBinary_##storage (integer row1, integer row2, integer col1, integer col2, FILE *f) { \
		T **result = nullptr; \
		try { \
			result = NUMmatrix <T> (row1, row2, col1, col2); \
			for (integer irow = row1; irow <= row2; irow ++) for (integer icol = col1; icol <= col2; icol ++) \
				result [irow] [icol] = binget##storage (f); \
			return result; \
		} catch (MelderError) { \
			NUMmatrix_free (result, row1, col1); \
			throw; \
		} \
	} \
	automatrix<T> matrix_readBinary_##storage (integer nrow, integer ncol, FILE *f) { \
		automatrix<T> result = newmatrixzero<T> (nrow, ncol); \
		for (integer irow = 1; irow <= nrow; irow ++) for (integer icol = 1; icol <= ncol; icol ++) \
			result [irow] [icol] = binget##storage (f); \
		return result; \
	}

FUNCTION (signed char, i8)
FUNCTION (int, i16)
FUNCTION (long, i32)
FUNCTION (integer, integer32BE)
FUNCTION (integer, integer16BE)
FUNCTION (unsigned char, u8)
FUNCTION (unsigned int, u16)
FUNCTION (unsigned long, u32)
FUNCTION (double, r32)
FUNCTION (double, r64)
FUNCTION (dcomplex, c64)
FUNCTION (dcomplex, c128)
#undef FUNCTION

/* End of file melder_tensorio.cpp */

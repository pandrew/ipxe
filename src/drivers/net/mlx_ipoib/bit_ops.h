/*
  This software is available to you under a choice of one of two
  licenses.  You may choose to be licensed under the terms of the GNU
  General Public License (GPL) Version 2, available at
  <http://www.fsf.org/copyleft/gpl.html>, or the OpenIB.org BSD
  license, available in the LICENSE.TXT file accompanying this
  software.  These details are also available at
  <http://openib.org/license.html>.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  Copyright (c) 2004 Mellanox Technologies Ltd.  All rights reserved.
*/

#ifndef __bit_ops_h__
#define __bit_ops_h__

typedef unsigned long MT_offset_t;
typedef unsigned long MT_size_t;
typedef unsigned char pseudo_bit_t;
struct addr_64_st {
	__u32 addr_l;
	__u32 addr_h;
};

#define MT_BIT_OFFSET(object_struct,reg_path) \
    ((MT_offset_t) &( ((struct object_struct *)(0))-> reg_path ))

#define MT_BIT_SIZE(object_struct,reg_path) \
    ((MT_size_t) sizeof( ((struct object_struct *)(0))-> reg_path ))

#define MT_BIT_OFFSET_SIZE(object_struct,reg_path) \
    MT_BIT_OFFSET(object_struct,reg_path),MT_BIT_SIZE(object_struct,reg_path)

#define MT_BYTE_OFFSET(object_struct,reg_path) \
    ((MT_offset_t) (MT_BIT_OFFSET(object_struct,reg_path)/8))

#define MT_BYTE_SIZE(object_struct,reg_path) \
    ((MT_size_t) MT_BIT_SIZE(object_struct,reg_path)/8)

#define MT_BYTE_OFFSET_SIZE(object_struct,reg_path) \
    MT_BYTE_OFFSET(object_struct,reg_path),MT_BYTE_SIZE(object_struct,reg_path)

#define MT_STRUCT_SIZE(object_struct) (sizeof(struct object_struct) >> 3)

/*****************************************************************************************
 * Bit manipulation macros
 *****************************************************************************************/

/* MASK generate a bit mask S bits width */
#define MASK32(S)         ( ((__u32) ~0L) >> (32-(S)) )

/*
 * BITS generate a bit mask with bits O+S..O set (assumes 32 bit integer).
 *      numbering bits as following:    31........................76543210
 */
#define BITS32(O,S)       ( MASK32(S) << (O) )

/* 
 * MT_EXTRACT32 macro extracts S bits from (__u32)W with offset O 
 *  and shifts them O places to the right (right justifies the field extracted).
 */
#define MT_EXTRACT32(W,O,S)  ( ((W)>>(O)) & MASK32(S) )

/*
 * MT_INSERT32 macro inserts S bits with offset O from field F into word W (__u32)
 */
#define MT_INSERT32(W,F,O,S) ((W)= ( ( (W) & (~BITS32(O,S)) ) | (((F) & MASK32(S))<<(O)) ))

/*
 * MT_EXTRACT_ARRAY32 macro is similar to EXTRACT but works on an array of (__u32),
 * thus offset may be larger than 32 (but not size).
 */
#define MT_EXTRACT_ARRAY32(A,O,S) MT_EXTRACT32(((__u32*)A)[O >> 5],(O & MASK32(5)),S)

/*
 * MT_EXTRACT_ARRAY32_BE macro is similar to EXTRACT but works on an array of (__u32),
 * thus offset may be larger than 32 (but not size).
 *
 * (added by mcb30)
 */
#define MT_EXTRACT_ARRAY32_BE(A,O,S) MT_EXTRACT32(be32_to_cpu(((__u32*)A)[O >> 5]),(O & MASK32(5)),S)

/*
 * MT_INSERT_ARRAY32 macro is similar to INSERT but works on an array of (__u32),
 * thus offset may be larger than 32 (but not size).
 */
#define MT_INSERT_ARRAY32(A,F,O,S) MT_INSERT32(((__u32*)A)[O >> 5],F,(O & MASK32(5)),S)

#define INS_FLD(src, a, st, fld) MT_INSERT_ARRAY32(a, src, MT_BIT_OFFSET(st, fld), MT_BIT_SIZE(st, fld))

#define EX_FLD(a, st, fld) MT_EXTRACT_ARRAY32(a, MT_BIT_OFFSET(st, fld), MT_BIT_SIZE(st, fld))

#define EX_FLD_BE(a, st, fld) MT_EXTRACT_ARRAY32_BE(a, MT_BIT_OFFSET(st, fld), MT_BIT_SIZE(st, fld))

/* return the address of the dword holding the field 

	buf = pointer to buffer where to place the value
	st = struct describing the buffer
	fld = field in the struct where to insert the value */

#define FLD_DW_ADDR(buf, st, fld) ((__u32 *)((__u32 *)(buf)+(((__u32)(&(((struct st *)(0))->fld))) >> 5)))

/*
	val = value to insert
	buf = pointer to buffer where to place the value
	st = struct describing the buffer
	fld = field in the struct where to insert the value */

#define INS_FLD_TO_BE(val, buf, st, fld) \
	do { \
		*FLD_DW_ADDR(buf, st, fld) = be32_to_cpu(*FLD_DW_ADDR(buf, st, fld)); \
		INS_FLD(val, buf, st, fld); \
		*FLD_DW_ADDR(buf, st, fld) = cpu_to_be32(*FLD_DW_ADDR(buf, st, fld)); \
	} \
	while(0)

#define EX_FLD_FROM_BE(buf, st, fld, type) \
({ \
	type field; \
				 \
	*FLD_DW_ADDR(buf, st, fld) = be32_to_cpu(*FLD_DW_ADDR(buf, st, fld)); \
	field= EX_FLD(buf, st, fld); \
	*FLD_DW_ADDR(buf, st, fld) = cpu_to_be32(*FLD_DW_ADDR(buf, st, fld)); \
																		  \
	field; \
})



/* Remaining code Copyright Fen Systems Ltd. 2007 */

/**
 * Wrapper structure for pseudo_bit_t structures
 *
 * This structure provides a wrapper around the autogenerated
 * pseudo_bit_t structures.  It has the correct size, and also
 * encapsulates type information about the underlying pseudo_bit_t
 * structure, which allows the MLX_FILL etc. macros to work without
 * requiring explicit type information.
 */
#define MLX_DECLARE_STRUCT( _structure )				     \
	_structure {							     \
	    union {							     \
		uint8_t bytes[ sizeof ( struct _structure ## _st ) / 8 ];    \
		uint32_t dwords[ sizeof ( struct _structure ## _st ) / 32 ]; \
		struct _structure ## _st *dummy[0];			     \
	    } u;							     \
	}

/** Get pseudo_bit_t structure type from wrapper structure pointer */
#define MLX_PSEUDO_STRUCT( _ptr )					     \
	typeof ( *((_ptr)->u.dummy[0]) )

/** Bit offset of a field within a pseudo_bit_t structure */
#define MLX_BIT_OFFSET( _structure_st, _field )				     \
	offsetof ( _structure_st, _field )

/** Dword offset of a field within a pseudo_bit_t structure */
#define MLX_DWORD_OFFSET( _structure_st, _field )			     \
	( MLX_BIT_OFFSET ( _structure_st, _field ) / 32 )

/** Dword bit offset of a field within a pseudo_bit_t structure
 *
 * Yes, using mod-32 would work, but would lose the check for the
 * error of specifying a mismatched field name and dword index.
 */
#define MLX_DWORD_BIT_OFFSET( _structure_st, _index, _field )		     \
	( MLX_BIT_OFFSET ( _structure_st, _field ) - ( 32 * (_index) ) )

/** Bit width of a field within a pseudo_bit_t structure */
#define MLX_BIT_WIDTH( _structure_st, _field )				     \
	sizeof ( ( ( _structure_st * ) NULL )->_field )

/** Bit mask for a field within a pseudo_bit_t structure */
#define MLX_BIT_MASK( _structure_st, _field )				     \
	( ( ~( ( uint32_t ) 0 ) ) >>					     \
	  ( 32 - MLX_BIT_WIDTH ( _structure_st, _field ) ) )

/*
 * Assemble native-endian dword from named fields and values
 *
 */

#define MLX_ASSEMBLE_1( _structure_st, _index, _field, _value )		     \
	( (_value) << MLX_DWORD_BIT_OFFSET ( _structure_st, _index, _field ) )

#define MLX_ASSEMBLE_2( _structure_st, _index, _field, _value, ... )	     \
	( MLX_ASSEMBLE_1 ( _structure_st, _index, _field, _value ) |	     \
	  MLX_ASSEMBLE_1 ( _structure_st, _index, __VA_ARGS__ ) )

#define MLX_ASSEMBLE_3( _structure_st, _index, _field, _value, ... )	     \
	( MLX_ASSEMBLE_1 ( _structure_st, _index, _field, _value ) |	     \
	  MLX_ASSEMBLE_2 ( _structure_st, _index, __VA_ARGS__ ) )

#define MLX_ASSEMBLE_4( _structure_st, _index, _field, _value, ... )	     \
	( MLX_ASSEMBLE_1 ( _structure_st, _index, _field, _value ) |	     \
	  MLX_ASSEMBLE_3 ( _structure_st, _index, __VA_ARGS__ ) )

#define MLX_ASSEMBLE_5( _structure_st, _index, _field, _value, ... )	     \
	( MLX_ASSEMBLE_1 ( _structure_st, _index, _field, _value ) |	     \
	  MLX_ASSEMBLE_4 ( _structure_st, _index, __VA_ARGS__ ) )

#define MLX_ASSEMBLE_6( _structure_st, _index, _field, _value, ... )	     \
	( MLX_ASSEMBLE_1 ( _structure_st, _index, _field, _value ) |	     \
	  MLX_ASSEMBLE_5 ( _structure_st, _index, __VA_ARGS__ ) )

/*
 * Build native-endian (positive) dword bitmasks from named fields
 *
 */

#define MLX_MASK_1( _structure_st, _index, _field )			     \
	( MLX_BIT_MASK ( _structure_st, _field ) <<			     \
	  MLX_DWORD_BIT_OFFSET ( _structure_st, _index, _field ) )

#define MLX_MASK_2( _structure_st, _index, _field, ... )		     \
	( MLX_MASK_1 ( _structure_st, _index, _field ) |		     \
	  MLX_MASK_1 ( _structure_st, _index, __VA_ARGS__ ) )

#define MLX_MASK_3( _structure_st, _index, _field, ... )		     \
	( MLX_MASK_1 ( _structure_st, _index, _field ) |		     \
	  MLX_MASK_2 ( _structure_st, _index, __VA_ARGS__ ) )

#define MLX_MASK_4( _structure_st, _index, _field, ... )		     \
	( MLX_MASK_1 ( _structure_st, _index, _field ) |		     \
	  MLX_MASK_3 ( _structure_st, _index, __VA_ARGS__ ) )

#define MLX_MASK_5( _structure_st, _index, _field, ... )		     \
	( MLX_MASK_1 ( _structure_st, _index, _field ) |		     \
	  MLX_MASK_4 ( _structure_st, _index, __VA_ARGS__ ) )

#define MLX_MASK_6( _structure_st, _index, _field, ... )		     \
	( MLX_MASK_1 ( _structure_st, _index, _field ) |		     \
	  MLX_MASK_5 ( _structure_st, _index, __VA_ARGS__ ) )

/*
 * Populate big-endian dwords from named fields and values
 *
 */

#define MLX_FILL( _ptr, _index, _assembled )				     \
	do {								     \
		uint32_t *__ptr = &(_ptr)->u.dwords[(_index)];		     \
		uint32_t __assembled = (_assembled);			     \
		*__ptr = cpu_to_be32 ( __assembled );			     \
	} while ( 0 )

#define MLX_FILL_1( _ptr, _index, ... )					     \
	MLX_FILL ( _ptr, _index, MLX_ASSEMBLE_1 ( MLX_PSEUDO_STRUCT ( _ptr ),\
						  _index, __VA_ARGS__ ) )

#define MLX_FILL_2( _ptr, _index, ... )					     \
	MLX_FILL ( _ptr, _index, MLX_ASSEMBLE_2 ( MLX_PSEUDO_STRUCT ( _ptr ),\
						  _index, __VA_ARGS__ ) )

#define MLX_FILL_3( _ptr, _index, ... )					     \
	MLX_FILL ( _ptr, _index, MLX_ASSEMBLE_3 ( MLX_PSEUDO_STRUCT ( _ptr ),\
						  _index, __VA_ARGS__ ) )

#define MLX_FILL_4( _ptr, _index, ... )					     \
	MLX_FILL ( _ptr, _index, MLX_ASSEMBLE_4 ( MLX_PSEUDO_STRUCT ( _ptr ),\
						  _index, __VA_ARGS__ ) )

#define MLX_FILL_5( _ptr, _index, ... )					     \
	MLX_FILL ( _ptr, _index, MLX_ASSEMBLE_5 ( MLX_PSEUDO_STRUCT ( _ptr ),\
						  _index, __VA_ARGS__ ) )

#define MLX_FILL_6( _ptr, _index, ... )					     \
	MLX_FILL ( _ptr, _index, MLX_ASSEMBLE_6 ( MLX_PSEUDO_STRUCT ( _ptr ),\
						  _index, __VA_ARGS__ ) )

/*
 * Modify big-endian dword using named field and value
 *
 */

#define MLX_SET( _ptr, _field, _value )					     \
	do {								     \
		unsigned int __index = 					     \
		    MLX_DWORD_OFFSET ( MLX_PSEUDO_STRUCT ( _ptr ), _field ); \
		uint32_t *__ptr = &(_ptr)->u.dwords[__index];		     \
		uint32_t __value = be32_to_cpu ( *__ptr );		     \
		__value &= ~( MLX_MASK_1 ( MLX_PSEUDO_STRUCT ( _ptr ),	     \
					   __index, _field ) );		     \
		__value |= MLX_ASSEMBLE_1 ( MLX_PSEUDO_STRUCT ( _ptr ),	     \
					    __index, _field, _value );	     \
		*__ptr = cpu_to_be32 ( __value );			     \
	} while ( 0 )

/*
 * Extract value of named field
 *
 */

#define MLX_GET( _ptr, _field )						     \
	( {								     \
		unsigned int __index = 					     \
		    MLX_DWORD_OFFSET ( MLX_PSEUDO_STRUCT ( _ptr ), _field ); \
		uint32_t *__ptr = &(_ptr)->u.dwords[__index];		     \
		uint32_t __value = be32_to_cpu ( *__ptr );		     \
		__value >>=						     \
		    MLX_DWORD_BIT_OFFSET ( MLX_PSEUDO_STRUCT ( _ptr ),	     \
					    __index, _field );		     \
		__value &=						     \
		    MLX_BIT_MASK ( MLX_PSEUDO_STRUCT ( _ptr ), _field );     \
		__value;						     \
	} )

#endif				/* __bit_ops_h__ */

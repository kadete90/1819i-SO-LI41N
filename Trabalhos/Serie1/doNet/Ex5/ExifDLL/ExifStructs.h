#ifndef __EXIF_H
#define __EXIF_H

/*#pragma pack instructs the compiler to pack structure members with particular alignment.
Most compilers, when you declare a struct, will insert padding between members to ensure that they are aligned to appropriate addresses in memory(usually a multiple of the type's size). 
This avoids the performance penalty (or outright error) on some architectures associated with accessing variables that are not aligned properly. */

#pragma pack(push, 1)
typedef struct
{
	unsigned long num;
	unsigned long denom;

} URATIONAL;

typedef struct
{
	long num;
	long denom;

} RATIONAL;

typedef struct
{
	unsigned short tag;
	unsigned short data_format;
	unsigned int number_components;
	unsigned int data;

} ENTRY_STRUCT;

typedef struct
{
	unsigned short number_directory_entry;
	ENTRY_STRUCT directory[1];

} IFD_STRUCT;

typedef struct
{
	char byte_align[2];
	short tag_mark;
	int Exif_IFD_offset;
	IFD_STRUCT IFD;
	int GPS_IFD_offset;
	IFD_STRUCT GPS;

} TIFF_STRUCT;

typedef struct
{
	unsigned short data_size;
	char exifTags[4];
	unsigned short padding;
	TIFF_STRUCT TIFF_header;

} EXIF_STRUCT;

typedef struct
{
	double  latitude;
	double  longitude;
	double  altitude;
	char north_south; // ASCII
	char east_west; // ASCII

} GPS_IFD_STRUCT;
#pragma pack(pop)

// used to index the nº of bytes per component to the data_format value
const short DATA_FORMATS[12] = {
	1, 1, 2, 4, 8, 1,
	1, 2, 4, 8, 4, 8
};

const short FORMAT_U_BYTE = 1;
const short FORMAT_ASCII_STR = 2;
const short FORMAT_U_SHORT = 3;
const short FORMAT_U_LONG = 4;
const short FORMAT_U_RATIONAL = 5;
const short FORMAT_BYTE = 6;
const short FORMAT_UNDEFINED = 7;
const short FORMAT_SHORT = 8;
const short FORMAT_LONG = 9;
const short FORMAT_RATIONAL = 10;
const short FORMAT_FLOAT = 11;
const short FORMAT_DOUBLE = 12;

const short GPS_NORTH_SOUTH_LAT_TAG = 0x1;
const short GPS_LATITUDE_TAG = 0x2;
const short GPS_EAST_WEST_LONG_ID = 0x3;
const short GPS_LONGITUDE_TAG = 0x4;
const short GPS_ALTITUDE_TAG = 0x6;

const unsigned short MODEL_TAG = 0x0110;
const unsigned short WIDTH_TAG = 0xa002;
const unsigned short HEIGH_TAG = 0xa003;
const unsigned short DATA_TAG = 0x0132;
const unsigned short ISO_TAG = 0x8827;
const unsigned short SHUTTER_TAG = 0x9201;
const unsigned short APERTURE_TAG = 0x9202;

const unsigned short EXIF_OFFSET_TAG = 0x8769;
const unsigned short GPS_INFO_TAG = 0x8825;
#endif
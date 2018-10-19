#include "stdafx.h"
#include "ExifStructs.h"
#include <string>
#include <set>

std::set<unsigned short> PRINT_TAGS =
{
	MODEL_TAG, WIDTH_TAG, HEIGH_TAG, DATA_TAG, ISO_TAG, SHUTTER_TAG, APERTURE_TAG, GPS_INFO_TAG
};

void _print(char* tiff_h, IFD_STRUCT* ifd, int i, bool ascii_char = false)
{
	switch (ifd->directory[i].data_format)
	{
	case FORMAT_U_RATIONAL:
	{
		URATIONAL* uRational = reinterpret_cast<URATIONAL*>(tiff_h) + ifd->directory[i].data;
		printf("0x%04x\t- %u / %u\n", ifd->directory[i].tag, uRational->num, uRational->denom);
		break;
	}
	case FORMAT_RATIONAL:
	{
		RATIONAL* rational = reinterpret_cast<RATIONAL*>(tiff_h) + ifd->directory[i].data;
		printf("0x%04x\t- %d / %d\n", ifd->directory[i].tag, rational->num, rational->denom);
		break;
	}
	default:
	{
		const short size_of_data = DATA_FORMATS[ifd->directory[i].data_format - 1] * ifd->directory[i].number_components;

		if (ascii_char)
		{
			char _char = static_cast<char>(ifd->directory[i].data);
			printf("0x%04x\t- %c\n", ifd->directory[i].tag, _char);
			break;
		}

		char * data = size_of_data > 4
			? tiff_h + ifd->directory[i].data
			: reinterpret_cast<char*>(ifd->directory[i].data);

		switch (ifd->directory[i].data_format)
		{
		case FORMAT_U_BYTE:
		case FORMAT_U_SHORT:
		case FORMAT_U_LONG:
		case FORMAT_UNDEFINED:
		{
			printf("0x%04x\t- %u\n", ifd->directory[i].tag, reinterpret_cast<unsigned>(data));
			break;
		}
		case FORMAT_BYTE:
		case FORMAT_SHORT:
		case FORMAT_LONG:
		{
			printf("0x%04x\t- %u\n", ifd->directory[i].tag, reinterpret_cast<signed>(data));
			break;
		}
		case FORMAT_FLOAT:
		{
			printf("0x%04x\t- %d\n", ifd->directory[i].tag, std::stoi(data));
			break;
		}
		case FORMAT_DOUBLE:
		{
			printf("0x%04x\t- %f\n", ifd->directory[i].tag, std::stod(data));
			break;
		}
		case FORMAT_ASCII_STR:
		default:
		{
			printf("0x%04x\t- %s\n", ifd->directory[i].tag, data);
			break;
		}
		}
		break;
	}
	}
}

void _helper_PrintGPSStruct(char* tiff_h, IFD_STRUCT* ifd)
{
	for (int i = 0; i < ifd->number_directory_entry; i++)
	{
		switch (ifd->directory[i].tag)
		{
			case GPS_NORTH_SOUTH_LAT_TAG:
			{
				_print(tiff_h, ifd, i, true);
				break;
			}
			case GPS_LATITUDE_TAG:
			{
				_print(tiff_h, ifd, i);
				break;
			}
			case GPS_EAST_WEST_LONG_ID:
			{
				_print(tiff_h, ifd, i, true);
				break;
			}
			case GPS_LONGITUDE_TAG:
			{
				_print(tiff_h, ifd, i);
				break;
			}
			case GPS_ALTITUDE_TAG:
			{
				_print(tiff_h, ifd, i);
				break;
			}
			default:
				break;
		}
	}
}

void _helper_PrintIFDStruct(char* tiff_h, IFD_STRUCT* ifd)
{
	for (int i = 0; i < ifd->number_directory_entry; i++)
	{
		if (ifd->directory[i].tag == EXIF_OFFSET_TAG)
		{
			_helper_PrintIFDStruct(tiff_h, reinterpret_cast<IFD_STRUCT*>(tiff_h + ifd->directory[i].data));
			continue;
		}

		if (ifd->directory[i].tag == GPS_INFO_TAG)
		{
			_helper_PrintGPSStruct(tiff_h, reinterpret_cast<IFD_STRUCT*>(tiff_h + ifd->directory[i].data));
			continue;
		}

		if (PRINT_TAGS.find(ifd->directory[i].tag) == PRINT_TAGS.end())
		{
			continue;
		}

		_print(tiff_h, ifd, i);
	}
}
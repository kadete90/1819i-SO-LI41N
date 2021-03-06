// ExifDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ExifStructs.h"
#include <string>
#include "ExifDll.h"

EXIF_STRUCT* _MapFileInfoFromPath(LPCTSTR path)
{
	const HANDLE h_file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (h_file == INVALID_HANDLE_VALUE) {
		printf("Error opening file (%d)\n", GetLastError());
		return nullptr;
	}

	const HANDLE f_map = CreateFileMapping(h_file, nullptr, PAGE_READONLY, 0, 0, nullptr);
	const LPVOID base_addr = MapViewOfFile(f_map, FILE_MAP_READ, 0, 0, 0);

	if (f_map == nullptr && base_addr == nullptr)
	{
		printf("Error reading file.\n");
		return nullptr;
	}

	unsigned char* lp_mapped_file = static_cast<unsigned char*>(base_addr);

	if (lp_mapped_file == nullptr)
		return nullptr;

	//Every JPEG file starts from binary value '0xFFD8'(Start of image), ends by binary value '0xFFD9'(End of image). 
	//There are several binary 0xFFXX data in JPEG data, they are called as "Marker", and it means the period of JPEG information data. 
	while (!(lp_mapped_file[0] == 0xFF && lp_mapped_file[1] == 0xE1) && !(lp_mapped_file[0] == 0xFF && lp_mapped_file[1] == 0xD9))
	{
		lp_mapped_file++;
	}

	lp_mapped_file += 2;

	UnmapViewOfFile(f_map);
	CloseHandle(h_file);

	return reinterpret_cast<EXIF_STRUCT*>(lp_mapped_file);
}

LPVOID _GetGPSIFDValue(char* tiff_h, IFD_STRUCT* ifd)
{
	for (int i = 0; i < ifd->number_directory_entry; i++)
	{
		if (ifd->directory[i].tag == GPS_INFO_TAG)
		{
			IFD_STRUCT* gps_ifd = reinterpret_cast<IFD_STRUCT*>(tiff_h + ifd->directory[i].data);

			GPS_IFD_STRUCT* gps = new GPS_IFD_STRUCT();

			for (int g = 0; g < gps_ifd->number_directory_entry; g++)
			{
				switch (gps_ifd->directory[g].tag)
				{
					case GPS_NORTH_SOUTH_LAT_TAG:
					{
						gps->north_south = static_cast<char>(gps_ifd->directory[g].data);
						break;
					}
					case GPS_LATITUDE_TAG:
					{
						//RATIONAL* rational = reinterpret_cast<RATIONAL*>(tiff_h) + gps_ifd->directory[g].data;
						//printf("0x%04x\t- %d / %d\n", ifd->directory[g].tag, rational->num, rational->denom);

						gps->latitude = gps_ifd->directory[g].data;
						break;
					}
					case GPS_EAST_WEST_LONG_ID:
					{
						gps->east_west = static_cast<char>(gps_ifd->directory[g].data);
						break;
					}
					case GPS_LONGITUDE_TAG:
					{
						//RATIONAL* rational = reinterpret_cast<RATIONAL*>(tiff_h) + gps_ifd->directory[g].data;
						//printf("0x%04x\t- %d / %d\n", ifd->directory[g].tag, rational->num, rational->denom);

						gps->longitude = gps_ifd->directory[g].data;
						break;
					}
					case GPS_ALTITUDE_TAG:
					{
						//RATIONAL* rational = reinterpret_cast<RATIONAL*>(tiff_h) + gps_ifd->directory[g].data;
						//printf("0x%04x\t- %d / %d\n", ifd->directory[g].tag, rational->num, rational->denom);

						gps->altitude = gps_ifd->directory[g].data;
						break;
					}
					default:
					{
						break;
					}
				}
			}
			return gps;
		}
	}

	return nullptr;
}

LPVOID _GetIFDValue(char* tiff_h, IFD_STRUCT* ifd, unsigned short targetTag)
{
	for (int i = 0; i < ifd->number_directory_entry; i++)
	{
		if (ifd->directory[i].tag == EXIF_OFFSET_TAG)
		{
			return _GetIFDValue(tiff_h, reinterpret_cast<IFD_STRUCT*>(tiff_h + ifd->directory[i].data), targetTag);
		}

		if (targetTag == ifd->directory[i].tag)
		{
			const short size_of_data = DATA_FORMATS[ifd->directory[i].data_format - 1] * ifd->directory[i].number_components;

			if (size_of_data > 4)
				return tiff_h + ifd->directory[i].data;

			return  &ifd->directory[i].data;
		}
	}

	return nullptr;
}

/*
	Modelo: <camera model value>
	Dimensão : <width value> px x <height value> px
	Data : <date and time value>
	ISO : <ISO value>
	Velocidade : 1 / <exposure time value> s
	Abertura : F 1 / <aperture value>
	Latitude : <latitude value> <N | S>
	Longitude : <longitude value> <E | W>
	Altitude : <altitude value> m
*/

EXIF_API void PrintExifTags(TCHAR* filename)
{
#ifdef UNICODE
	// TCHAR type is wchar_t
	std::vector<char> buffer;
	int size = WideCharToMultiByte(CP_UTF8, 0, filename, -1, nullptr, 0, nullptr, nullptr);
	if (size > 0) {
		buffer.resize(size);
		WideCharToMultiByte(CP_UTF8, 0, filename, -1, &buffer[0], buffer.size(), nullptr, nullptr);
	}
	else {
		printf("Error reading UNICODE filename with WideCharToMultiByte (%d)", GetLastError());
		return;
	}
	//*/
	std::string string(&buffer[0]);
#else
	// TCHAR type is char
	std::string string(filename);
#endif

	EXIF_STRUCT* exif = _MapFileInfoFromPath(filename);

	if (exif == nullptr)
	{
		printf("Error. Couldn't find or map file on path %s", filename);
		return;
	}

	printf("\n%s:\n\n", filename);

	char* tiff_h = reinterpret_cast<char*>(&exif->TIFF_header);
	IFD_STRUCT* exif_ifd = static_cast<IFD_STRUCT*>(&exif->TIFF_header.IFD);

	// TODO DEFINE STRUCTURE WITH ALL FIELDS

	char* model = static_cast<char*>(_GetIFDValue(tiff_h, exif_ifd, MODEL_TAG));
	unsigned short* width = static_cast<unsigned short*>(_GetIFDValue(tiff_h, exif_ifd, WIDTH_TAG));
	unsigned short* height = static_cast<unsigned short*>(_GetIFDValue(tiff_h, exif_ifd, HEIGH_TAG));
	char* data = static_cast<char*>(_GetIFDValue(tiff_h, exif_ifd, DATA_TAG));
	unsigned short* iso = static_cast<unsigned short *>(_GetIFDValue(tiff_h, exif_ifd, ISO_TAG));
	RATIONAL* shutterSpeed = static_cast<RATIONAL*>(_GetIFDValue(tiff_h, exif_ifd, SHUTTER_TAG));
	URATIONAL* aperture = static_cast<URATIONAL*>(_GetIFDValue(tiff_h, exif_ifd, APERTURE_TAG));

	model != nullptr ? printf("\tModelo: %s\n", model) : printf("\tModelo: <n/a>\n");
	width != nullptr && height != nullptr ? printf("\tDimensao: %upx x %upx\n", *width, *height) : printf("\tDimensao: <n/a>\n");
	data != nullptr ? printf("\tData: %s\n", data) : printf("\tData: <n/a>\n");
	iso != nullptr ? printf("\tISO: %u\n", *iso) : printf("\tISO: <n/a>\n");
	shutterSpeed != nullptr ? printf("\tVelocidade: %d / %d\n", shutterSpeed->num, shutterSpeed->denom) : printf("\tVelocidade: <n/a>\n");
	aperture != nullptr ? printf("\tAbertura: F %u / %u\n", aperture->num, aperture->denom) : printf("\tAbertura: <n/a>\n");

	GPS_IFD_STRUCT* gps = static_cast<GPS_IFD_STRUCT*>(_GetGPSIFDValue(tiff_h, exif_ifd));
	if (gps != nullptr)
	{
		printf("\tLatitude: %.2f <%c>\n", gps->latitude, gps->north_south);
		printf("\tLongitude: %.2f <%c>\n", gps->longitude, gps->east_west);
		printf("\tAltitude: %.2f m\n", gps->altitude);
	}
	else
		printf("\tGPS: <n/a>\n");
}
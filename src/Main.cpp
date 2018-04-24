#include "stdio.h"
#include "windows.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"

/*
TODO:
- maybe should make it change the white balance of the image to make the range more noticable
  - essentially make it so an even number of the different characters are used, reguardless of the brightness of the image
  - i could take the whitest white and blackest black, and have that as the range of the colours
    or maybe instead of the range, i could do the quartile range to account for a few very white / black outliers

- i should have a maximum size for a text image and do some bilinear filtering to downscale the image for conversion to txt img
*/

#define arraySize(x) (sizeof(x) / sizeof(x[0]))
#define Assert(x) if(!(x)){(*(int*)0) = 0;}

void saveFile(char* name, void* mem, int size)
{
	HANDLE fileHandle = CreateFile(name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	int done = WriteFile(fileHandle, mem, size, 0, 0);
	Assert(done);
	CloseHandle(fileHandle);
}

int main()
{
	char greyscaleList[4][4] =
	{
		{ '@', 'V', 'Y', '^' },
		{ 'g', '$', 'T', '\"' },
		{ 'g', 'm', '#', '\'' },
		{ 'm', ',', '.', ' ' }
	};

	int greyscaleListSize = 4;

	LPCSTR filenames[] = 
	{
		"*.png",
		"*.jpg",
		"*.bmp"
	};

	HANDLE searchHandle;
	WIN32_FIND_DATA findData;

	for (int filenameIndex = 0; filenameIndex < arraySize(filenames); filenameIndex++)
	{
		LPCSTR filename = filenames[filenameIndex];
		int numFiles = 0;

		for (;;)
		{
			//find all files
			if (numFiles == 0)
			{
				searchHandle = FindFirstFile(filename, &findData);
				if (searchHandle == INVALID_HANDLE_VALUE) { break; }
			}
			else
			{
				if (!FindNextFile(searchHandle, &findData))
				{
					if (GetLastError() == ERROR_NO_MORE_FILES) { break; }
				}
			}
			++numFiles;
			printf("%d: %s\n", numFiles, findData.cFileName);

			//load image greyscale
			int x, y, componentsPerPixel;
			unsigned char* imgData = stbi_load(findData.cFileName, &x, &y, &componentsPerPixel, 1);

			int dataSize = ((x + 2) * ((y / 2) + 1)) + 1;
			char* charData = new char[dataSize];
			int charDataIndex = 0;

			//convert to txt file
			int greyscaleListBound = (int)(255.0f / (float)greyscaleListSize) + 1;

			int index = 0;

			for (int downIndex = 0; downIndex < y; downIndex += 2)
			{
				for (int acrossIndex = 0; acrossIndex < x; ++acrossIndex)
				{
					int firstPixelGrey = 0;
					int secondPixelGrey = 0;

					for (int greyscaleIndex = 0; greyscaleIndex < greyscaleListSize; ++greyscaleIndex)
					{
						if (imgData[index] < ((greyscaleIndex + 1) * greyscaleListBound))
						{
							firstPixelGrey = greyscaleIndex;
							break;
						}
					}

					for (int greyscaleIndex = 0; greyscaleIndex < greyscaleListSize; ++greyscaleIndex)
					{
						if (imgData[index + x] < ((greyscaleIndex + 1) * greyscaleListBound))
						{
							secondPixelGrey = greyscaleIndex;
							break;
						}
					}

					charData[charDataIndex++] = greyscaleList[secondPixelGrey][firstPixelGrey];
					index++;
				}
				charData[charDataIndex++] = '\r';
				charData[charDataIndex++] = '\n';
				index += x;
			}

			charData[charDataIndex] = 0;

			char* name = findData.cFileName;
			{
				char* i = name;
				while (*(i++));
				*(i - 4) = 't';
				*(i - 3) = 'x';
				*(i - 2) = 't';
			}

			saveFile(name, charData, charDataIndex);

			delete[] charData;
		}
	}

	FindClose(searchHandle);
	return 0;
}
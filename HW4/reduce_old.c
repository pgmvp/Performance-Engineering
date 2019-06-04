#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

typedef uint32_t DWORD;   // DWORD = unsigned 32 bit value
typedef uint16_t WORD;    // WORD = unsigned 16 bit value
typedef unsigned long LONG;

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;  //specifies the file type
    DWORD bfSize;  //specifies the size in bytes of the bitmap file
    WORD bfReserved1;  //reserved; must be 0
    WORD bfReserved2;  //reserved; must be 0
    DWORD bfOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER __attribute__((__packed__));

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;  //specifies the number of bytes required by the struct
    DWORD biWidth;  //specifies width in pixels
    DWORD biHeight;  //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage;  //size of image in bytes
    DWORD biXPelsPerMeter;  //number of pixels per meter in x axis
    DWORD biYPelsPerMeter;  //number of pixels per meter in y axis
    DWORD biClrUsed;  //number of colors used by th ebitmap
    DWORD biClrImportant;  //number of colors that are important
}BITMAPINFOHEADER __attribute__((__packed__));

#pragma pack(pop)

unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE *filePtr; //our file pointer
    BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
    unsigned char *bitmapImage;  //store image data
    int imageIdx=0;  //image index counter
    unsigned char tempRGB;  //our swap variable

    //open filename in read binary mode
    filePtr = fopen(filename,"rb");
    if (filePtr == NULL)
        return NULL;

    //read the bitmap file header
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER),1,filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader.bfType !=0x4D42)
    {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr);

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    //allocate enough memory for the bitmap image data
    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

    //verify memory allocation
    if (!bitmapImage)
    {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    //read in the bitmap image data
    fread(bitmapImage, bitmapInfoHeader->biSizeImage, 1, filePtr);

    //make sure bitmap image data was read
    if (bitmapImage == NULL)
    {
        fclose(filePtr);
        return NULL;
    }

    //swap the r and b values to get RGB (bitmap is BGR)
    for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage;imageIdx+=3) // fixed semicolon
    {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }

    //close file and return bitmap image data
    fclose(filePtr);
    return bitmapImage;
}

unsigned char** LoadBitmapFile3C(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE *filePtr; //our file pointer
    BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
    unsigned char *bitmapImage;  //store image data
    int imageIdx=0;  //image index counter
    unsigned char tempRGB;  //our swap variable



    //open filename in read binary mode
    filePtr = fopen(filename,"rb");
    if (filePtr == NULL)
        return NULL;

    //read the bitmap file header
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER),1,filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader.bfType !=0x4D42)
    {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr);

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    //allocate enough memory for the bitmap image data
    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

    unsigned char* r = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage / 3);
    unsigned char* g = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage / 3);
    unsigned char* b = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage / 3);


    //verify memory allocation
    if (!r || !g || !b || !bitmapImage)
    {
        free(r);
        free(g);
        free(b);
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    //read in the bitmap image data
    fread(bitmapImage, bitmapInfoHeader->biSizeImage, 1, filePtr);

    //make sure bitmap image data was read
    if (bitmapImage == NULL)
    {
        fclose(filePtr);
        return NULL;
    }

    //swap the r and b values to get RGB (bitmap is BGR)
    for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage;imageIdx+=3) // fixed semicolon
    {
        b[imageIdx / 3] = bitmapImage[imageIdx];
        g[imageIdx / 3] = bitmapImage[imageIdx + 1];
        r[imageIdx / 3] = bitmapImage[imageIdx + 2];
    }

    //close file and return bitmap image data
    fclose(filePtr);
    free(bitmapImage);

    unsigned char **res = (unsigned char**)malloc(3 *sizeof(unsigned char*));
    res[0] = r;
    res[1] = g;
    res[2] = b;

    return res;
}

int main (int argc, char *argv[]) {
    int rank, size;
	
	int rc = 1;
	
	long global_sum = 0;
	
	MPI_Status Stat;
    
    //initialize MPI library
    MPI_Init (&argc, &argv);
    
    //get number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size); 
    
    //get my process id(rank)
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    
    //do something
    printf ("Print message from rank (process id) %d \n", rank);
    if (rank == 0)
	{
		printf("MPI_WORLD size = %d processes\n", size);
		
		BITMAPINFOHEADER *bitmapInfoHeader;
		uint32_t image_size;
		uint32_t image_width;
		uint32_t image_height;

		bitmapInfoHeader = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER));

		unsigned char** image = LoadBitmapFile3C("./lenna.bmp", bitmapInfoHeader);

		unsigned char* channel = (unsigned char*)malloc(12 * sizeof(unsigned char));
		for(int i  = 0; i < 12; ++i)
			channel[i] = i;

		image_size = bitmapInfoHeader->biSizeImage;
		image_width = bitmapInfoHeader->biWidth;
		image_height = bitmapInfoHeader->biHeight;

		printf("%lu, %d, %d\n", image_size, image_height, image_width);
		
		int channel_size = 12;
		
		for(int i = 1; i != size; ++i)
		{
			int part = channel_size / (size - 1) + (channel_size % (size - 1)) * (int)(i == size - 1);
			unsigned char* p = (unsigned char*)malloc(part * sizeof(unsigned char));
			memcpy(p, channel + (i - 1) * channel_size / (size - 1), part);
			//printf("PPPP: %d, %d, %d", p[0], p[1], p[2]);
			//rc= MPI_Send(p, 3, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
		}
		
		
		free(channel);
		free(bitmapInfoHeader);
	}		
	else 
	{
		unsigned char* p = (unsigned char*)malloc(3 * sizeof(unsigned char));
		rc = MPI_Recv(p, 3, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, &Stat);
		
		
		float local_sum = 0;
		for(int i = 0; i < 3; ++i)
			local_sum += p[i];
		
		MPI_Reduce(&local_sum, &global_sum, 1, MPI_UNSIGNED_CHAR, MPI_SUM, 0, MPI_COMM_WORLD);
		
		printf("GGG: %d", global_sum);
		
		printf("Task %d: \n", rank);
		printf("PPPP: %d, %d, %d", p[0], p[1], p[2]);
	}
	
	if(rank == 0)
	{
		sleep(3);
		printf("REDUCE: %d\n", global_sum);
	}
    
    //MPI cleanup
    MPI_Finalize(); 
    return 0;
}

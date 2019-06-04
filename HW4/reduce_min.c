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
	
	if(argc != 2)
	{
		printf("Specify image path!\n");
		return 1;
	}
	
    int rank, size;
	
	unsigned char* channel;
	unsigned char* partial;
	
	uint32_t image_size;
	uint32_t image_width; 	
	uint32_t image_height;
	
	long global_min = 255;
	int partial_size;
	
	MPI_Status Stat;
    
    //initialize MPI library
    MPI_Init (&argc, &argv);
    
    //get number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size); 
    
    //get my process id(rank)
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    
    if (rank == 0)
	{
		printf("MPI_WORLD size = %d processes\n", size);
		
		BITMAPINFOHEADER *bitmapInfoHeader;
		bitmapInfoHeader = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER));

		unsigned char** image = LoadBitmapFile3C(argv[1], bitmapInfoHeader);
		
		image_size = bitmapInfoHeader->biSizeImage;
		image_width = bitmapInfoHeader->biWidth;
		image_height = bitmapInfoHeader->biHeight;
		
		int channel_size = image_size / 3;
		channel = image[0];
		partial_size = ceil((double)channel_size / size);
		
		for(int i = 0; i < size; ++i)
			MPI_Send(&partial_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		
		int diff = partial_size * size - channel_size;
		if (diff)
		{
			unsigned char* temp = (unsigned char*)malloc(partial_size * size * sizeof(unsigned char));
			memcpy(temp, channel, channel_size);
			for(int i = 0; i < diff; ++i)
				temp[channel_size + i] = 255;
			
			free(channel);
			channel = temp;
			temp = 0;
		}

		printf("%lu, %d, %d\n", image_size, image_height, image_width);
		free(bitmapInfoHeader);
	}
	else 
	{
		MPI_Recv(&partial_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &Stat);
	}
	
	double start = MPI_Wtime();
	partial = (unsigned char*)malloc(partial_size * sizeof(unsigned char));
	MPI_Scatter(channel, partial_size, MPI_UNSIGNED_CHAR, partial, partial_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
		
	int local_min = 255;
	for(int i = 0; i < partial_size; ++i)
		if(partial[i] < local_min)
			local_min = partial[i];
		
	MPI_Reduce(&local_min, &global_min, 1, MPI_LONG, MPI_MIN, 0, MPI_COMM_WORLD);
	
	double end = MPI_Wtime();
	

	if(rank == 0)
	{
		printf("REDUCE: %d\n", global_min);
		printf("TIME ELAPSED DISTRIBUTED: %f sec\n", end - start);
		
		start = MPI_Wtime();
		int single_min = 255;
		for(int i = 0; i < image_size / 3; ++i)
			if(channel[i] < single_min)
				single_min = channel[i];
		end = MPI_Wtime();
		printf("TIME ELAPSED SINGLE PROCESS: %f sec\n", end - start);	
		
		if(global_min == single_min)
			printf("All is ok\n");
		else
			printf("Incorrect result\n");
		
	}
    
    //MPI cleanup
    MPI_Finalize(); 
    return 0;
}

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

void transpose(unsigned char* input, unsigned char* output, int width, int height)
{
	for(int i = 0; i < height; ++i)
	{
		for(int j = 0; j < width; ++j)
		{
			output[i * height + j] = input[j * height + i];
		}
	}
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
	unsigned char* receive_data;
	unsigned char* transposed;
	
	long global_sum = 0;
	int partial_size, unit_size, unit_num;
	uint32_t image_size;
	uint32_t image_width; 
	uint32_t image_height;
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
		
		image_size   = bitmapInfoHeader->biSizeImage;
		image_width  = bitmapInfoHeader->biWidth;
		image_height = bitmapInfoHeader->biHeight;
		
		image_width = 4;
		image_height = 4;
		image_size = 16;
		unit_size = image_width;
		
		channel = (unsigned char*)malloc(16 * sizeof(unsigned char));
		for(int i = 0; i < 16; ++i)
			channel[i] = 1;
		
		// int channel_size = image_size / 3;
		int channel_size = image_size;
		// channel = image[0];
		unit_num = ceil((double)image_height / size);
		
		partial_size = unit_num * unit_size;
		
		for(int i = 0; i < size; ++i)
			MPI_Send(&partial_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		
		receive_data = (unsigned char*)malloc(partial_size * size * sizeof(unsigned char));
		
		int diff = partial_size * size - channel_size;
		if (diff)
		{
			printf("!!\n");
			unsigned char* temp = (unsigned char*)malloc(partial_size * size * sizeof(unsigned char));
			memcpy(temp, channel, channel_size);
			for(int i = 0; i < diff; ++i)
				temp[channel_size + i] = 0;
			
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
	
	// printf("TASK %d, PART_SIZE %d\n", rank, partial_size);
	unsigned char* S_1 = (unsigned char*)malloc(partial_size * sizeof(unsigned char));	
	
	S_1[0] = partial[0];
	for(int i = 1; i < partial_size; ++i)
		S_1[i] = partial[i] + S_1[i - 1];
	
	// printf("TASK %d, S: %d, %d, %d", rank, S_1[0], S_1[1], S_1[2]);
		
	MPI_Gather(S_1, partial_size, MPI_UNSIGNED_CHAR, receive_data, partial_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

	if(rank == 0)
	{
		printf("AFTER GATHER:\n");
		for(int i = 0; i < partial_size * size; ++i)
			printf("%d  ", receive_data[i]);
		
		//transpose received data
		transposed = (unsigned char*)malloc(partial_size * size * sizeof(unsigned char));
		transpose(receive_data, transposed, 4, 4);
		
		printf("AFTER TRANSPOSE:\n");
		for(int i = 0; i < partial_size * size; ++i)
			printf("%d  ", transposed[i]);
	}
	
	MPI_Scatter(transposed, partial_size, MPI_UNSIGNED_CHAR, partial, partial_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
	
	S_1[0] = partial[0];
	for(int i = 1; i < partial_size; ++i)
		S_1[i] = partial[i] + S_1[i - 1];
	
	// printf("TASK %d, S: %d, %d, %d", rank, S_1[0], S_1[1], S_1[2]);
		
	MPI_Gather(S_1, partial_size, MPI_UNSIGNED_CHAR, receive_data, partial_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
	
	if(rank == 0)
	{
		printf("AFTER GATHER 2:\n");
		for(int i = 0; i < partial_size * size; ++i)
			printf("%d  ", receive_data[i]);
		
		//transpose received data
		transpose(receive_data, transposed, 4, 4);
		
		printf("AFTER TRANSPOSE 2:\n");
		for(int i = 0; i < partial_size * size; ++i)
			printf("%d  ", transposed[i]);
	}
    
	
	double end = MPI_Wtime();
    //MPI cleanup
    MPI_Finalize(); 
    return 0;
}

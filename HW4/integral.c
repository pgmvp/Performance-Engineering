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
			output[j * width + i] = input[i * height + j];
		}
	}
}

void transpose2(unsigned char *m, int w, int h)
{
	int start, next, i;
	unsigned char tmp;
 
	for (start = 0; start <= w * h - 1; start++) {
		next = start;
		i = 0;
		do {	i++;
			next = (next % h) * w + next / h;
		} while (next > start);
		if (next < start || i == 1) continue;
 
		tmp = m[next = start];
		do {
			i = (next % h) * w + next / h;
			m[next] = (i == start) ? tmp : m[i];
			next = i;
		} while (next > start);
	}
}

void calc_integral_image(unsigned char* input,  unsigned char* output, int width, int height)
{
	printf("%d, %d\n", width, height);
	unsigned char* temp = (unsigned char*)malloc(width * height * sizeof(unsigned char));
	
	temp[0] = input[0];
	for(int i = 1; i < width * height; ++i)
	{
		if(i % width == 0)
			temp[i] = input[i];
		else
			temp[i] = input[i] + temp[i - 1];
	}
	
	transpose2(temp, width, height);
	
	output[0] = temp[0];
	for(int i = 1; i < width * height; ++i)
	{
		if(i % height == 0)
			output[i] = temp[i];
		else
			output[i] = temp[i] + output[i - 1];
	}
	
	transpose2(output, height, width);
	
	free(temp);
}

int check_images(unsigned char* g_truth, unsigned char* test, int size)
{
	for(int i = 0; i < size; ++i)
	{
		if(g_truth[i] != test[i])
			return 0;
	}
	
	return 1;
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
	unsigned char* padded_data;
	
	long global_sum = 0;
	int partial_size, unit_size, unit_num, actual_num, diff, channel_size;
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
	
		channel_size = image_size / 3;

		channel = image[0];
		
		unit_num = image_height % size;
		
		actual_num = image_height / size;
		
		partial_size = (actual_num + unit_num) * image_width;
		
		for(int i = 0; i < size; ++i)
		{
			MPI_Send(&partial_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&actual_num, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&image_width, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&image_height, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
		
		receive_data = (unsigned char*)malloc(partial_size * size * sizeof(unsigned char));
		padded_data  = (unsigned char*)malloc(partial_size * size * sizeof(unsigned char));
		
		diff = size - unit_num;
		if (diff)
		{
			printf("DIFF %d\n", diff);
			for(int i = 0; i < size - 1; ++i)
			{
				unsigned char* temp = (unsigned char*)malloc(partial_size * sizeof(unsigned char));
				memcpy(temp, channel + (i * actual_num * image_width), actual_num * image_width);
				for(int i = 0; i < diff * image_width; ++i)
					temp[actual_num * image_width + i] = 0;
				
				memcpy(padded_data + (i * partial_size), temp, partial_size);
				free(temp);
			}
			
			memcpy(padded_data + (size - 1) * partial_size, channel + (size - 1) * actual_num * image_width, partial_size);
		}
		
		printf("%lu, %d, %d\n", image_size, image_height, image_width);
		free(bitmapInfoHeader);
	}
	else 
	{
		MPI_Recv(&partial_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &Stat);
		MPI_Recv(&actual_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &Stat);
		MPI_Recv(&image_width, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &Stat);
		MPI_Recv(&image_height, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &Stat);
	}
	
	double start = MPI_Wtime();
	partial = (unsigned char*)malloc(partial_size * sizeof(unsigned char));
	
	MPI_Scatter(padded_data, partial_size, MPI_UNSIGNED_CHAR, partial, partial_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
	
	unsigned char* S_1 = (unsigned char*)malloc(partial_size * sizeof(unsigned char));	
	
	S_1[0] = partial[0];
	for(int i = 1; i < partial_size; ++i)
	{
		if(i % image_width == 0)
			S_1[i] = partial[i];
		else
			S_1[i] = partial[i] + S_1[i - 1];
	}
		
	MPI_Gather(S_1, partial_size, MPI_UNSIGNED_CHAR, receive_data, partial_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
	if(rank == 0)
	{	
		//take out zeros
		unsigned char* wo_padding = (unsigned char*)malloc(channel_size * sizeof(unsigned char));
		for(int i = 0; i < size - 1; ++i)
		{
			memcpy(wo_padding + (i * actual_num * image_width), receive_data + (i * partial_size), actual_num * image_width);
		}
		memcpy(wo_padding + (size - 1) * actual_num * image_width, receive_data + (size - 1) * partial_size, partial_size);
		
		//transpose received data
		transpose2(wo_padding, image_width, image_height);
		
		unit_num = image_width % size;
		actual_num = image_width / size;
		partial_size = (actual_num + unit_num) * image_height;
		
		free(padded_data);
		free(receive_data);
		receive_data = (unsigned char*)malloc(partial_size * size * sizeof(unsigned char));
		padded_data = (unsigned char*)malloc(partial_size * size * sizeof(unsigned char));
		
		diff = size - unit_num;
		if (diff)
		{
			//add padding again
			for(int i = 0; i < size - 1; ++i)
			{
				unsigned char* temp = (unsigned char*)malloc(partial_size * sizeof(unsigned char));
				memcpy(temp, wo_padding + (i * actual_num * image_height), actual_num * image_height);
				for(int i = 0; i < diff * image_height; ++i)
					temp[actual_num * image_height + i] = 0;
				
				memcpy(padded_data + (i * partial_size), temp, partial_size);
				free(temp);
			}
			
			memcpy(padded_data + (size - 1) * partial_size, wo_padding + (size - 1) * actual_num * image_height, partial_size);
		}
	}

	free(partial);
	partial = (unsigned char*)malloc((image_width % size + image_width / size) * image_height * sizeof(unsigned char));
	MPI_Scatter(padded_data, (image_width % size + image_width / size) * image_height, MPI_UNSIGNED_CHAR, partial, (image_width % size + image_width / size) * image_height, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

	free(S_1);
	S_1 = (unsigned char*)malloc((image_width % size + image_width / size) * image_height * sizeof(unsigned char));
	S_1[0] = partial[0];
	for(int i = 1; i < (image_width % size + image_width / size) * image_height; ++i)
	{
		if(i % image_height == 0)
			S_1[i] = partial[i];
		else
		S_1[i] = partial[i] + S_1[i - 1];
	}
	
	MPI_Gather(S_1, (image_width % size + image_width / size) * image_height, MPI_UNSIGNED_CHAR, receive_data, (image_width % size + image_width / size) * image_height, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
	
	if(rank == 0)
	{	
		unsigned char* wo_padding = (unsigned char*)malloc(channel_size * sizeof(unsigned char));
		for(int i = 0; i < size - 1; ++i)
		{
			memcpy(wo_padding + (i * actual_num * image_height), receive_data + (i * partial_size), actual_num * image_height);
		}
		memcpy(wo_padding + (size - 1) * actual_num * image_height, receive_data + (size - 1) * partial_size, partial_size);
		
		transpose2(wo_padding, image_height, image_width);
		
		double end = MPI_Wtime();
		
		printf("TIME ELAPSED DISTRIBUTED: %f sec\n", end - start);
		
		unsigned char* test = (unsigned char*)malloc(channel_size * sizeof(unsigned char));
		start = MPI_Wtime();
		calc_integral_image(channel, test, image_width, image_height);
		end = MPI_Wtime();
		
		printf("TIME ELAPSED SINGLE PROCESS: %f sec\n", end - start);
		
		if(!check_images(test, wo_padding, channel_size))
			printf("Incorrect result!\n");
		else
			printf("All is ok\n");
		
		
		free(test);
		free(wo_padding);
	}
	
	free(channel);
	
    //MPI cleanup
    MPI_Finalize(); 
    return 0;
}

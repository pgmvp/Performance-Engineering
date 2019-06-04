#include <stdio.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include <stdint.h>
#include <math.h>

#include <thread>
#include <vector>
#include <future>
#include <numeric>
#include <chrono>
#include <algorithm>

using namespace std;

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

template <typename T>
T add(const std::vector<T>& channel, int begin, int end)
{
//    T res = T();
//    for(int i = begin; i != end; ++i)
//        res += channel[i];

//    return res;
    return std::accumulate(channel.begin() + begin, channel.begin() + end, T());
}

template <typename T>
T parallel_sum1(const std::vector<T>& channel)
{
    auto copy = channel;
    auto blocks_num = std::thread::hardware_concurrency();

    int part = ceil((double)copy.size() / blocks_num);
    std::cout << part << std::endl;

    while(copy.size() != part * blocks_num)
        copy.push_back(0);

    std::vector<std::future<T>> tasks;
    for(int i = 0; i != blocks_num; ++i)
    {
        tasks.emplace_back(std::async(std::launch::async, add<T>, copy, i * part, i * part + part));
    }

    T sum = 0;
    for(auto& t : tasks)
        sum += t.get();

    return sum;
}

template <typename T>
T parallel_sum2(const std::vector<T>& channel)
{
    auto blocks_num = std::thread::hardware_concurrency();

    std::vector<std::future<T>> tasks;
    for(int i = 0; i != blocks_num; ++i)
    {
        int size = channel.size() / blocks_num + (channel.size() % blocks_num) * (int)(i == blocks_num - 1);
        tasks.emplace_back(std::async(std::launch::async, add<T>, channel, i * size, (i + 1) * size));
    }

    long sum = 0;
    for(auto& t : tasks)
        sum += t.get();

    return sum;
}

template <typename Iterator>
typename std::iterator_traits<Iterator>::value_type
adder(Iterator begin, Iterator end)
{
    using T = typename std::iterator_traits<Iterator>::value_type;
    return std::accumulate(begin, end, T());
}

template <typename Iterator>
typename std::iterator_traits<Iterator>::value_type
parallel_sum3(Iterator begin, Iterator end)
{
    using T = typename std::iterator_traits<Iterator>::value_type;
    auto mid = begin + std::distance(begin, end) / 2;
    auto sum1 = std::async(std::launch::async, adder<Iterator>, begin, mid);
    auto sum2 = std::async(std::launch::async, adder<Iterator>, mid, end);

    return sum1.get() + sum2.get();
}


//template <typename Iterator>
//typename std::iterator_traits<Iterator>::value_type
//min(Iterator begin, Iterator end)
//{
//    using T = typename std::iterator_traits<Iterator>::value_type;
////    return std::accumulate(begin, end, 255, std::less<T>());
//    return *std::min_element(begin, end);
//}

//template <typename Iterator>
//typename std::iterator_traits<Iterator>::value_type
//parallel_min(Iterator begin, Iterator end)
//{
//    using T = typename std::iterator_traits<Iterator>::value_type;
//    auto mid = begin + std::distance(begin, end) / 2;
//    auto sum1 = std::async(std::launch::async, min<Iterator>, begin, mid);
//    auto sum2 = std::async(std::launch::async, min<Iterator>, mid, end);

//    auto res1 = sum1.get();
//    auto res2 = sum2.get();
//    return  res1 < res2 ? res1 : res2;
//}

template <typename Iterator>
typename std::iterator_traits<Iterator>::value_type
min1(Iterator begin, Iterator end)
{
    return *std::min_element(begin, end);
}

template <typename T>
T parallel_min2(const std::vector<T>& channel)
{
    using Iterator = typename std::vector<T>::const_iterator;
    auto blocks_num = std::thread::hardware_concurrency();

    std::vector<std::future<T>> tasks;
    auto begin = channel.begin();
    for(int i = 0; i != blocks_num; ++i)
    {
        int size = channel.size() / blocks_num + (channel.size() % blocks_num) * (int)(i == blocks_num - 1);
        tasks.emplace_back(std::async(std::launch::async, min1<Iterator>, begin + i * size, begin + (i + 1) * size));
    }

    long min = 255;
    for(auto& t : tasks)
    {
        auto res = t.get();
        if(res < min)
            min = res;
    }

    return min;
}

template <typename T>
std::vector<T> cumsum(const std::vector<T>& input, int block)
{
    std::vector<T> res(input.size());
    res[0] = input[0];
    for(int i = 1; i < input.size(); ++i)
    {
        if(i % block == 0)
            res[i] = input[i];
        else
            res[i] = res[i - 1] + input[i];
    }

    return res;
}

template <typename T>
void transpose(std::vector<T>& input, int w, int h)
{
    int start, next, i;
    T tmp;

    for (start = 0; start <= w * h - 1; start++) {
        next = start;
        i = 0;
        do {	i++;
            next = (next % h) * w + next / h;
        } while (next > start);
        if (next < start || i == 1) continue;

        tmp = input[next = start];
        do {
            i = (next % h) * w + next / h;
            input[next] = (i == start) ? tmp : input[i];
            next = i;
        } while (next > start);
    }
}

template <typename T>
std::vector<std::future<std::vector<T>>>
split_into_blocks(const std::vector<T>& channel, int d1, int d2)
{
    auto blocks_num = std::thread::hardware_concurrency();
    int partial_size = d1 / blocks_num;
    int size = partial_size * d2;

    std::vector<std::future<std::vector<T>>> tasks;
    for(int i = 0; i != blocks_num - 1; ++i)
    {
        std::vector<T> subset(channel.begin() + i * size, channel.begin() + (i + 1) * size);
        tasks.emplace_back(std::async(std::launch::async, cumsum<T>, subset, d2));
    }

    std::vector<T> subset(channel.begin() + (blocks_num - 1) * size,
                          channel.end());
    tasks.emplace_back(std::async(std::launch::async, cumsum<T>, subset, d2));

    return tasks;
}

template <typename T>
std::vector<T> integral_image(const std::vector<T>& channel, size_t width, size_t height)
{
    auto tasks = split_into_blocks(channel, height, width);

    std::vector<T> trans;
    trans.reserve(width * height);
    for(auto& t : tasks)
    {
        auto res = t.get();
        trans.insert(trans.end(), res.begin(), res.end());
    }

    transpose(trans, width, height);

    tasks = split_into_blocks(trans, width, height);
    std::vector<T> result;
    result.reserve(width * height);
    for(auto& t : tasks)
    {
        auto res = t.get();
        result.insert(result.end(), res.begin(), res.end());
    }

    transpose(result, height, width);
    return result;
}

template <typename T>
std::vector<T> cpu_integral_image(const std::vector<T>& channel, int width, int height)
{
    auto res = cumsum(channel, width);
    transpose(res, width, height);

    res = cumsum(res, height);
    transpose(res, height, width);

    return res;
}

template <typename T>
bool check_images(const std::vector<T>& g_truth, const std::vector<T>& test)
{
    if(g_truth.size() != test.size())
        return false;

    for(int i = 0; i < test.size(); ++i)
    {
        if(g_truth[i] != test[i])
            return false;
    }

    return true;
}

int main()
{
    BITMAPINFOHEADER *bitmapInfoHeader;
    uint32_t image_size;
    uint32_t image_width;
    uint32_t image_height;

    bitmapInfoHeader = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER));

    unsigned char** image = LoadBitmapFile3C("C:\\Users\\Max\\Documents\\QT\\Homework3\\small.bmp",
    bitmapInfoHeader);

    unsigned char* channel = image[0];

    image_size = bitmapInfoHeader->biSizeImage;
    image_width = bitmapInfoHeader->biWidth;
    image_height = bitmapInfoHeader->biHeight;

    printf("%lu, %d, %d\n", image_size, image_height, image_width);

    //    image_width = 5;
    //    image_height = 6;
    //    image_size = 30;
//    std::vector<unsigned char> vec = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    std::vector<long> vec(channel, channel + image_size / 3);

    //***************SUM***************
    auto start = std::chrono::system_clock::now();
    long sum = parallel_sum1(vec);
    auto end = std::chrono::system_clock::now();

    std::cout << "Parallel sum 1: " << sum << std::endl;
    std::chrono::duration<double> time = end - start;
    std::cout << "Parallel sum 1 time: " << time.count() << std::endl;


    start = std::chrono::system_clock::now();
    sum = parallel_sum2(vec);
    end = std::chrono::system_clock::now();

    std::cout << "Parallel sum 2: " << sum << std::endl;
    time = end - start;
    std::cout << "Parallel sum 2 time: " << time.count() << std::endl;


    start = std::chrono::system_clock::now();
    sum = parallel_sum3(vec.begin(), vec.end());
    end = std::chrono::system_clock::now();

    std::cout << "Parallel sum 3: " << sum << std::endl;
    time = end - start;
    std::cout << "Parallel sum 3 time: " << time.count() << std::endl;


    sum = 0;
    start = std::chrono::system_clock::now();
    for(int i = 0; i < image_size / 3; ++i)
        sum += vec[i];
    end = std::chrono::system_clock::now();

    std::cout << "CPU sum: " << sum << std::endl;
    time = end - start;
    std::cout << "CPU time: " << time.count() << std::endl;


    //***************MIN***************
    start = std::chrono::system_clock::now();
    sum = parallel_min2(vec);
    end = std::chrono::system_clock::now();

    std::cout << "Parallel min: " << sum << std::endl;
    time = end - start;
    std::cout << "Parallel min time: " << time.count() << std::endl;


    int min = 255;
    start = std::chrono::system_clock::now();
    for(int i = 0; i < image_size / 3; ++i)
        if(vec[i] < min)
            min = vec[i];
    end = std::chrono::system_clock::now();

    std::cout << "CPU min: " << min << std::endl;
    time = end - start;
    std::cout << "CPU time: " << time.count() << std::endl;


    //***************INTEGRAL IMAGE***************
    start = std::chrono::system_clock::now();
    auto par_integral = integral_image(vec, image_width, image_height);
    end = std::chrono::system_clock::now();
    time = end - start;
    std::cout << "Parallel integral image time: " << time.count() << std::endl;

    std::cout << "result:" << std::endl;
    for(auto v : par_integral)
        std::cout << (int)v << " ";

    start = std::chrono::system_clock::now();
    auto cpu_integral = cpu_integral_image(vec, image_width, image_height);
    end = std::chrono::system_clock::now();
    time = end - start;
    std::cout << "CPU integral image time: " << time.count() << std::endl;

    if(!check_images(cpu_integral, par_integral))
        std::cout << "Result is incorrect!" << std::endl;
    else
        std::cout << "All is ok!" << std::endl;

    free(channel);
    free(bitmapInfoHeader);

    return(0);
}

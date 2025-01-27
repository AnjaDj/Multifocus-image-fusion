#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdfix.h>
#include <builtins.h>
#include <cycle_count.h>

#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0
#define E 0.1

int kernel[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

typedef struct {
    int type;
    int size;
    int reserved1;
    int reserved2;
    int offset;
    int header_size;
    int width;
    int height;
    int planes;
    int bits_per_pixel;
    int compression;
    int image_size;
    int x_pixels_per_meter;
    int y_pixels_per_meter;
    int colors_used;
    int colors_important;
} bmp_header_t;

#pragma section("seg_block1")
unsigned char data1[30000];

#pragma section("seg_block2")
unsigned char data2[30000];

#pragma section("seg_sdram1")
unsigned char weightsA[30000];

#pragma section("seg_sdram1")
unsigned int localMAXs[30000];

#pragma section("seg_sdram1")
unsigned int localMINs[30000];

void read_image(bmp_header_t* bmp_header, FILE* in, unsigned char* pixels)
{
	int b1, b2, b3, b4, i = 0;

	/* CITANJE TIPA */
	b1 = fgetc(in);
	b2 = fgetc(in);
	bmp_header->type = b2;
	bmp_header->type = bmp_header->type << 8;
	bmp_header->type = bmp_header->type | b1;

	//printf("Tip: %c%c\n", bmp_header->type, bmp_header->type >> 8);

	/* CITANJE VELICINE FAJLA */
	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->size = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	/* PRESKAKANJE REZERVISANIH BAJTOVA */
	b1 = fgetc(in);
	bmp_header->reserved1 = (fgetc(in) << 8) | b1;
	b1 = fgetc(in);
	bmp_header->reserved2 = (fgetc(in) << 8) | b1;

	/* CITANJE OFFSETA */
	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->offset = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	/* CITANJE VELICINE HEDERA */
	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->header_size = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	/* CITANJE DIMENZIJA SLIKE */
	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->width = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->height = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	//printf("Dimenzije slike su: %d x %d\n", bmp_header->width, bmp_header->height);

	/* PLANES */
	b1 = fgetc(in);
	bmp_header->planes = (fgetc(in) << 8) | b1;

	/* BITS PER PIXEL */
	b1 = fgetc(in);
	bmp_header->bits_per_pixel = (fgetc(in) << 8) | b1;

	//printf("Bytes per pixel: %d\n", bmp_header->bits_per_pixel/8);

	/* COMPRESSION */
	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->compression = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	/* IMAGE SIZE */
	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->image_size = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	/* X/Y PIXELS PER METER */
	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->x_pixels_per_meter = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->y_pixels_per_meter = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	/* COLORS USED */
	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->colors_used = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;

	/* COLORS IMPORTANT */
	b1 = fgetc(in);
	b2 = fgetc(in);
	b3 = fgetc(in);
	bmp_header->colors_important = (((fgetc(in) << 24) | (b3 << 16)) | (b2 << 8)) | b1;


	/* READ PIXELS */
	int index = 0;
	int bytePerPixel    = (bmp_header->bits_per_pixel)/8;
	int unpaddedRowSize = bmp_header->width * bytePerPixel;
	int paddedRowSize   = (((bmp_header->width * bytePerPixel) + 3) / 4) * 4;
	int height          = bmp_header->height;

	for(int i = 0; i < height ; i++)
	{
		for (int j = 0; j < paddedRowSize; j++)
		{
			if ((expected_true(j+3 <= unpaddedRowSize)))
			{
				b1 = fgetc(in); j++;
				b2 = fgetc(in); j++;
				b3 = fgetc(in);
				unsigned char gray = muliur(b1 , 0.3ur) + muliur(b2 , 0.59ur) + muliur(b3 , 0.11ur);
				pixels[index++] = gray;
			}
			else
				fgetc(in); // Skip padding
		}
	}

}

void write_image(bmp_header_t* bmp_header, FILE* out, unsigned char* pixels)
{
    fputc(bmp_header->type, out);
    fputc((bmp_header->type >> 8), out);
    fputc(bmp_header->size, out);
    fputc((bmp_header->size >> 8), out);
    fputc((bmp_header->size >> 16), out);
    fputc((bmp_header->size >> 24), out);
    fputc(bmp_header->reserved1, out);
    fputc((bmp_header->reserved1 >> 8), out);
    fputc(bmp_header->reserved2, out);
    fputc((bmp_header->reserved2 >> 8), out);
    fputc(bmp_header->offset, out);
    fputc((bmp_header->offset >> 8), out);
    fputc((bmp_header->offset >> 16), out);
    fputc((bmp_header->offset >> 24), out);
    fputc(bmp_header->header_size, out);
    fputc((bmp_header->header_size >> 8), out);
    fputc((bmp_header->header_size >> 16), out);
    fputc((bmp_header->header_size >> 24), out);
    fputc(bmp_header->width, out);
    fputc((bmp_header->width >> 8), out);
    fputc((bmp_header->width >> 16), out);
    fputc((bmp_header->width >> 24), out);
    fputc(bmp_header->height, out);
    fputc((bmp_header->height >> 8), out);
    fputc((bmp_header->height >> 16), out);
    fputc((bmp_header->height >> 24), out);
    fputc(bmp_header->planes, out);
    fputc((bmp_header->planes >> 8), out);
    fputc(bmp_header->bits_per_pixel, out);
    fputc((bmp_header->bits_per_pixel >> 8), out);
    fputc(bmp_header->compression, out);
    fputc((bmp_header->compression >> 8), out);
    fputc((bmp_header->compression >> 16), out);
    fputc((bmp_header->compression >> 24), out);
    fputc(bmp_header->image_size, out);
    fputc((bmp_header->image_size >> 8), out);
    fputc((bmp_header->image_size >> 16), out);
    fputc((bmp_header->image_size >> 24), out);
    fputc(bmp_header->x_pixels_per_meter, out);
    fputc((bmp_header->x_pixels_per_meter >> 8), out);
    fputc((bmp_header->x_pixels_per_meter >> 16), out);
    fputc((bmp_header->x_pixels_per_meter >> 24), out);
    fputc(bmp_header->y_pixels_per_meter, out);
    fputc((bmp_header->y_pixels_per_meter >> 8), out);
    fputc((bmp_header->y_pixels_per_meter >> 16), out);
    fputc((bmp_header->y_pixels_per_meter >> 24), out);
    fputc(bmp_header->colors_used, out);
    fputc((bmp_header->colors_used >> 8), out);
    fputc((bmp_header->colors_used >> 16), out);
    fputc((bmp_header->colors_used >> 24), out);
    fputc(bmp_header->colors_important, out);
    fputc((bmp_header->colors_important >> 8), out);
    fputc((bmp_header->colors_important >> 16), out);
    fputc((bmp_header->colors_important >> 24), out);

	/*
	 * ------------------- Color table -------------------------------------
	 * For 8-bit per pixel we have  256 shades of gray,so we need 256 values
	 * in color table to represent each of 256 shades of gray. Each shade is
	 * represented with 4 values (R-G-B-padding). For example:
	 *
	 *   (0,0,0,0)			- BLACK
	 *   (255,255,255,0)	- WHITE
	*/

	char padding = 0;
	for (int i = 0; i < 256; i++)
	{
		fputc(i, out); // R
		fputc(i, out); // G
		fputc(i, out); // B
		fputc(padding, out); // padding byte
	}

	int index = 0;
	int bytePerPixel    = (bmp_header->bits_per_pixel)/8;
	int unpaddedRowSize = bmp_header->width * bytePerPixel;
	int paddedRowSize   = (((bmp_header->width * bytePerPixel) + 3) / 4) * 4;
	int height          = bmp_header->height;

	for(int i = 0; i < height ; i++)
	{
		for (int j = 0; j < paddedRowSize; j++)
		{
			if (expected_true(j+3 <= unpaddedRowSize))
			{
		    	fputc(pixels[index++], out);
		    	j++;
		    	j++;
			}
			else
				fputc(0x00, out);
		}
	}
}

void emd(unsigned char *data, int N);
void fusion(bmp_header_t* bmp_header, int N);
void convolve(bmp_header_t* bmp_header, unsigned char* data, unsigned int* var);

int main(int argc, char *argv[])
{
	bmp_header_t bmp_header;

	FILE* in = fopen("img1.bmp", "r");
	read_image(&bmp_header, in, data1);
	fclose(in);

	in = fopen("img2.bmp", "r");
	read_image(&bmp_header, in, data2);
	fclose(in);

	emd(data1, (bmp_header.width)*(bmp_header.height)); // data1 je sada imf1
	emd(data2, (bmp_header.width)*(bmp_header.height)); // data2 je sada imf1

	fusion(&bmp_header, (bmp_header.width)*(bmp_header.height));

	int size =  (bmp_header.width)*(bmp_header.height);

	for(int i = 0; i < size; i++)
		data1[i] = weightsA[i]*data1[i] + (1 - weightsA[i]) * data2[i] ;

	FILE *out = fopen( "fused.bmp","w");
	write_image(&bmp_header, out, data1);
}

void emd(unsigned char *data, int N)
{
	// Pronalazak lokalnih ekstrema
    unsigned char isLocalMax;
    unsigned char isLocalMin;

    for (int i = N - 2; i >= 1; i--)
	{
       isLocalMax = (data[i] >= data[i - 1]) & (data[i] >= data[i + 1]);
       isLocalMin = (data[i] <= data[i - 1]) & (data[i] <= data[i + 1]);

       localMAXs[i] = data[i] * isLocalMax;
       localMINs[i] = data[i] * isLocalMin;
    }

    // Kubna interpolacija za glatke prelaze izmedju piksela
    int tStep = 10, index, j, k;
    unsigned int x0, x1, x2, x3, a0, a1, a2, a3;
    unsigned int y0, y1, y2, y3, b0, b1, b2, b3;

    for (int i = 0; i < N - 1; i++)
	{
    	j = i > 0 ? i - 1 : 0;
    	k = i + 2 < N ? i + 2 : i + 1;

        x0 = localMAXs[j];
        x1 = localMAXs[i];
        x2 = localMAXs[i + 1];
        x3 = localMAXs[k];

        y0 = localMINs[j];
        y1 = localMINs[i];
        y2 = localMINs[i + 1];
        y3 = localMINs[k];

        // Cubic interpolation coefficients
        a0 = x3 - x2 - x0 + x1;
        a1 = x0 - x1 - a0;
        a2 = x2 - x0;
        a3 = x1;

        b0 = y3 - y2 - y0 + y1;
        b1 = y0 - y1 - b0;
        b2 = y2 - y0;
        b3 = y1;

        for (int m = 0; m < tStep; m++)
        {
            unsigned fract t = urdivi(m , tStep);

            index = i * tStep + m;

            localMAXs[index] = muliur(a0 , t * t * t) + muliur(a1 , t * t) + muliur(a2 , t) + a3; // gornji omotac (interpolirani maksimumi)
            localMINs[index] = muliur(b0 , t * t * t) + muliur(b1 , t * t) + muliur(b2 , t) + b3; // donji  omotac (interpolirani minnimumi)
        }
    }

	for (int i = N-1; i >= 0; i--)
		data[i] = (unsigned char)(data[i] - ( (localMAXs[i] + localMINs[i]) >> 2 ) ); // IMF1
}

void convolve(bmp_header_t* bmp_header, unsigned char* data, unsigned int* var)
{
	unsigned int susjed1, susjed2, susjed3, susjed4, susjed5, susjed6, susjed7, susjed8, i = 0, sum = 0, sum_temp = 0, avg = 0;
	fract f = 0.1ur;

	int width  = bmp_header->width;
	int height = bmp_header->height;

	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			i = (y * width) + x;

			susjed1 = (y-1)*width + (x-1);
			susjed2 = (y-1)*width + (x);
			susjed3 = (y-1)*width + (x+1);

			susjed4 = (y*width) + (x-1);
			susjed5 = (y*width) + (x+1);

			susjed6 = (y+1)*width + (x-1);
			susjed7 = (y+1)*width + (x);
			susjed8 = (y+1)*width + (x+1);

	        sum   = data[susjed1] * kernel[0][0] + data[susjed2]  * kernel[0][1] + data[susjed3] * kernel[0][2] +
	        		data[susjed4] * kernel[1][0] + data[i]        * kernel[1][1] + data[susjed5] * kernel[1][2] +
					data[susjed6] * kernel[2][0] + data[susjed7]  * kernel[2][1] + data[susjed8] * kernel[2][2];

	        avg = muliur(sum, f);

	        sum_temp =  (data[susjed1] - avg)*(data[susjed1] - avg) +
		       	    	(data[susjed2] - avg)*(data[susjed2] - avg) +
						(data[susjed3] - avg)*(data[susjed3] - avg) +
						(data[susjed4] - avg)*(data[susjed4] - avg) +
						(data[i]       - avg)*(data[i]       - avg) +
						(data[susjed5] - avg)*(data[susjed5] - avg) +
						(data[susjed6] - avg)*(data[susjed6] - avg) +
						(data[susjed7] - avg)*(data[susjed7] - avg) +
						(data[susjed8] - avg)*(data[susjed8] - avg);

	        var[i] = muliur(sum_temp , f);
	    }
	}
}

void fusion(bmp_header_t* bmp_header, int N)
{
	convolve(bmp_header, data1, localMAXs); // data1 je imf1 , localMAXs je var1
	convolve(bmp_header, data2, localMINs); // data2 je imf1 , localMINs je var2

	// Racunanje tezinskih koeficijenata
	for (int i = N-1; i >= 0 ; i--)
	{
		int diff = localMAXs[i] - localMINs[i];
		weightsA[i] = 0 + ((diff - E) > 0);
    }
}

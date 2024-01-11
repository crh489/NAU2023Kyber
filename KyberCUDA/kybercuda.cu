
// @Author: Arpan Jati
// Adapted from NewHope Reference Codebase and Parallelized using CUDA
// Updated : August 2019
// -------------------------------------------------------------
// CODE FOR PERFORMANCE COMPARISON. NOT FOR ACTUAL DEPLOYMENT
// -------------------------------------------------------------

#include "main.h";
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rng.h"
#include "api.h"
#include "params.h"
#include "indcpa.h"
#include <chrono>
#include <iostream>
#define	MAX_MARKER_LEN		50
#define KAT_SUCCESS          0
#define KAT_FILE_OPEN_ERROR -1
#define KAT_DATA_ERROR      -3
#define KAT_CRYPTO_FAILURE  -4

#define cudaCheckErrors(msg) \
    do { \
        cudaError_t __err = cudaGetLastError(); \
        if (__err != cudaSuccess) { \
            fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
                msg, cudaGetErrorString(__err), \
                __FILE__, __LINE__); \
            fprintf(stderr, "*** FAILED - ABORTING\n"); \
            exit(1); \
        } \
    } while (0)

using namespace std;
// MAIN CPA 

void allocatePolySet(poly_set4* polySet)
{
	HANDLE_ERROR(cudaMalloc(&(polySet->a), sizeof(poly)));
	HANDLE_ERROR(cudaMalloc(&(polySet->b), sizeof(poly)));
	HANDLE_ERROR(cudaMalloc(&(polySet->c), sizeof(poly)));
	HANDLE_ERROR(cudaMalloc(&(polySet->d), sizeof(poly)));

	HANDLE_ERROR(cudaMalloc(&(polySet->av), sizeof(polyvec)));
	HANDLE_ERROR(cudaMalloc(&(polySet->bv), sizeof(polyvec)));
	HANDLE_ERROR(cudaMalloc(&(polySet->cv), sizeof(polyvec)));
	HANDLE_ERROR(cudaMalloc(&(polySet->dv), sizeof(polyvec)));
	HANDLE_ERROR(cudaMalloc(&(polySet->ev), sizeof(polyvec)));
	HANDLE_ERROR(cudaMalloc(&(polySet->fv), sizeof(polyvec)));

	HANDLE_ERROR(cudaMalloc(&(polySet->AV), sizeof(polyvec) * 4));

	HANDLE_ERROR(cudaMalloc(&(polySet->seed), (KYBER_SYMBYTES * 2) * N_TESTS));

	HANDLE_ERROR(cudaMalloc(&(polySet->large_buffer_a), LARGE_BUFFER_SZ * N_TESTS));
	HANDLE_ERROR(cudaMalloc(&(polySet->large_buffer_b), LARGE_BUFFER_SZ * N_TESTS));
}

void freePolySet(poly_set4* polySet)
{
	HANDLE_ERROR(cudaFree(polySet->a));
	HANDLE_ERROR(cudaFree(polySet->b));
	HANDLE_ERROR(cudaFree(polySet->c));
	HANDLE_ERROR(cudaFree(polySet->d));

	HANDLE_ERROR(cudaFree(polySet->av));
	HANDLE_ERROR(cudaFree(polySet->bv));
	HANDLE_ERROR(cudaFree(polySet->cv));
	HANDLE_ERROR(cudaFree(polySet->dv));
	HANDLE_ERROR(cudaFree(polySet->ev));
	HANDLE_ERROR(cudaFree(polySet->fv));

	HANDLE_ERROR(cudaFree(polySet->seed));

	HANDLE_ERROR(cudaFree(polySet->large_buffer_a));
	HANDLE_ERROR(cudaFree(polySet->large_buffer_b));
}

int COUNT = 1;

int kybercuda(int kyber_implementation, char* input_file_path, char* encryption_file_path, char* decryption_file_path, int mode, int numThreads)
{
	FILE* f_input;
	FILE* f_encryption;
	FILE* f_decryption;
	uint8_t header[54] = "";
	size_t cipherfile_size;
	uint8_t* cipherfile;
	uint8_t* cipherfile_d;
	long long fileSize, originalFileSize;
	unsigned char* pk_h_0;
	unsigned char* sk_h_0;
	unsigned char* coins_h_0;
	unsigned char* rng_buf_h_0;
	unsigned char* pk_d_0;
	unsigned char* sk_d_0;
	unsigned char* coins_d_0;
	unsigned char* rng_buf_d_0;
	uint8_t* image_d;
	uint8_t* dec_image_d;
	cudaEvent_t start, stop, start1, stop1, start2, stop2, start3, stop3;
	uint8_t* image;
	uint8_t* dec_image;
	/*

	Read Input File
	*/
	f_input = fopen(input_file_path, "rb");
	f_encryption = fopen(encryption_file_path, "wb+");
	f_decryption = fopen(decryption_file_path, "wb+");
	if (!f_input) {
		printf("Invalid file path, file might not exist: %s\nProgram End.\n", input_file_path);
		return 0;
	}
	_fseeki64(f_input, 0, SEEK_END);
	fileSize = _ftelli64(f_input);
	originalFileSize = fileSize;
	_fseeki64(f_input, 0, SEEK_SET);
	image = (uint8_t*)calloc(1, fileSize);
	dec_image = (uint8_t*)calloc(1, fileSize*2);
	fread(header, 1, 54, f_input);
	fwrite(header, 54, 1, f_encryption);
	fwrite(header, 54, 1, f_decryption);
	_fseeki64(f_input, 0, SEEK_SET);
	fread(image, 1, fileSize, f_input);
	_fseeki64(f_input, 0, SEEK_SET);
	cipherfile_size = fileSize * (KYBER_INDCPA_BYTES / KYBER_INDCPA_MSGBYTES);
	
	HANDLE_ERROR(cudaSetDevice(0));
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventCreate(&start1);
	cudaEventCreate(&stop1);
	cudaEventCreate(&start2);
	cudaEventCreate(&stop2);
	cudaEventCreate(&start3);
	cudaEventCreate(&stop3);
	HANDLE_ERROR(cudaHostAlloc((void**)&pk_h_0, KYBER_INDCPA_PUBLICKEYBYTES, cudaHostAllocDefault));
	HANDLE_ERROR(cudaHostAlloc((void**)&sk_h_0, KYBER_INDCPA_SECRETKEYBYTES, cudaHostAllocDefault));
	HANDLE_ERROR(cudaHostAlloc((void**)&coins_h_0, KYBER_SYMBYTES, cudaHostAllocDefault));
	HANDLE_ERROR(cudaHostAlloc((void**)&rng_buf_h_0, KYBER_SYMBYTES * 2, cudaHostAllocDefault));
	HANDLE_ERROR(cudaHostAlloc((void**)&cipherfile, cipherfile_size, cudaHostAllocDefault));
	HANDLE_ERROR(cudaMalloc((void**)&pk_d_0, KYBER_INDCPA_PUBLICKEYBYTES));
	HANDLE_ERROR(cudaMalloc((void**)&sk_d_0, KYBER_INDCPA_SECRETKEYBYTES));
	HANDLE_ERROR(cudaMalloc((void**)&coins_d_0, KYBER_SYMBYTES));
	HANDLE_ERROR(cudaMalloc((void**)&rng_buf_d_0, KYBER_SYMBYTES * 2));
	HANDLE_ERROR(cudaMalloc((void**)&image_d, fileSize));
	HANDLE_ERROR(cudaMalloc((void**)&dec_image_d, fileSize*2));
	HANDLE_ERROR(cudaMalloc((void**)&cipherfile_d, cipherfile_size));
	memset(coins_h_0, 0, KYBER_SYMBYTES);
	randombytes(rng_buf_h_0, KYBER_SYMBYTES * 2);
	cudaStream_t stream_0;
	HANDLE_ERROR(cudaStreamCreate(&stream_0));
	poly_set4 tempPoly_0;
	cudaEventRecord(start);
	HANDLE_ERROR(cudaMemcpyAsync(pk_d_0, pk_h_0, KYBER_INDCPA_PUBLICKEYBYTES, cudaMemcpyHostToDevice, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(sk_d_0, sk_h_0, KYBER_INDCPA_SECRETKEYBYTES, cudaMemcpyHostToDevice, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(coins_d_0, coins_h_0, KYBER_SYMBYTES, cudaMemcpyHostToDevice, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(rng_buf_d_0, rng_buf_h_0, KYBER_SYMBYTES * 2, cudaMemcpyHostToDevice, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(image_d, image, fileSize, cudaMemcpyHostToDevice, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(dec_image_d, dec_image, fileSize*2, cudaMemcpyHostToDevice, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(cipherfile_d, cipherfile, cipherfile_size, cudaMemcpyHostToDevice, stream_0));
	size_t fileSize_d = fileSize;
	size_t cipherfile_size_d = cipherfile_size;
	allocatePolySet(&tempPoly_0);
	indcpa_keypair(1, &tempPoly_0, pk_d_0, sk_d_0, rng_buf_d_0, stream_0);
	cudaEventRecord(stop);
	cudaEventSynchronize(stop);
	/*
	
	ENCRYPT
	
	*/
	cudaEventRecord(start1);
	for (long long i = 0; i < fileSize_d - 54; i += KYBER_INDCPA_MSGBYTES) {
		indcpa_enc(1, &tempPoly_0, &cipherfile_d[i * (KYBER_INDCPA_BYTES / KYBER_INDCPA_MSGBYTES)], &image_d[i+54], pk_d_0, coins_d_0, stream_0);
	}
	cudaEventRecord(stop1);
	cudaEventSynchronize(stop1);
	float ms = 0;
	cudaEventElapsedTime(&ms, start1, stop1);
	printf("\nEncryption Finished, %f\n\n", ms);
	/*
	
	DECRYPT
	
	*/
	cudaEventRecord(start2);
	for (long long i = 0; i < cipherfile_size_d; i += KYBER_INDCPA_BYTES) {
		indcpa_dec(1, &tempPoly_0, &dec_image_d[i / (KYBER_INDCPA_BYTES / KYBER_INDCPA_MSGBYTES)], &cipherfile_d[i], sk_d_0, stream_0);
	}
	cudaEventRecord(stop2);
	cudaEventSynchronize(stop2);
	ms = 0;
	cudaEventElapsedTime(&ms, start2, stop2);
	printf("\nDecryption Finished, %f (ms)\n\n", ms);
	cudaEventRecord(start3);
	HANDLE_ERROR(cudaMemcpyAsync(pk_h_0, pk_d_0, KYBER_INDCPA_PUBLICKEYBYTES, cudaMemcpyDeviceToHost, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(sk_h_0, sk_d_0, KYBER_INDCPA_SECRETKEYBYTES, cudaMemcpyDeviceToHost, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(coins_h_0, coins_d_0, KYBER_SYMBYTES, cudaMemcpyDeviceToHost, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(rng_buf_h_0, rng_buf_d_0, KYBER_SYMBYTES * 2, cudaMemcpyDeviceToHost, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(image, image_d, fileSize, cudaMemcpyDeviceToHost, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(dec_image, dec_image_d, fileSize*2, cudaMemcpyDeviceToHost, stream_0));
	HANDLE_ERROR(cudaMemcpyAsync(cipherfile, cipherfile_d, cipherfile_size, cudaMemcpyDeviceToHost, stream_0));
	cudaEventRecord(stop3);
	cudaEventSynchronize(stop3);
	HANDLE_ERROR(cudaStreamDestroy(stream_0));
	fwrite(cipherfile, cipherfile_size, 1, f_encryption);
	fclose(f_encryption);
	fclose(f_input);
	fwrite(dec_image, fileSize, 1, f_decryption);
	fclose(f_decryption);
	HANDLE_ERROR(cudaFree(pk_d_0));
	HANDLE_ERROR(cudaFree(sk_d_0));
	HANDLE_ERROR(cudaFree(coins_d_0));
	HANDLE_ERROR(cudaFree(rng_buf_d_0));
	HANDLE_ERROR(cudaFree(image_d));
	HANDLE_ERROR(cudaFree(dec_image_d));
	HANDLE_ERROR(cudaFree(cipherfile_d));
	HANDLE_ERROR(cudaFreeHost(pk_h_0));
	HANDLE_ERROR(cudaFreeHost(sk_h_0));
	HANDLE_ERROR(cudaFreeHost(coins_h_0));
	HANDLE_ERROR(cudaFreeHost(rng_buf_h_0));
	HANDLE_ERROR(cudaFreeHost(cipherfile));
	memset(image, 0, sizeof(*image));
	free(image);
	freePolySet(&tempPoly_0);
	HANDLE_ERROR(cudaDeviceSynchronize());
	return 0;
}
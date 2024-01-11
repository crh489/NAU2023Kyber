#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <oqs/oqs.h>
#include <oqs/kem_kyber.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <Windows.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <omp.h>
#pragma warning(disable : 4996)
#pragma warning(disable : 6386)
#pragma warning(disable : 6262)
#pragma warning(disable : 4996)

int crypt(int kyber_implementation, char* input_file_path, char* encryption_file_path, char* decryption_file_path, int mode, int numThreads, long double* difference_enc, long double* difference_dec) {
	// Initialize Kyber512 variables
	uint8_t public_key_512[OQS_KEM_kyber_512_length_public_key] = { "" };
	uint8_t secret_key_512[OQS_KEM_kyber_512_length_secret_key] = { "" };
	uint8_t ciphertext_512[OQS_KEM_kyber_512_length_ciphertext] = { "" };

	// Initialize Kyber768 variables
	uint8_t public_key_768[OQS_KEM_kyber_768_length_public_key] = { "" };
	uint8_t secret_key_768[OQS_KEM_kyber_768_length_secret_key] = { "" };
	uint8_t ciphertext_768[OQS_KEM_kyber_768_length_ciphertext] = { "" };

	// Initialize Kyber1024 variables
	uint8_t public_key_1024[OQS_KEM_kyber_1024_length_public_key] = { "" };
	uint8_t secret_key_1024[OQS_KEM_kyber_1024_length_secret_key] = { "" };
	uint8_t ciphertext_1024[OQS_KEM_kyber_1024_length_ciphertext] = { "" };

	// Initialize program variables

	FILE* f_input;
	FILE* f_encryption;
	FILE* f_decryption;
	uint8_t header[54] = "";
	int start_enc = 0, end_enc = 0;
	int start_dec = 0, end_dec = 0;
	long long fileSize, originalFileSize;
	long long i = 0;
	int k = 0;
	int j = 0;
	size_t cipherfile_512_size;
	size_t cipherfile_768_size;
	size_t cipherfile_1024_size;
	uint8_t* cipherfile_512;
	uint8_t* cipherfile_768;
	uint8_t* cipherfile_1024;
	uint8_t* cipherfile;
	/*

	Read Input File
	*/
	f_input = fopen(input_file_path, "rb");
	f_encryption = fopen(encryption_file_path, "wb+");
	if (!f_input) {
		printf("Invalid file path, file might not exist: %s\nProgram End.\n", input_file_path);
		return 0;
	}
	_fseeki64(f_input, 0, SEEK_END);
	fileSize = _ftelli64(f_input);
	originalFileSize = fileSize;
	_fseeki64(f_input, 0, SEEK_SET);
	uint8_t* image = (uint8_t*)calloc(1, fileSize);
	fread(header, 1, 54, f_input);
	fwrite(header, 54, 1, f_encryption);
	_fseeki64(f_input, 0, SEEK_SET);
	fread(image, 1, fileSize, f_input);
	_fseeki64(f_input, 0, SEEK_SET);

	/*

	Initialize cipherfile

	*/
	cipherfile_512_size = 24 * (fileSize);
	cipherfile_768_size = 34 * (fileSize);
	cipherfile_1024_size = 49 * (fileSize);
	cipherfile_512 = (uint8_t*)malloc(cipherfile_512_size);
	cipherfile_768 = (uint8_t*)malloc(cipherfile_768_size);
	cipherfile_1024 = (uint8_t*)malloc(cipherfile_1024_size);

	/*
	Keygen
	*/
	OQS_KEM_kyber_512_keypair(public_key_512, secret_key_512);

	OQS_KEM_kyber_768_keypair(public_key_768, secret_key_768);

	OQS_KEM_kyber_1024_keypair(public_key_1024, secret_key_1024);

	/*

	Encryption
	*/
	i = 0;
	if (mode == 1) {
		start_enc = clock();
		
		if (kyber_implementation == 1) {
			for (i = 54; i < fileSize; i += 32) {
				OQS_KEM_kyber_512_encrypt(&cipherfile_512[(i - 54) * 24], &image[i], public_key_512);
			}
		}
		else if (kyber_implementation == 2) {
			for (i = 54; i < fileSize; i += 32) {
				OQS_KEM_kyber_768_encrypt(&cipherfile_768[(i - 54) * 34], &image[i], public_key_768);
			}
		}
		else if (kyber_implementation == 3) {
			for (i = 54; i < fileSize; i += 32) {
				OQS_KEM_kyber_1024_encrypt(&cipherfile_1024[(i - 54) * 49], &image[i], public_key_1024);
			}
		}
		end_enc = clock();
	}
	else if (mode == 2) {
		start_enc = clock();
		
		
		if (kyber_implementation == 1) {
			omp_set_num_threads(numThreads);
			#pragma omp parallel for schedule(dynamic)
			for (i = 54; i < fileSize; i += 32) {
				OQS_KEM_kyber_512_encrypt(&cipherfile_512[(i - 54) * 24], &image[i], public_key_512);
			}
		}
		else if (kyber_implementation == 2) {
			omp_set_num_threads(numThreads);
			#pragma omp parallel for schedule(dynamic)
			for (i = 54; i < fileSize; i += 32) {
				OQS_KEM_kyber_768_encrypt(&cipherfile_768[(i - 54) * 34], &image[i], public_key_768);
			}
		}
		else if (kyber_implementation == 3) {
			omp_set_num_threads(numThreads);
			#pragma omp parallel for schedule(dynamic)
			for (i = 54; i < fileSize; i += 32) {
				OQS_KEM_kyber_1024_encrypt(&cipherfile_1024[(i-54) * 49], &image[i], public_key_1024);
			}
		}
		end_enc = clock();
	}

	/*
		Write Encryption to File
		*/
	if (kyber_implementation == 1) {
		fwrite(cipherfile_512, cipherfile_512_size, 1, f_encryption);
	}
	else if (kyber_implementation == 2) {
		fwrite(cipherfile_768, cipherfile_768_size, 1, f_encryption);
	}
	else if (kyber_implementation == 3) {
		fwrite(cipherfile_1024, cipherfile_1024_size, 1, f_encryption);
	}
	fclose(f_encryption);
	*difference_enc = (long double)(end_enc - start_enc) / CLOCKS_PER_SEC * 1000;
	printf("\nTime to encrypt file %s (ms): %5.0Lf", input_file_path, *difference_enc);

	fclose(f_input);

	/*
		Read encrypted file

	*/

	f_encryption = fopen(encryption_file_path, "rb");
	f_decryption = fopen(decryption_file_path, "wb+");
	if (!f_encryption) {
		printf("Invalid file path, file might not exist: %s\nProgram End.\n", encryption_file_path);
		return 0;
	}
	_fseeki64(f_encryption, 0, SEEK_END);
	fileSize = _ftelli64(f_encryption);
	cipherfile = (uint8_t*)calloc(1, fileSize*2);
	_fseeki64(f_encryption, 0, SEEK_SET);
	fread(cipherfile, 1, fileSize, f_encryption);
	_fseeki64(f_encryption, 0, SEEK_SET);
	
	

	/*

	Initialize decrypted image

	*/
	image = (uint8_t*)calloc(1, originalFileSize*2);
	for (k = 0; k < 54; k++) image[k] = header[k];
	i = 0;
	int increment_amount = 1;

	if (kyber_implementation == 1) {
		increment_amount = 768;
	}
	else if (kyber_implementation == 2) {
		increment_amount = 1088;
	}
	else if (kyber_implementation == 3) {
		increment_amount = 1568;
	}
	
	i = 54;
	start_dec = clock();
	if (kyber_implementation == 1) {
		omp_set_num_threads(numThreads);
		#pragma omp parallel for schedule(dynamic)
		for (i = 54; i < fileSize; i += increment_amount) {
			OQS_KEM_kyber_512_decrypt(&image[54 + (i - 54) / 24], &cipherfile[i], secret_key_512);
		}
	}
	else if (kyber_implementation == 2) {
		omp_set_num_threads(numThreads);
		#pragma omp parallel for schedule(dynamic)
		for (i = 54; i < fileSize; i += increment_amount) {
			OQS_KEM_kyber_768_decrypt(&image[54 + (i - 54) / 34], &cipherfile[i], secret_key_768);

		}
	}
	else if (kyber_implementation == 3) {
		omp_set_num_threads(numThreads);
		#pragma omp parallel for schedule(dynamic)
		for (i = 54; i < fileSize; i += increment_amount) {
			OQS_KEM_kyber_1024_decrypt(&image[54 + (i - 54) / 49], &cipherfile[i], secret_key_1024);
		}
	}

	end_dec = clock();
	/*
	
	Write Decryption to File

	*/
	fwrite(image, originalFileSize, 1, f_decryption);
	fclose(f_decryption);
	fclose(f_encryption);
	*difference_dec = (long double)(end_dec - start_dec) / CLOCKS_PER_SEC * 1000;
	printf("\nTime to decrypt file %s (ms): %5.0Lf\n", input_file_path, *difference_dec);


	/*

	Clear Memory

	
	*/
	memset(public_key_512, 0, sizeof(*public_key_512));
	memset(ciphertext_512, 0, sizeof(*ciphertext_512));
	memset(secret_key_512, 0, sizeof(*secret_key_512));
	memset(public_key_768, 0, sizeof(*public_key_768));
	memset(secret_key_768, 0, sizeof(*secret_key_768));
	memset(ciphertext_768, 0, sizeof(*ciphertext_768));
	memset(public_key_1024, 0, sizeof(*public_key_1024));
	memset(secret_key_1024, 0, sizeof(*secret_key_1024));
	memset(ciphertext_1024, 0, sizeof(*ciphertext_1024));

	memset(cipherfile_512, 0, sizeof(*cipherfile_512));
	memset(cipherfile_768, 0, sizeof(*cipherfile_768));
	memset(cipherfile_1024, 0, sizeof(*cipherfile_1024));
	free(cipherfile_512);
	free(cipherfile_768);
	free(cipherfile_1024);

	memset(image, 0, sizeof(*image));
	free(image);

	memset(cipherfile, 0, sizeof(*cipherfile));
	free(cipherfile);

	return 0;
}
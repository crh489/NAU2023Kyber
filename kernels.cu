//#include <oqs/oqs.h>
//#include <oqs/kem_kyber.h>
//#include <inttypes.h>
//#include "cuda_runtime.h"
//#include "device_launch_parameters.h"
//
//__global__ void encrypt512_kernel(long long fileSize, uint8_t* cipherfile_512, uint8_t image[], uint8_t* public_key_512)
//{
//	// thread handler - this currently doesn't actually affect loop
//	int index = blockIdx.x * blockDim.x + threadIdx.x;
//	int stride = blockDim.x * gridDim.x;
//
//
//	for (int i = 54; i < fileSize; i += 32) {
//	//OQS_KEM_kyber_512_encrypt((cipherfile_512 + (((i - 54) / 32) * 768)), &image[i], public_key_512);
//	 }
//
//
//
//}

# Facial Recognition with Post-Quantum Cryptography
## Background
This code repository is a fork of the code repository used for my senior year capstone project at Northern Arizona University, 2023. Our group's project sponsor was Professor Tuy Nguyen of the School of Informatics, Computing, and Cyber Systems. 

This project is a continuation of his work from his publication (Duong-Ngoc, P. Nguyen, T., & Lee, H. (2020). Efficient NewHope Cryptography Based Facial Security System on a GPU. IEEE Access, 8 108158-108168).

## Application Description
The two main functions of this Windows-based application are a facial recognition function for images files, video files, and live webcam video using the Open Computer Vision Library (OpenCV), and an encryption/decryption function for detected faces using the CRYSTALS-Kyber Quantum Safe algorithm. Applications of this tool can be for secure facial recognition systems such as referenced in https://www.mdpi.com/2079-9292/12/3/774.

Running the tool allows users to select which image or video to encrypt/decrypt, which implementation of Kyber to use (Kyber512, Kyber768, or Kyber1024), and which processing mode to use (CPU, CPU Multithreaded with a user-selected number of threads, and GPU via Nvidia-CUDA). The console output will display the time measured to both encrypt and decrypt the selected media and encrypted and decrypted images will be generated in the "Outputs" folder. Outputs from video inputs will generate an encrypted and decrypted image for each face detection for each frame of the video.

## Output Analysis
A MATLAB script accompanies this program in the "Image Analysis" folder. This script allows users to analyze the encrypted and decrypted images for security and performance metrics such as entropy of the plain/cipher images, entropy difference between plain/cipher images for each color channel (Red, Green, Blue), correlation coefficients between the plain/cipher images each color channel (Red, Green, Blue), NPCR score and distribution, UACI score and distribution. Histograms for the plain and cipher images will also be generated in the "./Outputs/Histograms/Plain" and "./Outputs/Histograms/Cipher" directories, respectively.

## Prototype
A zipped copy of the compiled prototype we used is included in "Prototype.7z". This archive contains Windows 10 (tested and verified) compatible executables that are ready to run using pre-selected media in the "Inputs" directory.

## Visual Studio Project Files
Project files are also included in this repository for future maintenance/updates/bug fixing.

## Project Poster
![Capstone Poster.png](https://github.com/crh489/NAU2023Kyber/blob/main/Capstone%20Poster.png?raw=true)
*Poster displaying project results.*

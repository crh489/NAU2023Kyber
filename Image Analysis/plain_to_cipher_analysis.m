clear;
clf;
clc;

% Read Plain Image and Cipher Image
outputDir = dir("../Output");
numOutputFolders = length(outputDir)-2;

for i = 1:numOutputFolders
    subFolder = outputDir(i+2);
    subFolderDir = dir("../Output/" + subFolder.name);
    
    histogramDir1Path = "../Output/" + subFolder.name + "/Histograms";
    histogramDir2Path = "../Output/" + subFolder.name + "/Histograms/Plain";
    histogramDir3Path = "../Output/" + subFolder.name + "/Histograms/Cipher";
    subFolderName = subFolder.name;
    if ~exist(histogramDir1Path, 'dir')
        mkdir(histogramDir2Path);
        mkdir(histogramDir3Path);
        fileID1 = fopen("../Output/" + subFolder.name + "/Results.csv", 'r');
        fileID2 = fopen("../Output/" + subFolder.name + "/AnalyzedResults.csv", 'w');
        csv1 = textscan(fileID1, '%s', 'Delimiter', '\n');
        csv1 = csv1{1,1};
        csv2 = {};
        fseek(fileID1,0,0);
        facesFolderDir = dir("../Output/" + subFolder.name + "/Faces");
        encryptionFolderDir = dir("../Output/" + subFolder.name + "/Encryption");
        decryptionFolderDir = dir("../Output/" + subFolder.name + "/Decryption");
        newHeader = csv1{1} + ",entropy_red,entropy_green,entropy_blue,entropy_cipher_red,entropy_cipher_green,entropy_cipher_blue," + ...
            "CorrCoef_Red,CorrCoef_Green,CorrCoef_Blue,npcr_score,npcr_pVal,uaci_score,uaci_pVal\n";
        numFaces = length(facesFolderDir) - 2;
        jValues = 0:numFaces-1;
        fprintf(fileID2, newHeader);
        fprintf("\nStart Analyzing folder:\n\t" + subFolderName + "\n");
        parfor jdx = 1:length(jValues)
            j = jValues(jdx);
            row = csv1{j+2};
            plainPath = "../Output/" + subFolderName + "/Faces/face" + j + ".bmp";
            cipherPath = "../Output/" + subFolderName + "/Encryption/face" + j + ".bmp";
            plain=imread(plainPath);
            cipher_image = imread(cipherPath);
            
            % Calculate NPCR and UACI values
            results = NPCR_and_UACI( plain, cipher_image, 0, 255 );
            
            % Extracting Color Channels
            Red_plain = plain(:,:,1);
            Green_plain = plain(:,:,2);
            Blue_plain = plain(:,:,3);
            Red_cipher=cipher_image(:,:,1);
            Green_cipher=cipher_image(:,:,2);
            Blue_cipher=cipher_image(:,:,3);
            
            % Entropy
            entropy_red = entropy(Red_plain);
            entropy_green = entropy(Green_plain);
            entropy_blue = entropy(Blue_plain);
            entropy_cipher_red = entropy(Red_cipher);
            entropy_cipher_green = entropy(Green_cipher);
            entropy_cipher_blue = entropy(Blue_cipher);
            
            % Correlation Coefficients
            R_red = corr2(Red_cipher,Red_plain);
            R_green = corr2(Green_cipher,Green_plain);
            R_blue = corr2(Blue_cipher,Blue_plain);
            
            % Histograms
            [yRed, x] = imhist(Red_plain);
            [yGreen, x] = imhist(Green_plain);
            [yBlue, x] = imhist(Blue_plain);
            [yRed_cipher, x] = imhist(Red_cipher);
            [yBlue_cipher, x] = imhist(Blue_cipher);
            [yGreen_cipher, x] = imhist(Green_cipher);
            
            % Print Correlation Coefficient
            %fprintf("Correlation Coefficient for red values in plain image vs cipher image: %3.4f\n",R_red);
            %fprintf("Correlation Coefficient for green values in plain image vs cipher image: %3.4f\n",R_green);
            %fprintf("Correlation Coefficient for blue values in plain image vs cipher image: %3.4f\n",R_blue);
            
            % Print Entropy
            %fprintf("Entropy of red channel (Plain Image): %3.4f\n", entropy_red);
            %fprintf("Entropy of green channel (Plain Image): %3.4f\n", entropy_green);
            %fprintf("Entropy of blue channel (Plain Image): %3.4f\n", entropy_blue);
            %fprintf("Entropy of red channel (Cipher Image): %3.4f\n", entropy_cipher_red);
            %fprintf("Entropy of green channel (Cipher Image): %3.4f\n", entropy_cipher_green);
            %fprintf("Entropy of blue channel (Cipher Image): %3.4f\n", entropy_cipher_blue);
            
            % Print Entropy Difference
            %fprintf("Entropy difference (Red Channel) %3.4f\n",entropy_red-entropy_cipher_red);
            %fprintf("Entropy difference (Green Channel) %3.4f\n",entropy_green-entropy_cipher_green);
            %fprintf("Entropy difference (Blue Channel) %3.4f\n",entropy_blue-entropy_cipher_blue);
            
            % Print NPCR
            %fprintf("NPCR score: %3.8f\n",results.npcr_score);
            %fprintf("NPCR pval: %3.8f\n",results.npcr_pVal);
            %fprintf("NPCR dist: [%d,%d]\n",results.npcr_dist(1),results.npcr_dist(2));
            
            % Print UACI
            %fprintf("UACI score: %3.8f\n",results.uaci_score);
            %fprintf("UACI pval: %3.8f\n",results.uaci_pVal);
            %fprintf("UACI dist: [%d,%d]\n",results.uaci_dist(1),results.uaci_dist(2));
            
            % Write new row to new CSV
            dataString = "," + entropy_red + "," + entropy_green + "," + entropy_blue + "," + entropy_cipher_red + "," + entropy_cipher_green + "," + entropy_cipher_blue + "," + R_red + "," + R_green + "," + R_blue + "," + results.npcr_score + "," + results.npcr_pVal + "," + results.uaci_score + "," + results.uaci_pVal + "\n";
            newRow = row + dataString;
            csv2 = [csv2,newRow];
            
            % Display Plain Image Histogram
            fig1 = figure('visible', 'off');
            bar(yRed, 'Red') ,hold on , bar(yGreen, 'Green'), hold on ,bar( yBlue, 'Blue');
            legend('Red Value','Green Value','Blue Value');
            xlabel("RGB Value (0-255)");
            ylabel("# Occurrences");
            xlim([0,255]);
            title("Plain Image Histogram");
            fig1Path = histogramDir2Path + "/face" + j + "_PlainImage_histogram";
            saveas(fig1, fig1Path, 'png');
            
            % Display Cipher Image Histogram
            fig2 = figure('visible', 'off');
            bar(yRed_cipher, 'Red') ,hold on , bar(yGreen_cipher, 'Green'), hold on ,bar( yBlue_cipher, 'Blue');
            legend('Red Value','Green Value','Blue Value');
            xlabel("RGB Value (0-255)");
            ylabel("# Occurrences");
            xlim([0,255]);
            title("Cipher Image Histogram");
            fig2Path = histogramDir3Path + "/face" + j + "_CipherImage_histogram";
            saveas(fig2, fig2Path, 'png');
        end
        for k = 1:length(csv2)
            fprintf(fileID2, csv2{k});
        end
        fclose(fileID1);
        fclose(fileID2);
        fprintf("\nFinished Analyzing folder:\n\t" + subFolderName + "\n");
    end
end

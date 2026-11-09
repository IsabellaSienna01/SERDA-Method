#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>

#define MAX_DIM 256

char inputName[256];
int row, col, matrix[MAX_DIM][MAX_DIM], s[MAX_DIM];
int orig_row, orig_col;
int orig_s[MAX_DIM], orig_d[MAX_DIM];
int TC[MAX_DIM] = {}, supply[MAX_DIM][MAX_DIM], sumRow[MAX_DIM] = {};
int status[MAX_DIM];
int benchmark_mode = 0;
long long final_tcc = 0;
int is_feasible = 1;
double elapsed_ms = 0.0;

void minimumCell() {
	int i, j, minimum;
	for(j = 0; j < col; j++){
		minimum = 0;
		for (i = 1; i < row; i++){
			if(matrix[minimum][j] > matrix[i][j]) minimum = i;
			else if(matrix[minimum][j] == matrix[i][j]) {
				if(TC[i] > TC[minimum]) minimum = i;
			}
		}
		supply[minimum][j] = orig_d[j];
	}
	
	for (i = 0; i < row; i++){
		sumRow[i] = 0;
		for (j = 0; j < col; j++)
			sumRow[i] += supply[i][j];
	}
}

int updateStatus(int *satisfied) {
	//1 = Satisfied
	//2 = needSupply
	//3 = Excess Row
	int i, nNeedSupply = 0, nExcess = 0;
	for (i = 0; i < row; i++){
		if (sumRow[i] == s[i]) {
			if(status[i] != 1) (*satisfied)--;
			status[i] = 1;
		} else if (sumRow[i] < s[i]) {
			status[i] = 2;
			nNeedSupply = 1;
		}
		else if (sumRow[i] > s[i]) {
			status[i] = 3;
			nExcess = 1;
		}
	}
	if (!nNeedSupply || !nExcess) return 0;
	return 1;
}

void findDonatur(int *donaturRow, int *receiverRow, int *donationCol, float *ratioflc, float *ratioslc) {
	int i, j, k, needSup[MAX_DIM] = {}, nNeedSup = 0;
	for (i = 0; i < row; i++){
		if(status[i] == 2) {	//2 = needSupply
			needSup[nNeedSup] = i;
			nNeedSup++;
		}
	}

	int temp = 0, smallestIndex = 0;
	static int localSmallest[MAX_DIM], localDon[MAX_DIM], localRec[MAX_DIM];
	int flag = 0;

	//Cari local-best donation
	for(j = 0; j < col; j++){
		localSmallest[j] = INT_MAX;
		
		//mencari semua kandidat donasi
		for(i = 0; i < row; i++){
			if(status[i] == 3 && supply[i][j]){		//3 = Excess Row
				for(k = 0; k < nNeedSup; k++){
					temp = matrix[i][j] - matrix[ needSup[k] ][j];
					if (temp < 0) temp *= -1;
					
					//Find local smallest
					if(localSmallest[j] > temp) {
						localSmallest[j] = temp;
						localDon[j] = i;
						localRec[j] = needSup[k];
					}
					else if (localSmallest[j] == temp){
						if( TC[needSup[k]] > TC[ localRec[j] ] ){
							localDon[j] = i;
							localRec[j] = needSup[k];
						}
					}
				}
			}
		}
		
		//mencari global smallest dan check flag
		if(localSmallest[smallestIndex] > localSmallest[j] || smallestIndex == j) {
			smallestIndex = j;
			flag = 0;
		} else if(localSmallest[smallestIndex] == localSmallest[j]) flag = 1;
	}

	// Calculate the ratios for FLC and SLC
	*ratioflc = (float)s[localDon[smallestIndex]] * matrix[localDon[smallestIndex]][smallestIndex];
	*ratioslc = (float)s[localRec[smallestIndex]] * matrix[localRec[smallestIndex]][smallestIndex];
	
	//donasi paling optimal
	if(!flag) {
		*donaturRow = localDon[smallestIndex];
		*receiverRow = localRec[smallestIndex];
		*donationCol = smallestIndex;
	} else {
		int donCapability1, donCapability2, maxDon1, maxDon2, priority1, priority2;
		for (j = smallestIndex + 1; j < col; j++){
			if(localSmallest[smallestIndex] == localSmallest[j]){
				//cari kapabilitas donasi kedua donatur
				donCapability1 = supply[ localDon[smallestIndex] ][smallestIndex];
				donCapability2 = supply[ localDon[j] ][j];
				
				//cari kapasitas donasi keduanya
				////cari prioritas satisfied 1
				if(*ratioflc > *ratioslc) 							//prioritas donatur satisfied
					priority1 = localDon[smallestIndex];
				else if (*ratioflc < *ratioslc) 					//prioritas receiver satisfied
					priority1 = localRec[smallestIndex];
				else if (localDon[smallestIndex] < localRec[smallestIndex])		//satisfied index terkecil
					priority1 = localDon[smallestIndex];
				else
					priority1 = localRec[smallestIndex];
				
				////cari kapasitas prioritas satisfied 1
				maxDon1 = s[ priority1 ] - sumRow[ priority1 ];
				if(maxDon1 < 0) maxDon1 *= -1;

				////cari prioritas satisfied 2
				if(*ratioflc > *ratioslc) 						//prioritas donatur satisfied
					priority2 = localDon[j];
				else if (*ratioflc < *ratioslc) 				//prioritas receiver satisfied
					priority2 = localRec[j];
				else if (localDon[j] < localRec[j])		//satisfied index terkecil
					priority2 = localDon[j];
				else
					priority2 = localRec[j];
					
				////cari kapasitas prioritas satisfied 2
				maxDon2 = s[ priority2 ] - sumRow[ priority2 ];
				if(maxDon2 < 0) maxDon1 *= -1; // <-- BUG ASLI DARI KODE KONSOL LAMA
				
				//tentukan jumlah donasi
				if(donCapability1 < maxDon1) maxDon1 = donCapability1;
				if(donCapability2 < maxDon2) maxDon2 = donCapability2;
				
				//bandingkan jumlah donasi
				if(maxDon1 < maxDon2) {
					*donaturRow = localDon[smallestIndex];
					*receiverRow = localRec[smallestIndex];
					*donationCol = smallestIndex;
				} 
				//jika maxDon sama
				else{
					//tentukan maxDon * cell
					int hasilKali1 = maxDon1 * matrix[ localDon[smallestIndex] ][smallestIndex];
					int hasilKali2 = maxDon2 * matrix[ localDon[j] ][j];
					
					if(hasilKali1 <= hasilKali2) {
						*donaturRow = localDon[smallestIndex];
						*receiverRow = localRec[smallestIndex];
						*donationCol = smallestIndex;
					}
					else{
						*donaturRow = localDon[j];
						*receiverRow = localRec[j];
						*donationCol = j;
						smallestIndex = j;
					}
				}
			}
		}
	}
}

void donate(int *donaturRow, int *receiverRow, int *donationCol, float ratioflc, float ratioslc) {
	int d = *donaturRow, r = *receiverRow, c = *donationCol;
	int priority1 = d, priority2 = r;
	
	if(ratioflc < ratioslc) {
		priority1 = r;
		priority2 = d;
	}

	int maxDon1 = s[ priority1 ] - sumRow[ priority1 ];
	int maxDon2 = s[ priority2 ] - sumRow[ priority2 ];
	
	if(maxDon1 < 0) maxDon1 *= -1;
	if(maxDon2 < 0) maxDon2 *= -1;
	
	int donor = maxDon1;
	if(maxDon1 > maxDon2) donor = maxDon2;
	if(donor > supply[d][c]) donor = supply[d][c];

	supply[d][c] -= donor;
	supply[r][c] += donor;
	sumRow[d] -= donor;
	sumRow[r] += donor;
}

void check(){
	if(benchmark_mode) return;
	int i, j;
	printf("Matrix \n");
	for (i = 0; i < row; i++){
		for(j = 0; j < col; j++){
			printf("%d [%d]\t", matrix[i][j], supply[i][j]);
		}
		printf("| %d\tSum: %d", s[i], sumRow[i]);
		if(status[i] == 1) printf(" (Satisfied)");
		else if(status[i] == 2) printf(" (Need Supply)");
		else if(status[i] == 3) printf(" (Excess Row)");
		printf("\n");
	}
	printf("\n");
}

void countTCC(){
	final_tcc = 0;
	int i, j;
	for (i = 0; i < row; i++){
		for(j = 0; j < col; j++){
			final_tcc += (long long)matrix[i][j] * supply[i][j];
		}
	}
	if(!benchmark_mode) printf("TCC = %lld\n", final_tcc);
}

void compute() {
	clock_t start_time = clock();
	FILE *inFile = fopen(inputName, "r");
	if(!inFile) {
		if(!benchmark_mode) printf("File %s Tidak Ditemukan\n", inputName);
		else printf("METHOD:RBSM,STATUS:ERROR,COST:0,FEASIBLE:0,TIME_MS:0.0\n");
		return;
	}
	
	fscanf(inFile,"%d %d", &row, &col);
	orig_row = row;
	orig_col = col;

	int i, j;
	for (i = 0; i < row; i++){
		TC[i] = 0;
		for(j = 0; j < col; j++){
			fscanf(inFile,"%d", &matrix[i][j]);
			supply[i][j] = 0;
			TC[i] += matrix[i][j];
		}
		status[i] = 0;
		sumRow[i] = 0;
	}

	int tot_s = 0;
	for (i = 0; i < row; i++) {
		fscanf(inFile,"%d", &s[i]);
		orig_s[i] = s[i];
		tot_s += s[i];
	}

	int tot_d = 0;
	for (j = 0; j < col; j++) {
		fscanf(inFile, "%d", &orig_d[j]);
		tot_d += orig_d[j];
	}
	fclose(inFile);

	// Handle balancing
	if (tot_s > tot_d) {
		int dummy_c = col;
		for (i = 0; i < row; i++) {
			matrix[i][dummy_c] = 0;
			supply[i][dummy_c] = 0;
		}
		orig_d[dummy_c] = tot_s - tot_d;
		col++;
	} else if (tot_d > tot_s) {
		int dummy_r = row;
		TC[dummy_r] = 0;
		for (j = 0; j < col; j++) {
			matrix[dummy_r][j] = 0;
			supply[dummy_r][j] = 0;
		}
		s[dummy_r] = tot_d - tot_s;
		status[dummy_r] = 0;
		sumRow[dummy_r] = 0;
		row++;
	}
	
	minimumCell();

	int satisfied = row, donaturRow, receiverRow, donationCol;
	float ratioflc, ratioslc;
	if(!updateStatus(&satisfied)) {
		if(!benchmark_mode) printf("Soal Salah\n");
		is_feasible = 0;
	} else {
		int iteration = 1;
		check();
		int max_iter = 2000;
		while(satisfied && max_iter-- > 0){
			if(!benchmark_mode) printf("=== Iterasi ke-%d ===\n", iteration++);
			donaturRow = -1; receiverRow = -1;
			findDonatur(&donaturRow, &receiverRow, &donationCol, &ratioflc, &ratioslc);	
			if (donaturRow == -1 || receiverRow == -1) break;
			donate(&donaturRow, &receiverRow, &donationCol, ratioflc, ratioslc);
			updateStatus(&satisfied);
			check();
		}
		countTCC();
	}

	// Feasibility check
	is_feasible = 1;
	for (i = 0; i < orig_row; i++) {
		int r_sum = 0;
		for (j = 0; j < col; j++) r_sum += supply[i][j];
		if (r_sum != orig_s[i]) is_feasible = 0;
	}
	for (j = 0; j < orig_col; j++) {
		int c_sum = 0;
		for (i = 0; i < row; i++) c_sum += supply[i][j];
		if (c_sum != orig_d[j]) is_feasible = 0;
	}

	clock_t end_time = clock();
	elapsed_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

	if (benchmark_mode) {
		printf("METHOD:RBSM,STATUS:%s,COST:%lld,FEASIBLE:%d,TIME_MS:%.4f\n",
		       is_feasible ? "SUCCESS" : "FAILED", final_tcc, is_feasible, elapsed_ms);
	}
}

void custom() {
	printf("Name = ");
	scanf("%s", inputName);
}

int main(int argc, char *argv[]){
	if (argc >= 2) {
		strncpy(inputName, argv[1], sizeof(inputName) - 1);
		if (argc >= 3 && strcmp(argv[2], "--benchmark") == 0) {
			benchmark_mode = 1;
		}
		compute();
		return 0;
	}

	int option = 0;
	printf("What is your input-file's name?\n1 = Default (input.txt)\n2 = Custom\n3 = Exit\n");
	printf("Option = ");
	if (scanf("%d", &option) != 1 || option == 3) return 0;
	do{
		if(option == 1) {strcpy(inputName, "input.txt"); compute();}
		else if (option == 2) {custom(); compute();}
		else if (option != 3) printf("Not Valid\n");
		printf("Option = ");
		if (scanf("%d", &option) != 1 || option == 3) break;
	} while(option != 3);
	printf("Program Stop\n\n");
	return 0;
}

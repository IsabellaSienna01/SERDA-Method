#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_S 256
#define MAX_D 256

int main(int argc, char *argv[]){
	int i,
		j,
		jumlah_supply,
		jumlah_demand,
		p,
		q,
		total_supply = 0,
		total_demand = 0;

	const char *filename = "N06.txt";
	int benchmark_mode = 0;

	if (argc >= 2) {
		filename = argv[1];
	}
	if (argc >= 3 && strcmp(argv[2], "--benchmark") == 0) {
		benchmark_mode = 1;
	}

	clock_t start_time = clock();
	FILE *in = fopen(filename, "r");
	if(!in){
		if(!benchmark_mode) printf("File %s tidak ditemukan!\n", filename);
		else printf("METHOD:SAM,STATUS:ERROR,COST:0,FEASIBLE:0,TIME_MS:0.0\n");
		return 1;
	}

	int coordinat;
	int jumlah_ER;
	int temp = 0;
	int coorselisih;
	long long total_alloc = 0;

	if(fscanf(in,"%d", &jumlah_supply) != 1 || fscanf(in,"%d", &jumlah_demand) != 1){
		fclose(in);
		return 1;
	}

	int orig_supply_count = jumlah_supply;
	int orig_demand_count = jumlah_demand;

	static int cost[MAX_S][MAX_D];
	static int alloc[MAX_S][MAX_D];
	static int allocation[MAX_S][MAX_D];
	static int total_alloc_baris[MAX_S];
	static int supply[MAX_S];
	static int demand[MAX_D];
	static int orig_supply[MAX_S];
	static int orig_demand[MAX_D];
	static int arr[MAX_S];
	static int satisfy[MAX_S];
	static double average_cost[MAX_S];
	static double average_cost_multiply_supply[MAX_S];

	static int flc[MAX_D];
	static int slc[MAX_D];
	static int diff[MAX_D];
	static int konflik[MAX_D];
	int itertemp = 0;

	// Input Cost
	for(i=0; i<jumlah_supply; i++){
		for(j=0; j<jumlah_demand; j++){
			fscanf(in,"%d", &cost[i][j]);
		}
	}

	// Input Supply
	for(i=0; i<jumlah_supply; i++){
		fscanf(in,"%d", &supply[i]);
		orig_supply[i] = supply[i];
		total_supply += supply[i];
	}

	// Input Demand
	for(i=0; i<jumlah_demand; i++){
		fscanf(in,"%d", &demand[i]);
		orig_demand[i] = demand[i];
		total_demand += demand[i];
	}

	fclose(in);

	// Balancing
	if(total_supply > total_demand){
		int dummy_col = jumlah_demand;
		for(i=0; i<jumlah_supply; i++) cost[i][dummy_col] = 0;
		demand[dummy_col] = total_supply - total_demand;
		jumlah_demand++;
	} else if(total_demand > total_supply){
		int dummy_row = jumlah_supply;
		for(j=0; j<jumlah_demand; j++) cost[dummy_row][j] = 0;
		supply[dummy_row] = total_demand - total_supply;
		jumlah_supply++;
	}

	for(i=0; i<jumlah_supply; i++){
		average_cost[i] = 0;
		for(j=0; j<jumlah_demand; j++){
			average_cost[i] += cost[i][j];
		}
		average_cost[i] = average_cost[i] / jumlah_demand;
		average_cost_multiply_supply[i] = average_cost[i] * supply[i];
		if(!benchmark_mode){
			printf("average_cost[%d] = %.2f\n", i, average_cost[i]);
			printf("average_cost_multiply_supply[%d] = %.2f\n", i, average_cost_multiply_supply[i]);
		}
	}

	// Set alokasi semua cell menjadi -1
	for(i=0; i<jumlah_supply; i++){
		for(j=0; j<jumlah_demand; j++){
			allocation[i][j] = -1;
			alloc[i][j] = 0;
		}
	}

	if(!benchmark_mode){
		printf("Jumlah Supply: %d\n", jumlah_supply);
		printf("Jumlah Demand: %d\n", jumlah_demand);

		printf("\n\n(Tabel Awal)");
		printf("\n--------------------------------------------\n");
		for(p=0; p<jumlah_supply; p++){
			for(q=0; q<jumlah_demand; q++){
				if(allocation[p][q] == -1){
					printf("%d\t", cost[p][q]);
				} else {
					printf("%d\t", allocation[p][q]);
				}
			}
			printf("| %d\n", supply[p]);
		}

		for(q=0; q<jumlah_demand; q++){
			printf("_______\t");
		}
		printf("\n");

		for(q=0; q<jumlah_demand; q++){
			printf("%d\t", demand[q]);
		}
		printf("\n--------------------------------------------\n\n");
	}

	// Cari cost terkecil pada tiap kolom
	for(p=0; p<jumlah_demand; p++){
		coordinat = 0;
		for(q=0; q<jumlah_supply; q++){
			if(cost[coordinat][p] > cost[q][p]){
				coordinat = q;
			} else if(cost[coordinat][p] == cost[q][p]){
				if(average_cost_multiply_supply[coordinat] < average_cost_multiply_supply[q]){
					coordinat = q;
					if(!benchmark_mode) printf("coordinat %d, %d\n", coordinat, p);
				}
			}
			if(!benchmark_mode) printf("%d\t", cost[q][p]);
		}
		if(!benchmark_mode) printf("lokasi min : %d,%d\n", coordinat, p);
		alloc[coordinat][p] = demand[p];
	}

	int failed_iteration = 0;

	while(1){
		if(!benchmark_mode) printf("ITER %d\n\n\n", itertemp);
		jumlah_ER = 0;

		for(p=0; p<jumlah_supply; p++){
			arr[p] = 0;
			satisfy[p] = 0;
			total_alloc_baris[p] = 0;

			for(q=0; q<jumlah_demand; q++){
				if(!benchmark_mode) printf("%d [%d]\t", cost[p][q], alloc[p][q]);
				total_alloc_baris[p] += alloc[p][q];
			}

			if(!benchmark_mode){
				printf("| %d\tT. Alloc : %d\tAC * S = %.2f\t", supply[p], total_alloc_baris[p], average_cost_multiply_supply[p]);
			}

			if(total_alloc_baris[p] > supply[p]){
				if(!benchmark_mode) printf("Status = ER\t");
				arr[p] = 1;
				jumlah_ER++;
			} else if(total_alloc_baris[p] == supply[p]){
				if(!benchmark_mode) printf("Status = SATISFY");
				satisfy[p] = 1;
			}
			if(!benchmark_mode) printf("\n");
		}
		if(!benchmark_mode) printf("\n");

		if(jumlah_ER == 0){
			if(!benchmark_mode) printf("Sudah habis\n");
			break;
		}

		itertemp++;
		if(itertemp >= 100){ // Batas iterasi wajar
			if(!benchmark_mode) printf("ITER LEWATTTTT\n");
			failed_iteration = 1;
			break;
		}

		for(q=0; q<jumlah_demand; q++){
			konflik[q] = 0;
			for(p=0; p<jumlah_supply; p++){
				if(alloc[p][q] > 0 && arr[p] == 1){
					konflik[q] = 1;
					break;
				}
			}
		}

		// Menghitung FLC
		for(q=0; q<jumlah_demand; q++){
			if(konflik[q] == 1){
				int iter = 0;
				for(p=0; p<jumlah_supply; p++){
					iter++;
					if(satisfy[p] != 1 && alloc[p][q] > 0){
						temp = cost[p][q];
						flc[q] = p;
						break;
					}
				}
				for(p=iter; p<jumlah_supply; p++){
					if(satisfy[p] != 1 && temp > cost[p][q] && alloc[p][q] > 0){
						temp = cost[p][q];
						flc[q] = p;
					}
				}
			}
		}

		// Menghitung SLC
		int has_valid_slc = 0;
		for(q=0; q<jumlah_demand; q++){
			if(konflik[q] == 1){
				int iter = 0;
				slc[q] = -1;
				for(p=0; p<jumlah_supply; p++){
					iter++;
					if(flc[q] != p && arr[p] != 1 && satisfy[p] != 1){
						temp = cost[p][q];
						slc[q] = p;
						has_valid_slc = 1;
						break;
					}
				}
				for(p=iter; p<jumlah_supply; p++){
					if(flc[q] != p && arr[p] != 1 && satisfy[p] != 1){
						if(temp > cost[p][q]){
							temp = cost[p][q];
							slc[q] = p;
						} else if(temp == cost[p][q] && slc[q] >= 0 && average_cost_multiply_supply[slc[q]] < average_cost_multiply_supply[p]){
							slc[q] = p;
						}
					}
				}
				if(!benchmark_mode && slc[q] >= 0) printf("FLC[%d] = %d, SLC[%d] = %d\n", q, flc[q], q, slc[q]);
			}
		}

		if(!has_valid_slc){
			if(!benchmark_mode) printf("Tidak ada SLC valid tersisa.\n");
			failed_iteration = 1;
			break;
		}

		// Menyimpan diff tiap kolom
		for(q=0; q<jumlah_demand; q++){
			if(konflik[q] == 1 && slc[q] >= 0){
				diff[q] = cost[slc[q]][q] - cost[flc[q]][q];
				if(!benchmark_mode) printf("diff[%d] = %d\n", q, diff[q]);
			}
		}

		coorselisih = -1;
		for(q=0; q<jumlah_demand; q++){
			if(konflik[q] == 1 && slc[q] >= 0){
				coorselisih = q;
				break;
			}
		}

		if(coorselisih == -1){
			failed_iteration = 1;
			break;
		}

		for(q=coorselisih+1; q<jumlah_demand; q++){
			if(konflik[q] == 1 && slc[q] >= 0){
				if(diff[coorselisih] == diff[q]){
					if(alloc[flc[coorselisih]][coorselisih] < alloc[flc[q]][q]){
						coorselisih = q;
					}
				} else if(diff[coorselisih] > diff[q]){
					coorselisih = q;
				}
			}
		}

		if(!benchmark_mode) printf("Selisih terkecil ada di kolom ke : %d\n", coorselisih);

		int baris_slc = slc[coorselisih];
		int baris_flc = flc[coorselisih];

		if(average_cost_multiply_supply[baris_flc] == average_cost_multiply_supply[baris_slc]){
			if(!benchmark_mode) printf("Sama");
			if(total_alloc_baris[baris_flc] - alloc[baris_flc][coorselisih] <= supply[baris_flc]){
				alloc[baris_flc][coorselisih] -= (total_alloc_baris[baris_flc] - supply[baris_flc]);
				alloc[baris_slc][coorselisih] += (total_alloc_baris[baris_flc] - supply[baris_flc]);
			} else if (total_alloc_baris[baris_slc] + alloc[baris_flc][coorselisih] >= supply[baris_slc]){
				alloc[baris_slc][coorselisih] += (supply[baris_slc] - total_alloc_baris[baris_slc]);
				alloc[baris_flc][coorselisih] -= (supply[baris_slc] - total_alloc_baris[baris_slc]);
			} else {
				alloc[baris_slc][coorselisih] += alloc[baris_flc][coorselisih];
				alloc[baris_flc][coorselisih] = 0;
			}
		} else if(jumlah_ER >= 2){
			if(average_cost_multiply_supply[baris_flc] > average_cost_multiply_supply[baris_slc]){
				if(!benchmark_mode) printf("satisfy supply baris index %d dulu", baris_flc);
				if(total_alloc_baris[baris_flc] - alloc[baris_flc][coorselisih] <= supply[baris_flc]){
					alloc[baris_flc][coorselisih] -= (total_alloc_baris[baris_flc] - supply[baris_flc]);
					alloc[baris_slc][coorselisih] += (total_alloc_baris[baris_flc] - supply[baris_flc]);
				} else {
					alloc[baris_slc][coorselisih] += alloc[baris_flc][coorselisih];
					alloc[baris_flc][coorselisih] = 0;
				}
			} else {
				if(!benchmark_mode) printf("satisfy supply baris index %d dulu - else", baris_slc);
				int alloc_sisa = supply[baris_slc] - total_alloc_baris[baris_slc];
				if(alloc_sisa >= alloc[baris_flc][coorselisih]){
					alloc[baris_slc][coorselisih] += alloc[baris_flc][coorselisih];
					alloc[baris_flc][coorselisih] = 0;
				} else {
					alloc[baris_slc][coorselisih] = alloc_sisa;
					alloc[baris_flc][coorselisih] -= alloc_sisa;
				}
			}
		} else {
			if(!benchmark_mode) printf("ER Lebih Kecil \n");
			if(average_cost_multiply_supply[baris_flc] < average_cost_multiply_supply[baris_slc]){
				if(!benchmark_mode) printf("satisfy supply baris index %d dulu", baris_flc);
				if(total_alloc_baris[baris_flc] - alloc[baris_flc][coorselisih] <= supply[baris_flc]){
					alloc[baris_flc][coorselisih] -= (total_alloc_baris[baris_flc] - supply[baris_flc]);
					alloc[baris_slc][coorselisih] += (total_alloc_baris[baris_flc] - supply[baris_flc]);
				} else {
					alloc[baris_slc][coorselisih] += alloc[baris_flc][coorselisih];
					alloc[baris_flc][coorselisih] = 0;
				}
			} else {
				if(!benchmark_mode) printf("satisfy supply baris index %d dulu - else", baris_slc);
				int alloc_sisa = supply[baris_slc] - total_alloc_baris[baris_slc];
				if(alloc_sisa >= alloc[baris_flc][coorselisih]){
					alloc[baris_slc][coorselisih] += alloc[baris_flc][coorselisih];
					alloc[baris_flc][coorselisih] = 0;
				} else {
					alloc[baris_slc][coorselisih] = alloc_sisa;
					alloc[baris_flc][coorselisih] -= alloc_sisa;
				}
			}
		}
	}

	// Hitung total biaya
	total_alloc = 0;
	for(p=0; p<jumlah_supply; p++){
		for(q=0; q<jumlah_demand; q++){
			total_alloc += (long long)cost[p][q] * alloc[p][q];
		}
	}

	// Feasibility check
	int feasible = 1;
	if (failed_iteration) feasible = 0;
	for(p=0; p<jumlah_supply; p++){
		int r_sum = 0;
		for(q=0; q<jumlah_demand; q++) r_sum += alloc[p][q];
		int exp_s = (p < orig_supply_count) ? orig_supply[p] : (total_demand - total_supply);
		if(r_sum != exp_s) feasible = 0;
	}
	for(q=0; q<jumlah_demand; q++){
		int c_sum = 0;
		for(p=0; p<jumlah_supply; p++) c_sum += alloc[p][q];
		int exp_d = (q < orig_demand_count) ? orig_demand[q] : (total_supply - total_demand);
		if(c_sum != exp_d) feasible = 0;
	}

	clock_t end_time = clock();
	double elapsed_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

	if(benchmark_mode){
		printf("METHOD:SAM,STATUS:%s,COST:%lld,FEASIBLE:%d,TIME_MS:%.4f\n",
		       feasible ? "SUCCESS" : "FAILED", total_alloc, feasible, elapsed_ms);
	} else {
		printf("\n\n");
		for(p=0; p<jumlah_supply; p++){
			total_alloc_baris[p] = 0;
			for(q=0; q<jumlah_demand; q++){
				printf("%d [%d]\t", cost[p][q], alloc[p][q]);
				total_alloc_baris[p] += alloc[p][q];
			}
			printf("| %d\tTotal alloc : %d\n", supply[p], total_alloc_baris[p]);
		}
		printf("\nJumlah iterasi yang dijalankan: %d\n", itertemp);
		printf("Total Alokasi tabel adalah: %lld\n", total_alloc);
	}

	return 0;
}

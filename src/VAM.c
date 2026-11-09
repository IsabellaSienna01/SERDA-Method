#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define MAX 256

int N_ROWS, N_COLS;
int orig_rows, orig_cols;
int costs[MAX][MAX];
int supply[MAX], demand[MAX];
int orig_supply[MAX], orig_demand[MAX];
int results[MAX][MAX];
int row_done[MAX], col_done[MAX];

/* Fungsi untuk mencari penalti (selisih dua cost terkecil) */
void calculate_diff(int index, int len, int is_row, int res[3]) {
    int min1 = INT_MAX, min2 = INT_MAX, min_pos = -1;
    int active_count = 0;
    int i, c;

    for (i = 0; i < len; i++) {
        if (is_row ? col_done[i] : row_done[i]) continue;
        
        c = is_row ? costs[index][i] : costs[i][index];
        active_count++;

        if (c < min1) {
            min2 = min1;
            min1 = c;
            min_pos = i;
        } else if (c < min2) {
            min2 = c;
        }
    }

    if (active_count == 0) {
        res[0] = -1; 
    } else if (active_count == 1) {
        res[0] = min1; 
    } else {
        res[0] = min2 - min1; 
    }
    res[1] = min1;   
    res[2] = min_pos; 
}

void get_next_cell(int cell[4]) {
    int max_pen = -1;
    int res[3];
    int i, j;

    /* Cek penalti baris */
    for (i = 0; i < N_ROWS; i++) {
        if (row_done[i]) continue;
        calculate_diff(i, N_COLS, TRUE, res);
        if (res[0] > max_pen) {
            max_pen = res[0];
            cell[0] = i;      
            cell[1] = res[2]; 
            cell[2] = res[1]; 
            cell[3] = max_pen;
        }
    }

    /* Cek penalti kolom */
    for (j = 0; j < N_COLS; j++) {
        if (col_done[j]) continue;
        calculate_diff(j, N_ROWS, FALSE, res);
        if (res[0] > max_pen) {
            max_pen = res[0];
            cell[0] = res[2]; 
            cell[1] = j;      
            cell[2] = res[1]; 
            cell[3] = max_pen;
        }
    }
}

int main(int argc, char *argv[]) {
    int total_supply = 0, total_demand = 0;
    int i, j, r, c, q;
    int supply_left = 0;
    int cell[4]; /* 0:row, 1:col, 2:cost, 3:penalty */
    int iteration_count = 0;
    long long total_cost = 0;
    
    const char *filename = "N32.txt";
    int benchmark_mode = 0;

    if (argc >= 2) {
        filename = argv[1];
    }
    if (argc >= 3 && strcmp(argv[2], "--benchmark") == 0) {
        benchmark_mode = 1;
    }

    clock_t start_time = clock();
    FILE *file = fopen(filename, "r");
    
    if (!file) {
        if (!benchmark_mode) {
            printf("File %s tidak ditemukan!\n", filename);
        } else {
            printf("METHOD:VAM,STATUS:ERROR,COST:0,FEASIBLE:0,TIME_MS:0.0\n");
        }
        return 1;
    }

    if (fscanf(file, "%d %d", &N_ROWS, &N_COLS) != 2) {
        fclose(file);
        return 1;
    }

    orig_rows = N_ROWS;
    orig_cols = N_COLS;

    for (i = 0; i < N_ROWS; i++)
        for (j = 0; j < N_COLS; j++)
            fscanf(file, "%d", &costs[i][j]);

    for (i = 0; i < N_ROWS; i++) {
        fscanf(file, "%d", &supply[i]);
        orig_supply[i] = supply[i];
        total_supply += supply[i];
    }
    for (i = 0; i < N_COLS; i++) {
        fscanf(file, "%d", &demand[i]);
        orig_demand[i] = demand[i];
        total_demand += demand[i];
    }
    fclose(file);

    memset(results, 0, sizeof(results));

    /* Penanganan Unbalanced */
    if (total_supply > total_demand) {
        for (i = 0; i < N_ROWS; i++) costs[i][N_COLS] = 0;
        demand[N_COLS] = total_supply - total_demand;
        N_COLS++;
    } else if (total_demand > total_supply) {
        for (j = 0; j < N_COLS; j++) costs[N_ROWS][j] = 0;
        supply[N_ROWS] = total_demand - total_supply;
        N_ROWS++;
    }

    for (i = 0; i < N_ROWS; i++) {
        supply_left += supply[i];
        row_done[i] = FALSE;
    }
    for (j = 0; j < N_COLS; j++) col_done[j] = FALSE;

    if (!benchmark_mode) {
        printf("=== PROSES ITERASI VAM ===\n");
    }

    while (supply_left > 0) {
        iteration_count++;
        get_next_cell(cell);
        r = cell[0];
        c = cell[1];
        
        q = (supply[r] < demand[c]) ? supply[r] : demand[c];
        
        if (!benchmark_mode) {
            /* Menampilkan detail tiap langkah */
            printf("Iterasi ke-%d:\n", iteration_count);
            printf("- Penalti Tertinggi: %d\n", cell[3]);
            printf("- Pilih Sel [%d, %d] dengan Biaya: %d\n", r+1, c+1, cell[2]);
            printf("- Alokasi: %d unit\n", q);
            printf("- Sisa Stok Baris %d: %d\n", r+1, supply[r] - q);
            printf("- Sisa Permintaan Kolom %d: %d\n\n", c+1, demand[c] - q);
        }

        results[r][c] = q;
        supply[r] -= q;
        demand[c] -= q;
        supply_left -= q;
        total_cost += (long long)q * costs[r][c];

        if (supply[r] == 0) row_done[r] = TRUE;
        if (demand[c] == 0) col_done[c] = TRUE;
    }

    /* Feasibility check */
    int feasible = 1;
    for (i = 0; i < N_ROWS; i++) {
        int r_sum = 0;
        for (j = 0; j < N_COLS; j++) r_sum += results[i][j];
        int exp_s = (i < orig_rows) ? orig_supply[i] : (total_demand - total_supply);
        if (r_sum != exp_s) feasible = 0;
    }
    for (j = 0; j < N_COLS; j++) {
        int c_sum = 0;
        for (i = 0; i < N_ROWS; i++) c_sum += results[i][j];
        int exp_d = (j < orig_cols) ? orig_demand[j] : (total_supply - total_demand);
        if (c_sum != exp_d) feasible = 0;
    }

    clock_t end_time = clock();
    double elapsed_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

    if (benchmark_mode) {
        printf("METHOD:VAM,STATUS:SUCCESS,COST:%lld,FEASIBLE:%d,TIME_MS:%.4f\n",
               total_cost, feasible, elapsed_ms);
    } else {
        printf("=== HASIL AKHIR ===\n");
        printf("Total Iterasi (FNI): %d\n", iteration_count);
        
        printf("\nMatriks Alokasi:\n");
        for (i = 0; i < N_ROWS; i++) {
            for (j = 0; j < N_COLS; j++) printf("%d\t", results[i][j]);
            printf("\n");
        }
        printf("\nTotal Transportation Cost = %lld\n", total_cost);
    }

    return 0;
}

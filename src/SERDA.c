#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>

#define EPSILON 1e-9

typedef struct {
    int m;
    int n;
    int orig_m;
    int orig_n;
    int alloc_rows;
    int alloc_cols;
    double **cost;
    double *supply;
    double *demand;
    double *orig_supply;
    double *orig_demand;
} TPProblem;

typedef struct {
    int alloc_rows;
    int alloc_cols;
    double **alloc;
    double total_cost;
    int feasible;
    double elapsed_time_ms;
} TPSolution;

TPProblem* create_problem(int m, int n) {
    TPProblem *p = (TPProblem*)calloc(1, sizeof(TPProblem));
    p->m = m;
    p->n = n;
    p->orig_m = m;
    p->orig_n = n;
    int max_m = m + 2;
    int max_n = n + 2;
    p->alloc_rows = max_m;
    p->alloc_cols = max_n;
    p->cost = (double**)malloc(max_m * sizeof(double*));
    for (int i = 0; i < max_m; i++) {
        p->cost[i] = (double*)calloc(max_n, sizeof(double));
    }
    p->supply = (double*)calloc(max_m, sizeof(double));
    p->demand = (double*)calloc(max_n, sizeof(double));
    p->orig_supply = (double*)calloc(max_m, sizeof(double));
    p->orig_demand = (double*)calloc(max_n, sizeof(double));
    return p;
}

void free_problem(TPProblem *p) {
    if (!p) return;
    for (int i = 0; i < p->alloc_rows; i++) {
        free(p->cost[i]);
    }
    free(p->cost);
    free(p->supply);
    free(p->demand);
    free(p->orig_supply);
    free(p->orig_demand);
    free(p);
}

TPSolution* create_solution(int m, int n) {
    TPSolution *s = (TPSolution*)calloc(1, sizeof(TPSolution));
    s->alloc_rows = m;
    s->alloc_cols = n;
    s->alloc = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++) {
        s->alloc[i] = (double*)calloc(n, sizeof(double));
    }
    return s;
}

void free_solution(TPSolution *s) {
    if (!s) return;
    for (int i = 0; i < s->alloc_rows; i++) {
        free(s->alloc[i]);
    }
    free(s->alloc);
    free(s);
}

/* Balance the transportation problem by adding dummy row or column with cost 0 */
void balance_problem(TPProblem *p) {
    double tot_s = 0.0, tot_d = 0.0;
    for (int i = 0; i < p->m; i++) tot_s += p->supply[i];
    for (int j = 0; j < p->n; j++) tot_d += p->demand[j];

    p->orig_m = p->m;
    p->orig_n = p->n;

    if (tot_s < tot_d - EPSILON) {
        int dummy_r = p->m;
        for (int j = 0; j < p->n; j++) {
            p->cost[dummy_r][j] = 0.0;
        }
        p->supply[dummy_r] = tot_d - tot_s;
        p->m++;
    } else if (tot_s > tot_d + EPSILON) {
        int dummy_c = p->n;
        for (int i = 0; i < p->m; i++) {
            p->cost[i][dummy_c] = 0.0;
        }
        p->demand[dummy_c] = tot_s - tot_d;
        p->n++;
    }
}

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

typedef struct {
    int is_row;     /* 1 if row, 0 if col */
    int idx;        /* row or col index */
    double penalty; /* second smallest - smallest */
    double min_cr;  /* smallest CR */
} LineCandidate;

/* Tie-breaking comparator for candidate lines in SERDA:
 * 1. Penalty terbesar (descending)
 * 2. Min CR terkecil (ascending)
 * 3. Row sebelum col (is_row = 1 before 0)
 * 4. Indeks terkecil (ascending)
 */
static int cmp_line_candidates(const void *a, const void *b) {
    const LineCandidate *la = (const LineCandidate *)a;
    const LineCandidate *lb = (const LineCandidate *)b;

    if (fabs(la->penalty - lb->penalty) > EPSILON) {
        return (la->penalty > lb->penalty) ? -1 : 1;
    }
    if (fabs(la->min_cr - lb->min_cr) > EPSILON) {
        return (la->min_cr < lb->min_cr) ? -1 : 1;
    }
    if (la->is_row != lb->is_row) {
        return la->is_row ? -1 : 1;
    }
    return (la->idx < lb->idx) ? -1 : (la->idx > lb->idx ? 1 : 0);
}

TPSolution* solve_SERDA(TPProblem *prob, int verbose) {
    clock_t start_time = clock();

    balance_problem(prob);

    int m = prob->m;
    int n = prob->n;
    TPSolution *sol = create_solution(m, n);

    double *s = (double*)malloc(m * sizeof(double));
    double *d = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < m; i++) s[i] = prob->supply[i];
    for (int j = 0; j < n; j++) d[j] = prob->demand[j];

    int iteration = 0;

    if (verbose) {
        printf("============================================================\n");
        printf("               METODE SERDA / DMO-VAM                       \n");
        printf("============================================================\n");
        printf("Dimensi: %d sumber x %d tujuan\n", m, n);
    }

    int *active_r = (int*)malloc(m * sizeof(int));
    int *active_c = (int*)malloc(n * sizeof(int));
    double *row_mean = (double*)malloc(m * sizeof(double));
    double *col_mean = (double*)malloc(n * sizeof(double));
    double *cr_vals = (double*)malloc((m > n ? m : n) * sizeof(double));
    LineCandidate *lines = (LineCandidate*)malloc((m + n) * sizeof(LineCandidate));

    while (1) {
        int n_active_r = 0;
        int n_active_c = 0;

        for (int i = 0; i < m; i++) {
            if (s[i] > EPSILON) active_r[n_active_r++] = i;
        }
        for (int j = 0; j < n; j++) {
            if (d[j] > EPSILON) active_c[n_active_c++] = j;
        }

        if (n_active_r == 0 || n_active_c == 0) break;

        iteration++;

        /* 1. Calculate active averages */
        for (int ri = 0; ri < n_active_r; ri++) {
            int i = active_r[ri];
            double sum = 0.0;
            for (int cj = 0; cj < n_active_c; cj++) {
                int j = active_c[cj];
                sum += prob->cost[i][j];
            }
            row_mean[i] = sum / (double)n_active_c;
        }

        for (int cj = 0; cj < n_active_c; cj++) {
            int j = active_c[cj];
            double sum = 0.0;
            for (int ri = 0; ri < n_active_r; ri++) {
                int i = active_r[ri];
                sum += prob->cost[i][j];
            }
            col_mean[j] = sum / (double)n_active_r;
        }

        /* 2. Build candidate lines */
        int n_lines = 0;

        /* Row candidates */
        for (int ri = 0; ri < n_active_r; ri++) {
            int i = active_r[ri];
            for (int cj = 0; cj < n_active_c; cj++) {
                int j = active_c[cj];
                cr_vals[cj] = prob->cost[i][j] - row_mean[i] - col_mean[j];
            }
            qsort(cr_vals, n_active_c, sizeof(double), cmp_double);
            double penalty = (n_active_c >= 2) ? (cr_vals[1] - cr_vals[0]) : 0.0;
            lines[n_lines].is_row = 1;
            lines[n_lines].idx = i;
            lines[n_lines].penalty = penalty;
            lines[n_lines].min_cr = cr_vals[0];
            n_lines++;
        }

        /* Column candidates */
        for (int cj = 0; cj < n_active_c; cj++) {
            int j = active_c[cj];
            for (int ri = 0; ri < n_active_r; ri++) {
                int i = active_r[ri];
                cr_vals[ri] = prob->cost[i][j] - row_mean[i] - col_mean[j];
            }
            qsort(cr_vals, n_active_r, sizeof(double), cmp_double);
            double penalty = (n_active_r >= 2) ? (cr_vals[1] - cr_vals[0]) : 0.0;
            lines[n_lines].is_row = 0;
            lines[n_lines].idx = j;
            lines[n_lines].penalty = penalty;
            lines[n_lines].min_cr = cr_vals[0];
            n_lines++;
        }

        /* 3. Sort lines by tie-breaking rule */
        qsort(lines, n_lines, sizeof(LineCandidate), cmp_line_candidates);
        LineCandidate chosen_line = lines[0];

        /* 4. Cell selection in chosen line */
        int sel_i = -1, sel_j = -1;
        if (chosen_line.is_row) {
            sel_i = chosen_line.idx;
            double best_cr = DBL_MAX;
            double best_cost = DBL_MAX;
            for (int cj = 0; cj < n_active_c; cj++) {
                int j = active_c[cj];
                double cr = prob->cost[sel_i][j] - row_mean[sel_i] - col_mean[j];
                double c = prob->cost[sel_i][j];
                int is_better = 0;
                if (cr < best_cr - EPSILON) {
                    is_better = 1;
                } else if (fabs(cr - best_cr) <= EPSILON) {
                    if (c < best_cost - EPSILON) {
                        is_better = 1;
                    } else if (fabs(c - best_cost) <= EPSILON) {
                        if (sel_j == -1 || j < sel_j) is_better = 1;
                    }
                }
                if (is_better) {
                    best_cr = cr;
                    best_cost = c;
                    sel_j = j;
                }
            }
        } else {
            sel_j = chosen_line.idx;
            double best_cr = DBL_MAX;
            double best_cost = DBL_MAX;
            for (int ri = 0; ri < n_active_r; ri++) {
                int i = active_r[ri];
                double cr = prob->cost[i][sel_j] - row_mean[i] - col_mean[sel_j];
                double c = prob->cost[i][sel_j];
                int is_better = 0;
                if (cr < best_cr - EPSILON) {
                    is_better = 1;
                } else if (fabs(cr - best_cr) <= EPSILON) {
                    if (c < best_cost - EPSILON) {
                        is_better = 1;
                    } else if (fabs(c - best_cost) <= EPSILON) {
                        if (sel_i == -1 || i < sel_i) is_better = 1;
                    }
                }
                if (is_better) {
                    best_cr = cr;
                    best_cost = c;
                    sel_i = i;
                }
            }
        }

        /* 5. Allocate */
        double q = (s[sel_i] < d[sel_j]) ? s[sel_i] : d[sel_j];
        sol->alloc[sel_i][sel_j] += q;
        s[sel_i] -= q;
        d[sel_j] -= q;

        if (verbose) {
            printf("Iterasi %d:\n", iteration);
            printf("  Garis terpilih: %s %d (Penalti = %.4f, Min CR = %.4f)\n",
                   chosen_line.is_row ? "Baris" : "Kolom", chosen_line.idx + 1,
                   chosen_line.penalty, chosen_line.min_cr);
            printf("  Sel terpilih  : [%d, %d] dengan Biaya Asli: %.2f\n",
                   sel_i + 1, sel_j + 1, prob->cost[sel_i][sel_j]);
            printf("  Alokasi       : %.2f unit\n", q);
            printf("  Sisa Supply %d: %.2f, Sisa Demand %d: %.2f\n\n",
                   sel_i + 1, s[sel_i], sel_j + 1, d[sel_j]);
        }
    }

    free(active_r);
    free(active_c);
    free(row_mean);
    free(col_mean);
    free(cr_vals);
    free(lines);
    free(s);
    free(d);

    /* 6. Compute total cost with cost matrix */
    sol->total_cost = 0.0;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            sol->total_cost += sol->alloc[i][j] * prob->cost[i][j];
        }
    }

    /* 7. Verify feasibility */
    sol->feasible = 1;
    for (int i = 0; i < m; i++) {
        double row_sum = 0.0;
        for (int j = 0; j < n; j++) row_sum += sol->alloc[i][j];
        if (fabs(row_sum - prob->supply[i]) > 1e-4) sol->feasible = 0;
    }
    for (int j = 0; j < n; j++) {
        double col_sum = 0.0;
        for (int i = 0; i < m; i++) col_sum += sol->alloc[i][j];
        if (fabs(col_sum - prob->demand[j]) > 1e-4) sol->feasible = 0;
    }

    clock_t end_time = clock();
    sol->elapsed_time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

    if (verbose) {
        printf("======================= HASIL AKHIR =======================\n");
        printf("Status Feasibility : %s\n", sol->feasible ? "FEASIBLE" : "INFEASIBLE");
        printf("Matriks Alokasi:\n");
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                printf("%8.1f ", sol->alloc[i][j]);
            }
            printf("\n");
        }
        printf("Total Transportation Cost = %.2f\n", sol->total_cost);
        printf("Waktu Eksekusi            = %.4f ms\n", sol->elapsed_time_ms);
        printf("===========================================================\n");
    }

    return sol;
}

TPProblem* read_problem_from_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;

    int m, n;
    if (fscanf(f, "%d %d", &m, &n) != 2) {
        fclose(f);
        return NULL;
    }

    TPProblem *p = create_problem(m, n);

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            if (fscanf(f, "%lf", &p->cost[i][j]) != 1) {
                free_problem(p);
                fclose(f);
                return NULL;
            }
        }
    }

    for (int i = 0; i < m; i++) {
        if (fscanf(f, "%lf", &p->supply[i]) != 1) {
            free_problem(p);
            fclose(f);
            return NULL;
        }
        p->orig_supply[i] = p->supply[i];
    }

    for (int j = 0; j < n; j++) {
        if (fscanf(f, "%lf", &p->demand[j]) != 1) {
            free_problem(p);
            fclose(f);
            return NULL;
        }
        p->orig_demand[j] = p->demand[j];
    }

    fclose(f);
    return p;
}

#ifndef BENCHMARK_LIB
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Penggunaan: %s <file_testcase.txt> [--benchmark]\n", argv[0]);
        return 1;
    }

    const char *filepath = argv[1];
    int benchmark_mode = (argc >= 3 && strcmp(argv[2], "--benchmark") == 0);

    TPProblem *prob = read_problem_from_file(filepath);
    if (!prob) {
        fprintf(stderr, "Error: Tidak dapat membaca file %s\n", filepath);
        return 1;
    }

    TPSolution *sol = solve_SERDA(prob, !benchmark_mode);

    if (benchmark_mode) {
        printf("METHOD:SERDA,STATUS:SUCCESS,COST:%.2f,FEASIBLE:%d,TIME_MS:%.4f\n",
               sol->total_cost, sol->feasible, sol->elapsed_time_ms);
    }

    int ret = sol->feasible ? 0 : 2;
    free_solution(sol);
    free_problem(prob);
    return ret;
}
#endif

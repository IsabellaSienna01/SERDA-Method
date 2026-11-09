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

typedef struct {
    int is_row;
    int idx;
    double penalty;
    int ci;
    int cj;
    double cost;
    double q;
    double CA;
} THPCandidate;

/* Sort candidates by CA (asc), then cost (asc), then ci (asc), then cj (asc) */
static int cmp_thp_candidates(const void *a, const void *b) {
    const THPCandidate *ca = (const THPCandidate *)a;
    const THPCandidate *cb = (const THPCandidate *)b;

    if (fabs(ca->CA - cb->CA) > EPSILON) {
        return (ca->CA < cb->CA) ? -1 : 1;
    }
    if (fabs(ca->cost - cb->cost) > EPSILON) {
        return (ca->cost < cb->cost) ? -1 : 1;
    }
    if (ca->ci != cb->ci) {
        return (ca->ci < cb->ci) ? -1 : 1;
    }
    return (ca->cj < cb->cj) ? -1 : (ca->cj > cb->cj ? 1 : 0);
}

/* Compare helper for double descending sort */
static int cmp_double_desc(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da > db) return -1;
    if (da < db) return 1;
    return 0;
}

TPSolution* solve_THP(TPProblem *prob, int verbose) {
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
        printf("               METODE THP (Two Highest Penalties)           \n");
        printf("============================================================\n");
        printf("Dimensi: %d sumber x %d tujuan\n", m, n);
    }

    int *active_r = (int*)malloc(m * sizeof(int));
    int *active_c = (int*)malloc(n * sizeof(int));
    THPCandidate *all_penalties = (THPCandidate*)malloc((m + n) * sizeof(THPCandidate));
    THPCandidate *filtered_cands = (THPCandidate*)malloc((m + n) * sizeof(THPCandidate));
    double *distinct_p = (double*)malloc((m + n) * sizeof(double));

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

        int n_penalties = 0;

        /* Row penalties */
        for (int ri = 0; ri < n_active_r; ri++) {
            int i = active_r[ri];
            double min_val = DBL_MAX;
            double max_val = -DBL_MAX;
            int j_cand = -1;

            for (int cj = 0; cj < n_active_c; cj++) {
                int j = active_c[cj];
                double c = prob->cost[i][j];
                if (c > max_val) max_val = c;
                if (c < min_val) {
                    min_val = c;
                    j_cand = j;
                }
            }

            all_penalties[n_penalties].is_row = 1;
            all_penalties[n_penalties].idx = i;
            all_penalties[n_penalties].penalty = max_val - min_val;
            all_penalties[n_penalties].ci = i;
            all_penalties[n_penalties].cj = j_cand;
            all_penalties[n_penalties].cost = min_val;
            n_penalties++;
        }

        /* Column penalties */
        for (int cj = 0; cj < n_active_c; cj++) {
            int j = active_c[cj];
            double min_val = DBL_MAX;
            double max_val = -DBL_MAX;
            int i_cand = -1;

            for (int ri = 0; ri < n_active_r; ri++) {
                int i = active_r[ri];
                double c = prob->cost[i][j];
                if (c > max_val) max_val = c;
                if (c < min_val) {
                    min_val = c;
                    i_cand = i;
                }
            }

            all_penalties[n_penalties].is_row = 0;
            all_penalties[n_penalties].idx = j;
            all_penalties[n_penalties].penalty = max_val - min_val;
            all_penalties[n_penalties].ci = i_cand;
            all_penalties[n_penalties].cj = j;
            all_penalties[n_penalties].cost = min_val;
            n_penalties++;
        }

        /* Find distinct penalties, sort descending */
        int n_distinct = 0;
        for (int k = 0; k < n_penalties; k++) {
            double p_val = all_penalties[k].penalty;
            int found = 0;
            for (int d_idx = 0; d_idx < n_distinct; d_idx++) {
                if (fabs(distinct_p[d_idx] - p_val) <= EPSILON) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                distinct_p[n_distinct++] = p_val;
            }
        }
        qsort(distinct_p, n_distinct, sizeof(double), cmp_double_desc);

        /* Top 2 distinct levels */
        double top1 = distinct_p[0];
        double top2 = (n_distinct >= 2) ? distinct_p[1] : distinct_p[0];

        /* Filter candidates in top 2 levels */
        int n_filtered = 0;
        for (int k = 0; k < n_penalties; k++) {
            double p_val = all_penalties[k].penalty;
            if (fabs(p_val - top1) <= EPSILON || (n_distinct >= 2 && fabs(p_val - top2) <= EPSILON)) {
                filtered_cands[n_filtered] = all_penalties[k];
                int ci = filtered_cands[n_filtered].ci;
                int cj = filtered_cands[n_filtered].cj;
                double q_val = (s[ci] < d[cj]) ? s[ci] : d[cj];
                filtered_cands[n_filtered].q = q_val;
                filtered_cands[n_filtered].CA = prob->cost[ci][cj] * q_val;
                n_filtered++;
            }
        }

        /* Sort candidates by CA, cost, ci, cj */
        qsort(filtered_cands, n_filtered, sizeof(THPCandidate), cmp_thp_candidates);

        THPCandidate chosen = filtered_cands[0];
        int sel_i = chosen.ci;
        int sel_j = chosen.cj;
        double q = (s[sel_i] < d[sel_j]) ? s[sel_i] : d[sel_j];

        sol->alloc[sel_i][sel_j] += q;
        s[sel_i] -= q;
        d[sel_j] -= q;

        if (verbose) {
            printf("Iterasi %d:\n", iteration);
            printf("  Top penalties : %.2f, %.2f\n", top1, top2);
            printf("  Kandidat terpilih: %s %d -> Sel [%d, %d]\n",
                   chosen.is_row ? "Baris" : "Kolom", chosen.idx + 1,
                   sel_i + 1, sel_j + 1);
            printf("  CA = %.2f (Biaya = %.2f x Alokasi = %.2f)\n",
                   chosen.CA, chosen.cost, q);
            printf("  Sisa Supply %d: %.2f, Sisa Demand %d: %.2f\n\n",
                   sel_i + 1, s[sel_i], sel_j + 1, d[sel_j]);
        }
    }

    free(active_r);
    free(active_c);
    free(all_penalties);
    free(filtered_cands);
    free(distinct_p);
    free(s);
    free(d);

    /* Total cost */
    sol->total_cost = 0.0;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            sol->total_cost += sol->alloc[i][j] * prob->cost[i][j];
        }
    }

    /* Feasibility check */
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

    TPSolution *sol = solve_THP(prob, !benchmark_mode);

    if (benchmark_mode) {
        printf("METHOD:THP,STATUS:SUCCESS,COST:%.2f,FEASIBLE:%d,TIME_MS:%.4f\n",
               sol->total_cost, sol->feasible, sol->elapsed_time_ms);
    }

    int ret = sol->feasible ? 0 : 2;
    free_solution(sol);
    free_problem(prob);
    return ret;
}
#endif
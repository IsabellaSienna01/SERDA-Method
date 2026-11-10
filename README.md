# SERDA Method: Dynamic Mean-Opportunity Vogel's Approximation Method

> SERDA Method - Kelompok 5 - Riset Operasi (T)

  | Nama     | NRP      |
  |----------|----------|
  | Erica Triana Widyastuti | 5025241069 |
  | Dilbina Windi Azahra | 5025241180 |
  | Isabella Sienna Sulisthio | 5025241199 |
  | Nasyita Larasati Ertyananda | 5025241201 |
  | Rahma Sakinah | 5025241222 |

Repository ini berisi implementasi dari beberapa metode penyelesaian **Transportation Problem (TP)** dalam menentukan **Initial Basic Feasible Solution (IBFS)**, serta sistem pengujian otomatis (*automated benchmarking*) untuk mengevaluasi kinerja setiap metode. 

Fokus utama repository ini adalah membandingkan metode **VAM Standar (Vogel's Approximation Method)** dengan pengembangannya, yaitu **SERDA Method (Dynamic Mean-Opportunity Vogel's Approximation Method / DMO-VAM)**. Selain kedua metode tersebut, pengujian juga melibatkan 6 metode IBFS lainnya. Seluruh metode dievaluasi menggunakan **69 testcase standar** dengan membandingkan hasilnya terhadap solusi optimal eksak yang telah diverifikasi secara independen.

---

## Daftar Isi
- [Perbandingan Algoritma VAM Standar vs SERDA Method](#perbandingan-algoritma-vam-standar-vs-serda-method)
  - [1. Langkah Algoritma VAM Standar](#1-langkah-algoritma-vam-standar)
  - [2. Langkah Algoritma SERDA Method](#2-langkah-algoritma-serda-method)
  - [3. Perbandingan Pseudocode VAM Standar dan SERDA Method](#3-perbandingan-pseudocode-vam-standar-dan-serda-method)
  - [4. Implementasi Kode Inti `(src/SERDA.c)`](#4-implementasi-kode-inti-srcserdac)
  - [5. Tracing Eksekusi Program Step-by-Step](#5-tracing-eksekusi-program-step-by-step)
    - [5.1 Tracing Kasus Standar: `G1_N01.txt`](#51-tracing-kasus-standar-g1_n01txt)
    - [5.2 Tracing Kasus Perangkap Biaya VAM: `G3_N35.txt`](#52-tracing-kasus-perangkap-biaya-vam-g3_n35txt)
    - [5.3 Tracing Kasus Penalti Kolom Dini: `G2_N14.txt`](#53-tracing-kasus-penalti-kolom-dini-g2_n14txt)
- [Panduan Teknis Penggunaan Repositori](#panduan-teknis-penggunaan-repositori)
  - [6. Ringkasan 8 Metode yang Dibandingkan](#6-ringkasan-8-metode-yang-dibandingkan)
  - [7. Struktur Direktori](#7-struktur-direktori)
  - [8. Prasyarat Sistem & Kompilasi (`make all`, `make clean`)](#8-prasyarat-sistem--kompilasi)
  - [9. Menjalankan Automated Benchmark](#9-menjalankan-automated-benchmark)
  - [10. Menjalankan Program Secara Mandiri](#10-menjalankan-program-secara-mandiri)
  - [11. Format Input & Output](#11-format-input--output)
  - [12. Formula dan Metrik Evaluasi](#12-formula-dan-metrik-evaluasi)
- [Hasil Pengujian & Analisis Komparasi](#hasil-pengujian--analisis-komparasi)
  - [13. Ringkasan Hasil Benchmark (69 Testcase)](#13-ringkasan-hasil-benchmark-69-testcase)
  - [14. Tabel Perbandingan Lengkap: VAM vs SERDA vs Optimal (69 TC)](#14-tabel-perbandingan-lengkap-vam-vs-serda-vs-optimal-69-tc)
  - [15. Kesimpulan Komparasi VAM vs SERDA](#15-kesimpulan-komparasi-vam-vs-serda)

---

# Perbandingan Algoritma VAM Standar vs SERDA Method

## 1. Langkah Algoritma VAM Standar

Algoritma *Vogel's Approximation Method* (VAM) klasik bekerja berdasarkan prinsip penalti (*opportunity cost*) yang dihitung dari selisih dua elemen biaya terkecil pada setiap baris dan kolom:

1. **Inisialisasi & Penyeimbangan Masalah (Balancing)**:
   - Hitung total suplai $\sum S_i$ dan total permintaan $\sum D_j$.
   - Jika $\sum S_i > \sum D_j$, tambahkan kolom *dummy* dengan biaya 0 dan permintaan sebesar $\sum S_i - \sum D_j$.
   - Jika $\sum D_j > \sum S_i$, tambahkan baris *dummy* dengan biaya 0 dan suplai sebesar $\sum D_j - \sum S_i$.
2. **Hitung Penalti Tiap Baris dan Kolom Aktif**:
   - Untuk setiap baris aktif $i$, cari biaya terendah pertama ($c^{(1)}_i$) dan kedua ($c^{(2)}_i$):
     $$\text{Penalti Baris } P_i = c^{(2)}_i - c^{(1)}_i$$
   - Untuk setiap kolom aktif $j$, cari biaya terendah pertama ($c^{(1)}_j$) dan kedua ($c^{(2)}_j$):
     $$\text{Penalti Kolom } Q_j = c^{(2)}_j - c^{(1)}_j$$
3. **Pilih Garis dengan Penalti Terbesar**:
   - Tentukan baris atau kolom dengan nilai penalti maksimum: $\max(P_i, Q_j)$.
   - Aturan *tie-breaking*: Jika terdapat penalti bernilai sama, implementasi standar memilih kandidat baris/kolom pertama yang ditemukan.
4. **Pilih Sel Biaya Minimum pada Garis Terpilih**:
   - Pada baris/kolom terpilih tersebut, pilih sel $(i, j)$ dengan biaya transportasi asli terkecil:
     $$\min C_{ij}$$
5. **Alokasikan Kuantitas Transportasi**:
   - Hitung nilai alokasi:
     $$q = \min(S_i, D_j)$$
   - Kurangi suplai dan permintaan: $S_i \leftarrow S_i - q$, $D_j \leftarrow D_j - q$.
   - Jika $S_i = 0$, baris $i$ dieliminasi dari iterasi berikutnya.
   - Jika $D_j = 0$, kolom $j$ dieliminasi dari iterasi berikutnya.
6. **Iterasi**:
   - Ulangi langkah 2 sampai 5 hingga seluruh kapasitas suplai dan permintaan habis teralokasi.

### Flowchart Algoritma VAM Standar
Berikut adalah diagram alir eksekusi VAM standar:

![Flowchart Algoritma VAM Standar](img/flowchart-vam.png)

---

## 2. Langkah Algoritma SERDA Method

*Dynamic Mean-Opportunity Vogel's Approximation Method* (SERDA) memodifikasi perhitungan penalti dan seleksi sel menggunakan deviasi terhadap rata-rata baris dan kolom aktif (*Dynamic Mean Opportunity*) serta menerapkan hierarki *tie-breaking* deterministik:

1. **Inisialisasi & Penyeimbangan Masalah**:
   - Periksa total suplai dan permintaan. Tambahkan baris/kolom *dummy* berbiaya 0 bila masalah tidak seimbang.
2. **Hitung Rata-rata Biaya Baris & Kolom Aktif Tiap Iterasi**:
   - Rata-rata baris aktif ($i \in N'$):
     $$AVG_i = \frac{1}{|M'|} \sum_{j \in M'} C_{ij}$$
   - Rata-rata kolom aktif ($j \in M'$):
     $$AVG_j = \frac{1}{|N'|} \sum_{i \in N'} C_{ij}$$
     *(di mana $N'$ dan $M'$ adalah himpunan baris dan kolom yang masih memiliki sisa alokasi).*
3. **Transformasi Matriks Biaya Relatif ($CR_{ij}$)**:
   - Hitung nilai penyesuaian biaya relatif untuk setiap sel aktif $(i, j)$:
     $$CR_{ij} = C_{ij} - AVG_i - AVG_j$$
4. **Hitung Penalti Baris & Kolom berbasis $CR$**:
   - Cari dua nilai $CR$ terkecil pada setiap baris aktif dan kolom aktif.
   - Penalti baris: $P_i = CR^{(2)}_i - CR^{(1)}_i$.
   - Penalti kolom: $Q_j = CR^{(2)}_j - CR^{(1)}_j$.
5. **Seleksi Garis Terpilih (Tie-Breaking 4 Tingkat)**:
   - Evaluasi seluruh baris dan kolom aktif menggunakan kriteria terurut berikut:
     1. Prioritas 1: **Penalti Terbesar** ($\max \text{Penalty}$).
     2. Prioritas 2 (*Tie*): **Nilai Min $CR$ Terkecil** ($\min CR$, ascending).
     3. Prioritas 3 (*Tie*): **Baris diprioritaskan dibanding Kolom** (`is_row == 1` mendahului `is_row == 0`).
     4. Prioritas 4 (*Tie*): **Indeks terkecil** (ascending index).
6. **Seleksi Sel pada Garis Terpilih (Tie-Breaking 3 Tingkat)**:
   - Pada baris atau kolom yang terpilih, tentukan sel $(i, j)$ berdasarkan kriteria:
     1. Prioritas 1: **Nilai $CR_{ij}$ terkecil**.
     2. Prioritas 2 (*Tie*): **Biaya Transportasi Asli $C_{ij}$ terkecil**.
     3. Prioritas 3 (*Tie*): **Indeks terkecil**.
7. **Alokasi & Pembaruan Sisa Kapasitas**:
   - Alokasikan $q = \min(S_i, D_j)$ pada sel terpilih.
   - Perbarui sisa stok $S_i \leftarrow S_i - q$ dan sisa permintaan $D_j \leftarrow D_j - q$.
   - Eliminasi baris jika $S_i = 0$ dan eliminasi kolom jika $D_j = 0$.
   - Ulangi langkah 2–7 hingga seluruh permintaan terpenuhi.

### Flowchart Algoritma SERDA Method
Berikut adalah diagram alir eksekusi SERDA Method (DMO-VAM):

![Flowchart Algoritma SERDA Method](img/flowchart-serda.png)

---

## 3. Perbandingan Pseudocode VAM Standar dan SERDA Method

Tabel berikut merangkum perbedaan prinsip komputasi antara VAM standar dan SERDA Method:

| Aspek Algoritma | VAM Standar | SERDA Method (DMO-VAM) |
| :--- | :--- | :--- |
| **Metrik Biaya Penalti** | Biaya Transportasi Asli ($c_{(2)} - c_{(1)}$) | Biaya Relatif Dinamis ($CR^{(2)} - CR^{(1)}$) |
| **Perhitungan Deviasi Rata-rata** | Tidak ada | $AVG_i$ (baris aktif) dan $AVG_j$ (kolom aktif) dihitung tiap iterasi |
| **Matriks Biaya yang Digunakan** | Matriks biaya asli $C_{ij}$ | Matriks peluang biaya relatif $CR_{ij} = C_{ij} - AVG_i - AVG_j$ |
| **Aturan Tie-Breaking Garis** | Arbitrary / Garis pertama ditemukan | 4 Tingkat: Penalti $\to$ Min $CR$ $\to$ Baris $\to$ Indeks terkecil |
| **Aturan Tie-Breaking Sel** | Minimum biaya asli $C_{ij}$ | 3 Tingkat: Min $CR$ $\to$ Min biaya asli $C_{ij}$ $\to$ Indeks terkecil |

### 3.1 Pseudocode VAM Standar
```text
function solve_VAM(C, supply, demand):
  balance_problem(C, supply, demand)
  
  while supply_left > 0:
    // 1. Hitung Penalti Biaya Asli
    for each active row i:
      P[i] = second_min(C[i]) - min(C[i])
    for each active col j:
      Q[j] = second_min(C[:][j]) - min(C[:][j])
    
    // 2. Pilih Penalti Terbesar
    line = max(P, Q)
    
    // 3. Pilih Sel Biaya Minimum
    if line is row:
      j = argmin_{j in active_cols}(C[line][j])
      sel = (line, j)
    else:
      i = argmin_{i in active_rows}(C[i][line])
      sel = (i, line)
    
    // 4. Alokasikan Unit
    q = min(supply[sel.i], demand[sel.j])
    alloc[sel.i][sel.j] += q
    supply[sel.i] -= q
    demand[sel.j] -= q
    eliminate_zero_lines()

  return alloc, total_cost
```

### 3.2 Pseudocode SERDA Method (DMO-VAM)
```text
function solve_SERDA(C, supply, demand):
  balance_problem(C, supply, demand)
  
  while supply_left > 0:
    // 1. Hitung Rata-rata Baris & Kolom Aktif
    for each active row i:
      row_mean[i] = mean(C[i][active_cols])
    for each active col j:
      col_mean[j] = mean(C[active_rows][j])
      
    // 2. Transformasi CR & Penalti CR
    for each active row i:
      CR[i][*] = C[i][*] - row_mean[i] - col_mean[*]
      P[i] = second_min(CR[i]) - min(CR[i])
      min_cr_row[i] = min(CR[i])
    for each active col j:
      CR[*][j] = C[*][j] - row_mean[*] - col_mean[j]
      Q[j] = second_min(CR[:][j]) - min(CR[:][j])
      min_cr_col[j] = min(CR[:][j])
      
    // 3. Seleksi Garis Terbaik (Tie-break 4 Tingkat)
    line = select_best_line(
             P, Q, min_cr, priority:
             1. Max Penalty
             2. Min CR terkecil
             3. Baris sebelum kolom
             4. Indeks terkecil
           )
           
    // 4. Seleksi Sel Terbaik (Tie-break 3 Tingkat)
    sel = select_best_cell(
            line, priority:
            1. Min CR
            2. Min Original Cost C[i][j]
            3. Indeks terkecil
          )
          
    // 5. Alokasikan Unit
    q = min(supply[sel.i], demand[sel.j])
    alloc[sel.i][sel.j] += q
    supply[sel.i] -= q
    demand[sel.j] -= q
    eliminate_zero_lines()

  return alloc, total_cost
```

---

## 4. Implementasi Kode Inti `(src/SERDA.c)`

Logika algoritma SERDA diimplementasikan dalam berkas [`src/SERDA.c`](src/SERDA.c). Berikut adalah komponen fungsi utamanya:

### A. Struktur Data Kandidat Garis & Comparator Tie-Breaking
```c
typedef struct {
    int is_row;     /* 1 jika baris, 0 jika kolom */
    int idx;        /* indeks baris atau kolom */
    double penalty; /* selisih elemen CR terkecil kedua - terkecil pertama */
    double min_cr;  /* nilai CR terkecil pada garis */
} LineCandidate;

/* Comparator pembanding garis kandidat pada SERDA:
 * 1. Penalti terbesar (descending)
 * 2. Min CR terkecil (ascending)
 * 3. Baris sebelum kolom (is_row = 1 sebelum 0)
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
```

### B. Perhitungan Rata-rata Aktif dan Nilai $CR_{ij}$
```c
/* 1. Menghitung rata-rata baris dan kolom aktif */
for (int ri = 0; ri < n_active_r; ri++) {
    int i = active_r[ri];
    double sum = 0.0;
    for (int cj = 0; cj < n_active_c; cj++) {
        sum += prob->cost[i][active_c[cj]];
    }
    row_mean[i] = sum / (double)n_active_c;
}

for (int cj = 0; cj < n_active_c; cj++) {
    int j = active_c[cj];
    double sum = 0.0;
    for (int ri = 0; ri < n_active_r; ri++) {
        sum += prob->cost[active_r[ri]][j];
    }
    col_mean[j] = sum / (double)n_active_r;
}

/* 2. Menghitung matriks CR dan menentukan penalti baris */
for (int ri = 0; ri < n_active_r; ri++) {
    int i = active_r[ri];
    for (int cj = 0; cj < n_active_c; cj++) {
        int j = active_c[cj];
        cr_vals[cj] = prob->cost[i][j] - row_mean[i] - col_mean[j];
    }
    qsort(cr_vals, n_active_c, sizeof(double), cmp_double);
    lines[n_lines].is_row = 1;
    lines[n_lines].idx = i;
    lines[n_lines].penalty = (n_active_c >= 2) ? (cr_vals[1] - cr_vals[0]) : 0.0;
    lines[n_lines].min_cr = cr_vals[0];
    n_lines++;
}
```

### C. Pemilihan Sel pada Garis Terpilih
```c
/* Memilih sel pada baris terpilih berdasarkan Min CR -> Min Cost Asli -> Min Indeks */
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
}
```

---
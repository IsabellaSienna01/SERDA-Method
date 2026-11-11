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
  - [5. Tracing Eksekusi Program](#5-tracing-eksekusi-program)
    - [5.1 Case Standar: `G1_N01.txt`](#51-case-standar-g1_n01txt)
    - [5.2 Case Perbandingan Biaya: `G3_N35.txt`](#52-case-perbandingan-biaya-g3_n35txt)
    - [5.3 Case Prioritas Alokasi Kolom: `G2_N14.txt`](#53-case-prioritas-alokasi-kolom-g2_n14txt)
- [Panduan Teknis Penggunaan Repositori](#panduan-teknis-penggunaan-repositori)
  - [6. Metode yang Digunakan](#6-metode-yang-digunakan)
  - [7. Struktur Direktori](#7-struktur-direktori)
  - [8. Cara Menjalankan Program](#8-cara-menjalankan-program)
    - [8.1 Persiapan](#81-persiapan)
    - [8.2 Kompilasi Program](#82-kompilasi-program)
    - [8.3 Membersihkan File Kompilasi](#83-membersihkan-file-kompilasi)
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
     $$\text{Penalti Kolom } P_j = c^{(2)}_j - c^{(1)}_j$$
3. **Pilih Garis dengan Penalti Terbesar**:
   - Tentukan baris atau kolom dengan nilai penalti maksimum: $\max(P_i, P_j)$.
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



2. **Hitung Rata-rata Biaya Baris & Kolom Aktif:**
   - **Rata-rata baris aktif:** $AVG_i = \frac{\sum_{j \in J} C_{ij}}{|J|}$  
     Dengan $J$ adalah himpunan kolom aktif dan $|J|$ adalah jumlah kolom aktif.

   - **Rata-rata kolom aktif:** $AVG_j = \frac{\sum_{i \in I} C_{ij}}{|I|}$  
     Dengan $I$ adalah himpunan baris aktif dan $|I|$ adalah jumlah baris aktif.

3. **Transformasi Matriks Biaya Relatif ($CR_{ij}$)**:
   - Hitung biaya relatif untuk setiap sel aktif $(i, j)$:
     $$CR_{ij} = C_{ij} - AVG_i - AVG_j$$
4. **Hitung Penalti Baris & Kolom berbasis biaya relatif $CR$**:
   - Cari dua nilai $CR$ terkecil pada setiap baris aktif dan kolom aktif.
   - Penalti baris: $P_i = CR^{(2)}_i - CR^{(1)}_i$.
   - Penalti kolom: $P_j = CR^{(2)}_j - CR^{(1)}_j$.
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
8. **Ulangi langkah 2–7 hingga seluruh permintaan terpenuhi.**

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
| **Aturan Tie-Breaking Garis** | Arbitrary / Garis pertama ditemukan | 4 Tingkat: Penalti terbesar $\to$ Min $CR$ $\to$ Baris $\to$ Indeks terkecil |
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

## 5. Tracing Eksekusi Program

Tracing dilakukan pada tiga case untuk melihat bagaimana SERDA dan VAM menentukan alokasi, serta pengaruhnya terhadap total biaya transportasi.

### 5.1 Case Standar: `G1_N01.txt`

Case pertama menggunakan 3 sumber dan 3 tujuan. SERDA dan VAM menghasilkan biaya yang sama dengan solusi optimal.

- **Dimensi:** 3 sumber × 3 tujuan
- **Biaya optimal:** 5600
- **Hasil VAM:** 5600
- **Hasil SERDA:** 5600

**Proses alokasi SERDA:**

```text
Iterasi 1:
  Garis terpilih : Kolom 3 (Penalti = 80.0000, Min CR = -148.0000)
  Sel terpilih   : [3, 3] dengan biaya 60.00
  Alokasi        : 30.00 unit
  Sisa Supply 3 : 15.00
  Sisa Demand 3 : 0.00 (Kolom 3 selesai)

Iterasi 2:
  Garis terpilih : Baris 3 (Penalti = 85.3333, Min CR = -122.6667)
  Sel terpilih   : [3, 2] dengan biaya 80.00
  Alokasi        : 15.00 unit
  Sisa Supply 3 : 0.00 (Baris 3 selesai)
  Sisa Demand 2 : 20.00

Iterasi 3:
  Garis terpilih : Baris 1 (Penalti = 0.0000, Min CR = -50.0000)
  Sel terpilih   : [1, 1] dengan biaya 32.00
  Alokasi        : 20.00 unit
  Sisa Supply 1 : 0.00 (Baris 1 selesai)
  Sisa Demand 1 : 10.00

Iterasi 4:
  Garis terpilih : Baris 2 (Penalti = 0.0000, Min CR = -64.0000)
  Sel terpilih   : [2, 1] dengan biaya 60.00
  Alokasi        : 10.00 unit
  Sisa Supply 2 : 20.00
  Sisa Demand 1 : 0.00 (Kolom 1 selesai)

Iterasi 5:
  Garis terpilih : Baris 2 (Penalti = 0.0000, Min CR = -68.0000)
  Sel terpilih   : [2, 2] dengan biaya 68.00
  Alokasi        : 20.00 unit
  Sisa Supply 2 : 0.00
  Sisa Demand 2 : 0.00 (Semua terpenuhi)

Total Transportation Cost SERDA = 5600.00 (Optimal)
```

Seluruh suplai dan permintaan terpenuhi dalam lima iterasi. SERDA dan VAM sama-sama mencapai biaya optimal sebesar 5600.

---

### 5.2 Case Perbandingan Biaya: `G3_N35.txt`

SERDA menghasilkan biaya 280, sedangkan VAM menghasilkan biaya 320. Perbedaannya ada pada urutan alokasi, terutama saat menentukan sumber untuk memenuhi permintaan D1 dan D4.

**Matriks biaya dan kapasitas:**

| Sumber / Tujuan | D1 | D2 | D3 | D4 | Suplai |
|---|---:|---:|---:|---:|---:|
| **S1** | 1 | 6 | 5 | 9 | **20** |
| **S2** | 3 | 2 | 4 | 15 | **25** |
| **S3** | 8 | 9 | 3 | 10 | **15** |
| **Permintaan** | **15** | **15** | **10** | **20** | **60** |

**Biaya optimal eksak: 280**

#### Hasil SERDA

SERDA menyelesaikan alokasi dalam enam iterasi dengan total biaya 280.

```text
Iterasi 1:
  Kolom 2 → Sel [2, 2] = 15 unit, biaya 2
  Permintaan D2 terpenuhi

Iterasi 2:
  Baris 2 → Sel [2, 1] = 10 unit, biaya 3
  Suplai S2 habis

Iterasi 3:
  Kolom 1 → Sel [1, 1] = 5 unit, biaya 1
  Permintaan D1 terpenuhi

Iterasi 4:
  Baris 1 → Sel [1, 4] = 15 unit, biaya 9
  Suplai S1 habis

Iterasi 5:
  Baris 3 → Sel [3, 3] = 10 unit, biaya 3
  Permintaan D3 terpenuhi

Iterasi 6:
  Baris 3 → Sel [3, 4] = 5 unit, biaya 10
  Seluruh kebutuhan terpenuhi
```

**Matriks alokasi SERDA:**

| Sumber / Tujuan | D1 | D2 | D3 | D4 |
|---|---:|---:|---:|---:|
| **S1** | 5 | 0 | 0 | 15 |
| **S2** | 10 | 15 | 0 | 0 |
| **S3** | 0 | 0 | 10 | 5 |

**Total biaya SERDA:**

`(5 × 1) + (15 × 9) + (10 × 3) + (15 × 2) + (10 × 3) + (5 × 10) = 280`

#### Hasil VAM Standar

VAM menghasilkan biaya 320, lebih tinggi 14,29% dari biaya optimal.

**Matriks alokasi VAM:**

| Sumber / Tujuan | D1 | D2 | D3 | D4 |
|---|---:|---:|---:|---:|
| **S1** | 15 | 0 | 0 | 5 |
| **S2** | 0 | 15 | 0 | **10** |
| **S3** | 0 | 0 | 10 | 5 |

**Mengapa hasilnya berbeda?**

VAM lebih dulu mengalokasikan 15 unit dari S1 ke D1 dengan biaya 1 per unit. Karena permintaan D1 sudah habis, sisa 10 unit dari S2 harus dialokasikan ke D4 melalui sel `[2, 4]` yang biayanya 15 per unit.

SERDA mengambil keputusan berbeda. Setelah memenuhi D2, SERDA menggunakan 10 unit dari S2 untuk memenuhi D1 melalui sel `[2, 1]` dengan biaya 3 per unit.

Pemilihan alokasi SERDA mempertimbangkan nilai deviasi biaya (`CR`), yaitu perbedaan biaya sel terhadap rata-rata biaya baris dan kolom. Dengan urutan tersebut, SERDA tidak perlu menggunakan sel `[2, 4]` yang lebih mahal.

**Perbandingan hasil:**

| Metode | Total Biaya | Selisih dari Optimal |
|---|---:|---:|
| **SERDA** | 280 | 0% |
| **VAM** | 320 | +14,29% |

SERDA mencapai biaya optimal karena alokasinya menghindari penggunaan sel `[2, 4]` yang memiliki biaya tinggi.

---

### 5.3 Case Prioritas Alokasi Kolom: `G2_N14.txt`

SERDA menghasilkan biaya 920, sedangkan VAM menghasilkan biaya 960. Perbedaannya muncul sejak iterasi awal ketika kedua metode menentukan kolom yang harus dipenuhi terlebih dahulu.

**Matriks biaya dan kapasitas:**

| Sumber / Tujuan | D1 | D2 | D3 | D4 | Suplai |
|---|---:|---:|---:|---:|---:|
| **S1** | 4 | 6 | 8 | 8 | **40** |
| **S2** | 6 | 8 | 6 | 7 | **60** |
| **S3** | 5 | 7 | 6 | 8 | **50** |
| **Permintaan** | **20** | **30** | **50** | **50** | **150** |

**Biaya optimal eksak: 920**

#### Hasil SERDA

SERDA memprioritaskan D4 sejak iterasi pertama dan menghasilkan total biaya 920.

```text
Iterasi 1:
  Kolom 4 → Sel [2, 4] = 50 unit, biaya 7
  Permintaan D4 terpenuhi

Iterasi 2:
  Baris 2 → Sel [2, 3] = 10 unit, biaya 6
  Suplai S2 habis

Iterasi 3:
  Kolom 3 → Sel [3, 3] = 40 unit, biaya 6
  Permintaan D3 terpenuhi

Iterasi 4:
  Baris 1 → Sel [1, 1] = 20 unit, biaya 4
  Permintaan D1 terpenuhi

Iterasi 5:
  Baris 1 → Sel [1, 2] = 20 unit, biaya 6
  Suplai S1 habis

Iterasi 6:
  Baris 3 → Sel [3, 2] = 10 unit, biaya 7
  Seluruh kebutuhan terpenuhi
```

**Matriks alokasi SERDA:**

| Sumber / Tujuan | D1 | D2 | D3 | D4 |
|---|---:|---:|---:|---:|
| **S1** | 20 | 20 | 0 | 0 |
| **S2** | 0 | 0 | 10 | 50 |
| **S3** | 0 | 10 | 40 | 0 |

**Total biaya SERDA:**

`(20 × 4) + (20 × 6) + (10 × 6) + (50 × 7) + (10 × 7) + (40 × 6) = 920`

#### Hasil VAM Standar

VAM menghasilkan biaya 960, atau 4,35% di atas biaya optimal.

**Matriks alokasi VAM:**

| Sumber / Tujuan | D1 | D2 | D3 | D4 |
|---|---:|---:|---:|---:|
| **S1** | 20 | 20 | 0 | 0 |
| **S2** | 0 | 0 | 50 | 10 |
| **S3** | 0 | 10 | 0 | **40** |

**Mengapa hasilnya berbeda?**

VAM lebih dulu menggunakan 50 unit suplai S2 untuk memenuhi D3 melalui sel `[2, 3]` dengan biaya 6 per unit.

Akibatnya, hanya tersisa 10 unit dari S2 untuk D4. Kekurangan 40 unit lainnya harus dipenuhi dari S3 melalui sel `[3, 4]` dengan biaya 8 per unit.

SERDA justru memprioritaskan D4 dan langsung mengalokasikan 50 unit dari S2 melalui sel `[2, 4]` dengan biaya 7 per unit.

Keputusan ini membuat SERDA dapat memenuhi seluruh D4 tanpa menggunakan sel `[3, 4]` yang lebih mahal.

**Perbandingan hasil:**

| Metode | Total Biaya | Selisih dari Optimal |
|---|---:|---:|
| **SERDA** | 920 | 0% |
| **VAM** | 960 | +4,35% |

SERDA menghasilkan biaya lebih rendah karena memprioritaskan D4 sebelum suplai S2 digunakan untuk memenuhi tujuan lain.

---

# Panduan Teknis Penggunaan Repositori

## 6. Metode yang Digunakan

Repositori ini menggunakan delapan algoritma untuk menghasilkan *Initial Basic Feasible Solution* (IBFS), yaitu alokasi awal yang memenuhi seluruh suplai dan permintaan.

| Metode | File | Cara Kerja |
|---|---|---|
| **SERDA** | `src/SERDA.c` | Menghitung deviasi biaya terhadap rata-rata baris dan kolom untuk menentukan prioritas alokasi. Menggunakan empat tingkat *tie-breaking* agar hasilnya konsisten. |
| **SAM** | `src/SAM.c` | Menggunakan suplai dan rata-rata biaya baris untuk menentukan alokasi awal, lalu memindahkan alokasi dari baris yang memiliki kelebihan suplai. |
| **CSM** | `src/CSM.cpp` | Menggunakan total biaya dan suplai baris sebagai bobot, kemudian melakukan substitusi melalui sel donatur dengan biaya kedua terendah. |
| **RBSM** | `src/RBSM.cpp` | Menentukan substitusi antarbaris berdasarkan penalti dan perbandingan kapasitas suplai. |
| **IHW** | `src/IHW.c` | Menggunakan *Inverse Contra-Harmonic Mean* (ICHM) sebagai bobot penalti untuk memilih sel alokasi. |
| **VAM** | `src/VAM.c` | Menggunakan selisih dua biaya terendah untuk menentukan baris atau kolom yang diprioritaskan. |
| **THP** | `src/THP.c` | Memilih kandidat dari dua penalti tertinggi, lalu membandingkan biaya dikalikan jumlah alokasinya. |
| **FVAM** | `src/FVAM.c` | Menggunakan rasio dua biaya terendah untuk menentukan penalti, termasuk penanganan biaya bernilai nol. |

SERDA menggunakan rumus:

`CR_ij = C_ij - AVG_i - AVG_j`

Dengan:

- `C_ij`: biaya dari sumber `i` ke tujuan `j`.
- `AVG_i`: rata-rata biaya pada baris `i`.
- `AVG_j`: rata-rata biaya pada kolom `j`.
- `CR_ij`: deviasi biaya yang digunakan SERDA untuk menentukan prioritas alokasi.

Delapan metode tersebut diuji menggunakan dataset yang sama melalui program benchmark, sehingga biaya yang dihasilkan dapat dibandingkan.

---

## 7. Struktur Direktori

Struktur file pada repositori:

```text
.
├── dataset/
│   ├── G1_N01.txt ... G9_R5.txt  # 69 file data uji
│   └── exact_optimal.json       # Biaya optimal eksak
│
├── img/
│   ├── flowchart-serda.png      # Diagram alir SERDA
│   └── flowchart-vam.png        # Diagram alir VAM
│
├── src/
│   ├── SERDA.c                  # Implementasi SERDA
│   ├── FVAM.c                   # Implementasi FVAM
│   ├── THP.c                    # Implementasi THP
│   ├── VAM.c                    # Implementasi VAM
│   ├── SAM.c                    # Implementasi SAM
│   ├── RBSM.cpp                 # Implementasi RBSM
│   ├── CSM.cpp                  # Implementasi CSM
│   ├── IHW.c                    # Implementasi IHW
│   └── benchmark.cpp            # Program benchmark
│
├── results/
│   ├── benchmark_summary.csv    # Ringkasan performa 8 metode
│   ├── benchmark_details.csv    # Hasil tiap case
│   └── benchmark_report.txt     # Laporan perbandingan lengkap
│
├── Makefile                     # Perintah kompilasi
├── .gitattributes               # Pengaturan Git
└── README.md                    # Dokumentasi proyek
```

Folder `dataset/` berisi 69 file pengujian dan biaya optimal eksak sebagai pembanding. Implementasi delapan metode dan program benchmark berada di folder `src/`.

Hasil pengujian disimpan di folder `results/`, baik dalam bentuk ringkasan maupun rincian biaya untuk setiap case.

---

## 8. Cara Menjalankan Program

### 8.1 Persiapan

Program membutuhkan:

- **GCC dan G++** untuk kompilasi C/C++.
- **GNU Make** untuk menjalankan perintah dari `Makefile`.

Pengguna Windows dapat menggunakan MinGW-w64, sedangkan pengguna Linux dapat menggunakan GCC dan G++ yang tersedia pada sistem.

### 8.2 Kompilasi Program

Buka terminal di folder utama repositori, lalu jalankan:

```bash
make all
```

Perintah ini akan mengompilasi program sesuai konfigurasi dalam `Makefile`.

### 8.3 Membersihkan File Kompilasi

Jalankan:

```bash
make clean
```

Perintah ini menghapus file sementara hasil kompilasi, seperti `.exe` dan `.o`.

`Makefile` menyesuaikan perintah penghapusan berdasarkan sistem operasi:

- **Windows:** menggunakan `del`.
- **Linux/macOS:** menggunakan `rm`.

---

# Hasil Pengujian & Analisis Komparasi

## 13. Ringkasan Hasil Benchmark (69 Testcase)

Tabel berikut menyajikan rekapitulasi performa 8 metode yang diuji terhadap seluruh 69 testcase standar:

| Peringkat | Metode | Optimal Hits | Akurasi (%) | Mean Gap (%) | Median Gap (%) | Max Gap (%) | Std Dev (%) | Avg Time (ms) |
|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| 1 | **SERDA** | **57 / 69** | **82.61%** | **0.45%** | **0.00%** | **9.49%** | **1.53%** | 0.101 ms |
| 2 | **SAM** | 48 / 69 | 69.57% | 2.12% | 0.00% | 21.53% | 4.26% | 0.174 ms |
| 3 | **CSM** | 45 / 69 | 65.22% | 2.53% | 0.00% | 32.66% | 5.35% | 0.159 ms |
| 4 | **RBSM** | 45 / 69 | 65.22% | 2.33% | 0.00% | 28.31% | 4.77% | 0.290 ms |
| 5 | **IHW** | 40 / 69 | 57.97% | 4.44% | 0.00% | 66.99% | 10.41% | 0.638 ms |
| 6 | **VAM** | 36 / 69 | 52.17% | 3.57% | 0.00% | 21.14% | 5.44% | 0.406 ms |
| 7 | **THP** | 29 / 69 | 42.03% | 5.19% | 1.22% | 72.73% | 11.27% | 0.015 ms |
| 8 | **FVAM** | 28 / 69 | 40.58% | 4.19% | 0.41% | 48.33% | 7.46% | 0.130 ms |

### Perbandingan Runtime Setiap Metode

Tabel berikut menyajikan peringkat efisiensi waktu eksekusi (*runtime*) dari masing-masing metode yang diuji pada seluruh 69 testcase:

| Peringkat | Metode | Rata-rata Waktu (ms) | Total Waktu Eksekusi (69 TC) |
|:---:|:---|---:|---:|
| 1 | **THP** | 0.0145 ms | 1.00 ms |
| 2 | **SERDA** | 0.1014 ms | 7.00 ms |
| 3 | **FVAM** | 0.1304 ms | 9.00 ms |
| 4 | **CSM** | 0.1594 ms | 11.00 ms |
| 5 | **SAM** | 0.1739 ms | 12.00 ms |
| 6 | **RBSM** | 0.2899 ms | 20.00 ms |
| 7 | **VAM** | 0.4058 ms | 28.00 ms |
| 8 | **IHW** | 0.6377 ms | 44.00 ms |

---

## 14. Tabel Perbandingan Hasil Lengkap: VAM vs SERDA vs Optimal (69 TC)

Berikut adalah data komparasi biaya antara VAM standar, SERDA Method, dan Solusi Optimal Eksak pada 69 berkas testcase:

> **Catatan**: Tanda bintang (`*`) menandakan bahwa metode menghasilkan biaya yang identik dengan nilai solusi optimal eksak.

| No | Test Case | VAM Cost | SERDA Cost | Solusi Optimal | Keterangan Perbandingan |
|:---:|:---|---:|---:|---:|:---|
| 1 | `G1_N01.txt` | 5600* | 5600* | 5600 | Keduanya Optimal |
| 2 | `G1_N12.txt` | 425* | 425* | 425 | Keduanya Optimal |
| 3 | `G1_N21.txt` | 4205* | 4205* | 4205 | Keduanya Optimal |
| 4 | `G1_P07.txt` | 327 | 291* | 291 | SERDA Optimal, VAM Suboptimal (+12.37%) |
| 5 | `G1_P11.txt` | 600* | 600* | 600 | Keduanya Optimal |
| 6 | `G1_P17.txt` | 490 | 475* | 475 | SERDA Optimal, VAM Suboptimal (+3.16%) |
| 7 | `G1_P18.txt` | 555* | 555* | 555 | Keduanya Optimal |
| 8 | `G1_P19.txt` | 1500 | 1390* | 1390 | SERDA Optimal, VAM Suboptimal (+7.91%) |
| 9 | `G2_N02.txt` | 28* | 28* | 28 | Keduanya Optimal |
| 10 | `G2_N14.txt` | 960 | 920* | 920 | SERDA Optimal, VAM Suboptimal (+4.35%) |
| 11 | `G2_N15.txt` | 156* | 156* | 156 | Keduanya Optimal |
| 12 | `G2_N22.txt` | 12075* | 12075* | 12075 | Keduanya Optimal |
| 13 | `G2_N23.txt` | 23* | 23* | 23 | Keduanya Optimal |
| 14 | `G2_P12.txt` | 180* | 180* | 180 | Keduanya Optimal |
| 15 | `G3_N31.txt` | 1480* | 1480* | 1480 | Keduanya Optimal |
| 16 | `G3_N33.txt` | 1430 | 1250* | 1250 | SERDA Optimal, VAM Suboptimal (+14.40%) |
| 17 | `G3_N35.txt` | 320 | 280* | 280 | SERDA Optimal, VAM Suboptimal (+14.29%) |
| 18 | `G3_P02.txt` | 2850* | 2850* | 2850 | Keduanya Optimal |
| 19 | `G3_P03.txt` | 149* | 149* | 149 | Keduanya Optimal |
| 20 | `G3_P04.txt` | 450* | 450* | 450 | Keduanya Optimal |
| 21 | `G3_P09.txt` | 845* | 845* | 845 | Keduanya Optimal |
| 22 | `G3_P13.txt` | 3210* | 3210* | 3210 | Keduanya Optimal |
| 23 | `G3_P20.txt` | 3320* | 3320* | 3320 | Keduanya Optimal |
| 24 | `G3_S07.txt` | 326 | 305* | 305 | SERDA Optimal, VAM Suboptimal (+6.89%) |
| 25 | `G4_N07.txt` | 1220 | 1165 | 1160 | SERDA lebih mendekati optimal |
| 26 | `G4_N10.txt` | 990 | 910* | 910 | SERDA Optimal, VAM Suboptimal (+8.79%) |
| 27 | `G4_N11.txt` | 2980 | 2460* | 2460 | SERDA Optimal, VAM Suboptimal (+21.14%) |
| 28 | `G4_N16.txt` | 1590 | 1510* | 1510 | SERDA Optimal, VAM Suboptimal (+5.30%) |
| 29 | `G4_N17.txt` | 779 | 743* | 743 | SERDA Optimal, VAM Suboptimal (+4.85%) |
| 30 | `G4_N25.txt` | 849 | 809* | 809 | SERDA Optimal, VAM Suboptimal (+4.94%) |
| 31 | `G4_N29.txt` | 3857* | 3857* | 3857 | Keduanya Optimal |
| 32 | `G4_P01.txt` | 605* | 605* | 605 | Keduanya Optimal |
| 33 | `G5_N06.txt` | 8350 | 7750* | 7750 | SERDA Optimal, VAM Suboptimal (+7.74%) |
| 34 | `G5_N13.txt` | 5125 | 4525* | 4525 | SERDA Optimal, VAM Suboptimal (+13.26%) |
| 35 | `G5_N18.txt` | 1745 | 1650* | 1650 | SERDA Optimal, VAM Suboptimal (+5.76%) |
| 36 | `G5_N24.txt` | 3520 | 3460* | 3460 | SERDA Optimal, VAM Suboptimal (+1.73%) |
| 37 | `G5_N27.txt` | 68804 | 59356* | 59356 | SERDA Optimal, VAM Suboptimal (+15.92%) |
| 38 | `G5_N28.txt` | 2090* | 2090* | 2090 | Keduanya Optimal |
| 39 | `G5_N30.txt` | 473 | 465* | 465 | SERDA Optimal, VAM Suboptimal (+1.72%) |
| 40 | `G6_N04.txt` | 1580* | 1580* | 1580 | Keduanya Optimal |
| 41 | `G6_N09.txt` | 114 | 111* | 111 | SERDA Optimal, VAM Suboptimal (+2.70%) |
| 42 | `G6_N32.txt` | 1820 | 1820 | 1780 | Keduanya Suboptimal Seimbang |
| 43 | `G6_N34.txt` | 510* | 510* | 510 | Keduanya Optimal |
| 44 | `G6_P05.txt` | 290* | 290* | 290 | Keduanya Optimal |
| 45 | `G6_P10.txt` | 212* | 212* | 212 | Keduanya Optimal |
| 46 | `G6_RBSM1.txt` | 18855* | 18855* | 18855 | Keduanya Optimal |
| 47 | `G6_RBSM2.txt` | 153500* | 153500* | 153500 | Keduanya Optimal |
| 48 | `G6_S04.txt` | 3150 | 3150 | 3125 | Keduanya Suboptimal Seimbang |
| 49 | `G7_N03.txt` | 1104 | 1103 | 1102 | SERDA lebih mendekati optimal |
| 50 | `G7_N05.txt` | 470 | 415 | 410 | SERDA lebih mendekati optimal |
| 51 | `G7_N26.txt` | 490* | 502 | 490 | VAM Optimal |
| 52 | `G7_P06.txt` | 49* | 49* | 49 | Keduanya Optimal |
| 53 | `G7_P08.txt` | 11000 | 10625* | 10625 | SERDA Optimal, VAM Suboptimal (+3.53%) |
| 54 | `G7_P14.txt` | 175* | 175* | 175 | Keduanya Optimal |
| 55 | `G7_P15.txt` | 12810 | 10830* | 10830 | SERDA Optimal, VAM Suboptimal (+18.28%) |
| 56 | `G7_P16.txt` | 340 | 325* | 325 | SERDA Optimal, VAM Suboptimal (+4.62%) |
| 57 | `G8_RBSM3.txt` | 1044180* | 1044180* | 1044180 | Keduanya Optimal |
| 58 | `G8_RBSM4.txt` | 93351 | 92259 | 92212 | SERDA sangat dekat ke optimal (Gap 0.05%) |
| 59 | `G8_RBSM5.txt` | 116020* | 116020* | 116020 | Keduanya Optimal |
| 60 | `G8_S01.txt` | 5148 | 5302 | 5067 | Keduanya Suboptimal |
| 61 | `G8_S02.txt` | 1089 | 1098 | 1080 | Keduanya Suboptimal |
| 62 | `G8_S03.txt` | 948 | 969 | 885 | Keduanya Suboptimal |
| 63 | `G8_S05.txt` | 1801 | 1600 | 1586 | SERDA lebih baik (Gap 0.88% vs 13.55%) |
| 64 | `G8_S06.txt` | 5409 | 5022 | 4702 | SERDA lebih baik (Gap 6.80% vs 15.03%) |
| 65 | `G9_R1.txt` | 12928429* | 12928429* | 12928429 | Keduanya Optimal |
| 66 | `G9_R2.txt` | 13657053* | 13657053* | 13657053 | Keduanya Optimal |
| 67 | `G9_R3.txt` | 12296587* | 12296587* | 12296587 | Keduanya Optimal |
| 68 | `G9_R4.txt` | 14117586* | 14117586* | 14117586 | Keduanya Optimal |
| 69 | `G9_R5.txt` | 14017419* | 14017419* | 14017419 | Keduanya Optimal |

---

## 15. Kesimpulan Komparasi VAM vs SERDA

Berdasarkan hasil pengujian empiris terhadap 69 testcase standar:

1. **Akurasi Solusi Optimal**:
   Algoritma SERDA menghasilkan solusi optimal pada **57 dari 69 testcase (82.61%)**, dibandingkan VAM standar yang mencapai **36 dari 69 testcase (52.17%)**. Terdapat 21 kasus uji di mana SERDA berhasil mencapai solusi optimal sementara VAM menghasilkan nilai suboptimal.
2. **Tingkat Deviasi (Optimality Gap)**:
   Rata-rata deviasi biaya (*Mean Gap*) pada SERDA tercatat sebesar **0.45%**, lebih rendah dibandingkan VAM standar yang memiliki *Mean Gap* sebesar **3.57%**.
3. **Stabilitas Deviasi Terburuk (*Max Gap*)**:
   Deviasi terbesar (*Max Gap*) yang dihasilkan oleh VAM mencapai **21.14%** (terjadi pada kasus `G4_N11.txt`), sedangkan deviasi terbesar pada SERDA tidak melampaui **9.49%**.
4. **Waktu Komputasi**:
   Rata-rata waktu eksekusi SERDA berada pada kisaran **0.101 ms** per kasus uji, setara dengan VAM standar (**0.406 ms**). Hal ini menunjukkan bahwa penambahan komputasi rata-rata dinamis dan aturan *tie-breaking* bertingkat tidak membebani efisiensi waktu komputasi secara nyata dan tetap berada dalam kelas kompleksitas polinomial $O(N^2 M + NM^2)$.
   
---

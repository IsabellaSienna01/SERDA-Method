# Komputasi dan Automated Benchmarking Metode Transportation Problem (IBFS)

Repository ini berisi implementasi, standardisasi antarmuka, dan sistem pengujian otomatis (*automated benchmarking*) untuk menyelesaikan **Transportation Problem (TP)** menggunakan berbagai metode pembentuk **Initial Basic Feasible Solution (IBFS)**.

Sistem ini mengevaluasi dan membandingkan performa 8 metode IBFS secara objektif terhadap **69 testcase** transportasi standar dengan ground truth nilai optimal yang diverifikasi secara independen.

---

## Daftar Isi
- [1. Ringkasan Metode yang Diuji](#1-ringkasan-metode-yang-diuji)
- [2. Struktur Direktori](#2-struktur-direktori)
- [3. Prasyarat Sistem & Kompilasi](#3-prasyarat-sistem--kompilasi)
- [4. Cara Menjalankan Benchmark Otomatis](#4-cara-menjalankan-benchmark-otomatis)
- [5. Cara Menjalankan Setiap Metode Secara Mandiri](#5-cara-menjalankan-setiap-metode-secara-mandiri)
- [6. Format Input & Output](#6-format-input--output)
- [7. Formula dan Metrik Evaluasi](#7-formula-dan-metrik-evaluasi)
- [8. Ringkasan Hasil Benchmark (69 Testcase)](#8-ringkasan-hasil-benchmark-69-testcase)

---

## 1. Ringkasan Metode yang Diuji

Terdapat 8 metode IBFS yang terintegrasi di dalam repositori:

1. **SERDA (`SERDA.c`)**:
   - *Dynamic Mean-Opportunity Vogel's Approximation Method (DMO-VAM)*.
   - Menggunakan penyesuaian biaya relatif terhadap rata-rata baris dan kolom aktif:
     $$CR_{ij} = C_{ij} - \bar{C}_i - \bar{C}_j$$
   - Penalti dihitung dari selisih dua $CR$ terkecil pada setiap baris/kolom aktif, disertai aturan tie-breaking 4 level deterministik.

2. **FVAM (`FVAM.c`)**:
   - *Fractional Vogel's Approximation Method* (FVAM).
   - Menggunakan rasio kontras multiplikatif biaya kedua terkecil terhadap biaya terkecil:
     $$P_i = \frac{m(2)_i}{m(1)_i}, \quad Q_j = \frac{m(2)_j}{m(1)_j}$$
   - Dilengkapi penanganan biaya 0 dengan prioritas sangat tinggi secara deterministik dan tie-breaking baris sebelum kolom.

3. **THP (`THP.c`)**:
   - *Two Highest Penalties Method* (THP).
   - Penalti dihitung dari rentang $(\max - \min)$. Mengambil kandidat dari dua level penalti tertinggi (*two highest distinct penalties*), kemudian memilih alokasi dengan dampak biaya total terkecil: $CA = \text{cost} \times \text{allocation}$.

4. **VAM (`VAM.c`)**:
   - *Vogel's Approximation Method* klasik. Penalti berupa selisih absolut antara biaya kedua terkecil dan biaya terkecil ($c_{(2)} - c_{(1)}$).

5. **SAM (`SAM.c`)**:
   - *Supply Average Method*. Pendekatan dua tahap: tahap alokasi awal berdasarkan biaya minimum kolom (tie-break menggunakan rata-rata baris dikali suplai), dilanjutkan tahap redistribusi/donasi untuk mengatasi *Excess Rows* (ER).

6. **CSM (`CSM.cpp`)**:
   - *Cost-Supply Method v2*. Metode dua tahap berbasis substitusi sel minimum dengan pembobotan $CS = \text{Total Cost Baris} \times \text{Supply Baris}$ untuk penentuan prioritas donasi.

7. **RBSM (`RBSM.cpp`)**:
   - *Row-Based Substitution Method*. Pendekatan substitusi baris dengan metrik rasio donatur/penerima berbasis perkalian suplai dan biaya sel.

8. **IHW (`IHW.c`)**:
   - *Inverse Harmonic Weight Method* (RCWMCAM-TOCM) menggunakan Inverse Contra-Harmonic Mean (ICHM) sebagai penalti pembobot.

---

## 2. Struktur Direktori

```text
.
├── dataset/                    # Direktori dataset & ground truth
│   ├── G1_N01.txt ... G9_R5.txt# 69 file testcase transportation problem (.txt)
│   └── exact_optimal.json      # Ground truth nilai optimal terverifikasi Simplex
├── src/                        # Seluruh source code metode & benchmark
│   ├── SERDA.c                 # Implementasi C metode SERDA (DMO-VAM)
│   ├── FVAM.c                  # Implementasi C metode FVAM (Paper MDPI 2026)
│   ├── THP.c                   # Konversi C metode THP
│   ├── VAM.c                   # Implementasi C metode VAM
│   ├── SAM.c                   # Implementasi C metode SAM (disesuaikan)
│   ├── RBSM.cpp                # Implementasi C++ metode RBSM (disesuaikan)
│   ├── CSM.cpp                 # Implementasi C++ metode CSM (disesuaikan)
│   ├── IHW.c                   # Implementasi C metode IHW (disesuaikan)
│   └── benchmark.cpp           # Runner automated benchmarking 69 testcase
├── results/                    # Direktori output hasil benchmark
│   ├── benchmark_summary.csv   # Rekapitulasi metrik performa ke-8 metode
│   ├── benchmark_details.csv   # Detail biaya dan gap per testcase
│   └── benchmark_report.txt    # Laporan teks terstruktur hasil benchmark
├── Makefile                    # Script build otomasi GCC/G++
├── .gitattributes              # Konfigurasi normalisasi line-ending & binary git
└── README.md                   # Dokumentasi proyek ini
```

---

## 3. Prasyarat Sistem & Kompilasi

### Prasyarat:
- Kompiler C/C++: **GCC** dan **G++** (MinGW-w64 pada Windows atau GCC standar pada Linux).
- Alat bantu build: **GNU Make**.

### Kompilasi Seluruh Program:
Untuk mengompilasi seluruh source code menjadi executable di direktori root:
```bash
make all
```

Atau kompilasi secara manual per file dari folder `src/`:
```bash
gcc -O2 -Wall src/SERDA.c -o SERDA.exe
gcc -O2 -Wall src/THP.c -o THP.exe
gcc -O2 -Wall src/FVAM.c -o FVAM.exe
gcc -O2 -Wall src/VAM.c -o VAM.exe
gcc -O2 -Wall src/SAM.c -o SAM.exe
gcc -O2 -Wall src/IHW.c -o IHW.exe
g++ -O2 -Wall src/RBSM.cpp -o RBSM.exe
g++ -O2 -Wall src/CSM.cpp -o CSM.exe
g++ -O2 -Wall src/benchmark.cpp -o benchmark.exe
```

### Membersihkan Hasil Kompilasi (`make clean`):
Untuk menghapus seluruh file executable (`.exe`) dan file objek sementara (`.o`) yang telah dikompilasi:
```bash
make clean
```
Perintah ini dirancang *cross-platform* (otomatis mendeteksi sistem operasi Windows menggunakan perintah `del` atau Linux/macOS menggunakan `rm`), sehingga repository dapat kembali bersih hanya menyisakan kode sumber, dataset, dan laporan hasil tanpa menyertakan file biner yang tidak perlu.

---

## 4. Cara Menjalankan Benchmark Otomatis

Untuk menjalankan pengujian lengkap ($69 \text{ testcase} \times 8 \text{ metode} = 552 \text{ eksekusi}$):

### Menggunakan Makefile:
```bash
make benchmark
```

### Menjalankan Executable Langsung:
```bash
./benchmark.exe
```
*(Pada Windows PowerShell / CMD: `.\benchmark.exe`)*

Setelah proses selesai, tiga file laporan akan otomatis disimpan di dalam direktori `results/`:
1. `results/benchmark_summary.csv` — ringkasan statistik (Akurasi, Mean Gap, Median Gap, Max Gap, Std Dev, Rata-rata Waktu).
2. `results/benchmark_details.csv` — data mentah matriks biaya dan gap setiap metode di setiap testcase.
3. `results/benchmark_report.txt` — laporan tabel teks yang rapi dan mudah dibaca.

---

## 5. Cara Menjalankan Setiap Metode Secara Mandiri

Setiap program dapat dijalankan secara individual untuk memecahkan satu testcase tertentu.

### A. Mode Human-Readable (Format Tampilan Asli)
Menampilkan rincian iterasi, matriks alokasi, status baris, dan total biaya:
```bash
./SERDA.exe dataset/G1_N01.txt
./THP.exe   dataset/G1_N01.txt
./FVAM.exe  dataset/G1_N01.txt
./VAM.exe   dataset/G1_N01.txt
./SAM.exe   dataset/G1_N01.txt
./IHW.exe   dataset/G1_N01.txt
./RBSM.exe  dataset/G1_N01.txt
./CSM.exe  dataset/G1_N01.txt
```
*(Jika program lama seperti `IHW.exe`, `RBSM.exe`, atau `CSM.exe` dijalankan tanpa argumen file, program akan menampilkan menu interaktif aslinya).*

### B. Mode Machine-Readable (`--benchmark`)
Menghasilkan output terstruktur 1 baris untuk keperluan integrasi otomatis:
```bash
./SERDA.exe dataset/G1_N01.txt --benchmark
```
**Contoh Output:**
```text
METHOD:SERDA,STATUS:SUCCESS,COST:5600.00,FEASIBLE:1,TIME_MS:0.0870
```

---

## 6. Format Input & Output

### Format File Testcase (`.txt`)
Setiap file testcase di dalam direktori `dataset/` memiliki struktur standar berikut:
```text
m n

cost[0][0] cost[0][1] ... cost[0][n-1]
cost[1][0] cost[1][1] ... cost[1][n-1]
...
cost[m-1][0] ... cost[m-1][n-1]

supply[0] supply[1] ... supply[m-1]
demand[0] demand[1] ... demand[n-1]

optimal_cost (opsional / referensi)
```

### Penanganan Masalah Unbalanced
Seluruh program telah dilengkapi balancing otomatis:
- Jika $\sum \text{Supply} > \sum \text{Demand}$: Ditambahkan 1 kolom dummy dengan demand sebesar selisih dan biaya $0$.
- Jika $\sum \text{Demand} > \sum \text{Supply}$: Ditambahkan 1 baris dummy dengan supply sebesar selisih dan biaya $0$.

---

## 7. Formula dan Metrik Evaluasi

1. **Akurasi Solusi Optimal (%)**:
   Persentase testcase di mana metode menghasilkan biaya transportasi yang sama persis dengan solusi optimal eksak:
   $$\text{Accuracy (\%)} = \left( \frac{\text{Jumlah Testcase Optimal}}{\text{Total Testcase}} \right) \times 100\%$$
   Kriteria optimal:
   $$|Z_{\text{method}} - Z_{\text{optimal}}| \le 10^{-4}$$

2. **Optimality Gap (%)**:
   Persentase selisih biaya heuristik relatif terhadap biaya optimal:
   $$\text{Gap (\%)} = \left( \frac{Z_{\text{method}} - Z_{\text{optimal}}}{Z_{\text{optimal}}} \right) \times 100\%$$

3. **Statistik Gap**:
   - **Mean Gap**: Rata-rata gap dari seluruh testcase yang feasible.
   - **Median Gap**: Nilai tengah sebaran gap.
   - **Max Gap**: Deviasi terbesar dari nilai optimal.
   - **Std Dev Gap**: Standar deviasi yang mengukur stabilitas metode.

4. **Feasibility Check**:
   Solusi dinyatakan feasible jika:
   - $\sum_{j=1}^n x_{ij} = S_i$ untuk setiap baris sumber $i$.
   - $\sum_{i=1}^m x_{ij} = D_j$ untuk setiap kolom tujuan $j$.
   - $x_{ij} \ge 0$ untuk setiap sel $(i, j)$.

---

## 8. Ringkasan Hasil Benchmark (69 Testcase)

Tabel berikut merupakan hasil eksekusi aktual dari 552 kombinasi pengujian ($69 \text{ testcase} \times 8 \text{ metode}$):

| Method | Optimal Hits | Total TC | Accuracy (%) | Mean Gap (%) | Median Gap (%) | Max Gap (%) | Std Dev (%) | Feasibility | Avg Time (ms) |
|:---|---:|---:|---:|---:|---:|---:|---:|:---:|---:|
| **SERDA** | **57 / 69** | 69 | **82.61%** | **0.45%** | **0.00%** | **9.49%** | **1.53%** | 69 / 69 | 0.101 ms |
| **SAM** | 48 / 69 | 69 | 69.57% | 2.12% | 0.00% | 21.53% | 4.26% | 69 / 69 | 0.174 ms |
| **CSM** | 45 / 69 | 69 | 65.22% | 2.53% | 0.00% | 32.66% | 5.35% | 69 / 69 | 0.159 ms |
| **RBSM** | 45 / 69 | 69 | 65.22% | 2.33% | 0.00% | 28.31% | 4.77% | 69 / 69 | 0.290 ms |
| **IHW** | 40 / 69 | 69 | 57.97% | 4.44% | 0.00% | 66.99% | 10.41% | 69 / 69 | 0.638 ms |
| **VAM** | 36 / 69 | 69 | 52.17% | 3.57% | 0.00% | 21.14% | 5.44% | 69 / 69 | 0.406 ms |
| **THP** | 29 / 69 | 69 | 42.03% | 5.19% | 1.22% | 72.73% | 11.27% | 69 / 69 | 0.015 ms |
| **FVAM** | 28 / 69 | 69 | 40.58% | 4.19% | 0.41% | 48.33% | 7.46% | 69 / 69 | 0.130 ms |

---


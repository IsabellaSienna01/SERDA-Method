/*
 * benchmark.cpp - Automated Benchmarking System for Transportation Problem IBFS Methods
 * Evaluates 8 methods across 69 testcases:
 * 1. SERDA
 * 2. IHW
 * 3. CSM
 * 4. RBSM
 * 5. SAM
 * 6. VAM
 * 7. THP
 * 8. FVAM
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

using namespace std;

void ensure_directory(const string &dir) {
#ifdef _WIN32
    CreateDirectoryA(dir.c_str(), NULL);
#else
    mkdir(dir.c_str(), 0755);
#endif
}

struct MethodResult {
    string name;
    string status;
    double cost;
    int feasible;
    double time_ms;
    double gap_percent;
    bool is_optimal;
};

struct TestcaseRecord {
    string filename;
    int m;
    int n;
    double optimal_cost;
    map<string, MethodResult> results;
};

struct MethodStats {
    string name;
    int total_runs;
    int successful_runs;
    int feasible_runs;
    int failed_runs;
    int optimal_hits;
    double accuracy_percent;
    double mean_gap;
    double median_gap;
    double min_gap;
    double max_gap;
    double std_gap;
    double avg_time_ms;
};

// Ground truth exact optimal values for all 69 testcases verified by exact LP Simplex
map<string, double> load_exact_optimal_reference() {
    map<string, double> opt;
    vector<string> candidates = {
        "dataset/exact_optimal.json",
        "exact_optimal.json",
        "../dataset/exact_optimal.json"
    };
    ifstream f;
    for (const auto &p : candidates) {
        f.open(p);
        if (f.is_open()) break;
    }
    if (f.is_open()) {
        string line;
        while (getline(f, line)) {
            size_t colon = line.find(':');
            if (colon != string::npos) {
                size_t q1 = line.find('"');
                size_t q2 = line.find('"', q1 + 1);
                if (q1 != string::npos && q2 != string::npos && q2 < colon) {
                    string key = line.substr(q1 + 1, q2 - q1 - 1);
                    string val_str = line.substr(colon + 1);
                    size_t comma = val_str.find(',');
                    if (comma != string::npos) val_str = val_str.substr(0, comma);
                    try {
                        opt[key] = stod(val_str);
                    } catch (...) {}
                }
            }
        }
        f.close();
    }
    return opt;
}

vector<string> get_testcase_files(const string &dir_path) {
    vector<string> files;
#ifdef _WIN32
    string pattern = dir_path + "/*.txt";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(dir_path + "/" + fd.cFileName);
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
#else
    DIR *d = opendir(dir_path.c_str());
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            string fname = dir->d_name;
            if (fname.size() > 4 && fname.substr(fname.size() - 4) == ".txt") {
                files.push_back(dir_path + "/" + fname);
            }
        }
        closedir(d);
    }
#endif
    sort(files.begin(), files.end());
    return files;
}

string get_basename(const string &path) {
    size_t last_slash = path.find_last_of("/\\");
    if (last_slash != string::npos) return path.substr(last_slash + 1);
    return path;
}

string resolve_executable(const string &executable) {
    ifstream f(executable);
    if (f.is_open()) return executable;
    ifstream f_src("src/" + executable);
    if (f_src.is_open()) return "src/" + executable;
    ifstream f_dot("./" + executable);
    if (f_dot.is_open()) return "./" + executable;
    ifstream f_up("../" + executable);
    if (f_up.is_open()) return "../" + executable;
    return executable;
}

MethodResult run_method(const string &executable, const string &testcase_path, const string &method_name, double opt_cost) {
    MethodResult res;
    res.name = method_name;
    res.status = "FAILED";
    res.cost = -1.0;
    res.feasible = 0;
    res.time_ms = 0.0;
    res.gap_percent = 999.99;
    res.is_optimal = false;

    string actual_exe = resolve_executable(executable);
    string cmd = actual_exe + " \"" + testcase_path + "\" --benchmark";
    FILE *fp = popen(cmd.c_str(), "r");
    if (!fp) return res;

    char buffer[1024];
    string output = "";
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        output += buffer;
    }
    pclose(fp);

    // Parse METHOD:...,STATUS:...,COST:...,FEASIBLE:...,TIME_MS:...
    stringstream ss(output);
    string token;
    while (getline(ss, token, ',')) {
        size_t colon = token.find(':');
        if (colon != string::npos) {
            string k = token.substr(0, colon);
            string v = token.substr(colon + 1);
            while (!v.empty() && (v.back() == '\r' || v.back() == '\n' || v.back() == ' ')) v.pop_back();
            while (!v.empty() && v.front() == ' ') v.erase(v.begin());

            if (k == "STATUS") res.status = v;
            else if (k == "COST") {
                try { res.cost = stod(v); } catch (...) {}
            }
            else if (k == "FEASIBLE") {
                try { res.feasible = stoi(v); } catch (...) {}
            }
            else if (k == "TIME_MS") {
                try { res.time_ms = stod(v); } catch (...) {}
            }
        }
    }

    if (res.feasible && res.cost >= 0) {
        if (fabs(res.cost - opt_cost) <= 1e-4) {
            res.is_optimal = true;
            res.gap_percent = 0.0;
        } else {
            res.is_optimal = false;
            if (opt_cost > 1e-6) {
                res.gap_percent = ((res.cost - opt_cost) / opt_cost) * 100.0;
            } else {
                res.gap_percent = 0.0;
            }
        }
    }

    return res;
}

int main(int argc, char *argv[]) {
    cout << "=========================================================================\n";
    cout << "  AUTOMATED BENCHMARKING SYSTEM: TRANSPORTATION PROBLEM (69 TESTCASES)  \n";
    cout << "=========================================================================\n\n";

    map<string, double> exact_opt_map = load_exact_optimal_reference();
    string dataset_dir = "dataset";
    vector<string> testcase_files = get_testcase_files(dataset_dir);
    if (testcase_files.empty()) {
        dataset_dir = "../dataset";
        testcase_files = get_testcase_files(dataset_dir);
    }

    if (testcase_files.empty()) {
        cerr << "Error: Tidak ditemukan file testcase di direktori " << dataset_dir << "/\n";
        return 1;
    }

    string results_dir = (dataset_dir == "../dataset") ? "../results" : "results";

    cout << "Total testcase ditemukan: " << testcase_files.size() << " file.\n";

    vector<pair<string, string>> methods = {
        {"SERDA", "SERDA.exe"},
        {"IHW",   "IHW.exe"},
        {"CSM",  "CSM.exe"},
        {"RBSM",  "RBSM.exe"},
        {"SAM",   "SAM.exe"},
        {"VAM",   "VAM.exe"},
        {"THP",   "THP.exe"},
        {"FVAM",  "FVAM.exe"}
    };

    cout << "Metode yang diuji (8 metode):\n";
    for (size_t i = 0; i < methods.size(); i++) {
        cout << "  " << (i + 1) << ". " << methods[i].first << " (" << methods[i].second << ")\n";
    }
    cout << "\nMemulai benchmarking...\n\n";

    vector<TestcaseRecord> records;

    for (size_t tc_idx = 0; tc_idx < testcase_files.size(); tc_idx++) {
        string tc_path = testcase_files[tc_idx];
        string bname = get_basename(tc_path);

        TestcaseRecord rec;
        rec.filename = bname;

        // Read m, n and file reference cost
        ifstream in(tc_path);
        in >> rec.m >> rec.n;
        double dummy_val;
        for (int i = 0; i < rec.m * rec.n; i++) in >> dummy_val;
        for (int i = 0; i < rec.m; i++) in >> dummy_val;
        for (int j = 0; j < rec.n; j++) in >> dummy_val;
        double file_ref = 0.0;
        in >> file_ref;
        in.close();

        // Exact verified optimal
        if (exact_opt_map.find(bname) != exact_opt_map.end()) {
            rec.optimal_cost = exact_opt_map[bname];
        } else {
            rec.optimal_cost = file_ref;
        }

        cout << "[" << setw(2) << (tc_idx + 1) << "/69] " << setw(14) << left << bname
             << " (" << rec.m << "x" << rec.n << ") Opt: " << setw(10) << fixed << setprecision(1) << rec.optimal_cost << " | ";

        for (auto &m : methods) {
            MethodResult res = run_method(m.second, tc_path, m.first, rec.optimal_cost);
            rec.results[m.first] = res;
            if (res.is_optimal) cout << m.first << "[*] ";
            else if (res.feasible) cout << m.first << "[" << setprecision(1) << res.gap_percent << "%] ";
            else cout << m.first << "[FAIL] ";
        }
        cout << "\n";
        records.push_back(rec);
    }

    cout << "\n=========================================================================\n";
    cout << "                         BENCHMARK SUMMARY TABLE                         \n";
    cout << "=========================================================================\n";

    map<string, MethodStats> stats;
    for (auto &m : methods) {
        MethodStats s;
        s.name = m.first;
        s.total_runs = records.size();
        s.successful_runs = 0;
        s.feasible_runs = 0;
        s.failed_runs = 0;
        s.optimal_hits = 0;
        s.avg_time_ms = 0.0;

        vector<double> gaps;
        double sum_time = 0.0;

        for (auto &rec : records) {
            auto it = rec.results.find(m.first);
            if (it != rec.results.end()) {
                MethodResult r = it->second;
                sum_time += r.time_ms;
                if (r.feasible && r.status == "SUCCESS") {
                    s.successful_runs++;
                    s.feasible_runs++;
                    if (r.is_optimal) s.optimal_hits++;
                    gaps.push_back(r.gap_percent);
                } else {
                    s.failed_runs++;
                }
            } else {
                s.failed_runs++;
            }
        }

        s.accuracy_percent = (s.total_runs > 0) ? ((double)s.optimal_hits / s.total_runs) * 100.0 : 0.0;
        s.avg_time_ms = (s.total_runs > 0) ? (sum_time / s.total_runs) : 0.0;

        if (!gaps.empty()) {
            sort(gaps.begin(), gaps.end());
            double sum_gap = 0.0;
            for (double g : gaps) sum_gap += g;
            s.mean_gap = sum_gap / gaps.size();
            s.min_gap = gaps.front();
            s.max_gap = gaps.back();
            if (gaps.size() % 2 == 1) s.median_gap = gaps[gaps.size() / 2];
            else s.median_gap = (gaps[gaps.size() / 2 - 1] + gaps[gaps.size() / 2]) / 2.0;

            double sq_sum = 0.0;
            for (double g : gaps) sq_sum += (g - s.mean_gap) * (g - s.mean_gap);
            s.std_gap = sqrt(sq_sum / gaps.size());
        } else {
            s.mean_gap = 0.0;
            s.median_gap = 0.0;
            s.min_gap = 0.0;
            s.max_gap = 0.0;
            s.std_gap = 0.0;
        }

        stats[m.first] = s;
    }

    // Print summary table
    cout << setw(8)  << left  << "Method"
         << setw(14) << right << "Optimal Hits"
         << setw(16) << right << "Total TC"
         << setw(14) << right << "Accuracy(%)"
         << setw(14) << right << "Mean Gap(%)"
         << setw(14) << right << "Median Gap(%)"
         << setw(14) << right << "Max Gap(%)"
         << setw(14) << right << "Avg Time(ms)" << "\n";
    cout << string(98, '-') << "\n";

    for (auto &m : methods) {
        MethodStats s = stats[m.first];
        cout << setw(8)  << left  << s.name
             << setw(14) << right << (to_string(s.optimal_hits) + "/" + to_string(s.total_runs))
             << setw(16) << right << s.total_runs
             << setw(14) << right << fixed << setprecision(2) << s.accuracy_percent
             << setw(14) << right << fixed << setprecision(2) << s.mean_gap
             << setw(14) << right << fixed << setprecision(2) << s.median_gap
             << setw(14) << right << fixed << setprecision(2) << s.max_gap
             << setw(14) << right << fixed << setprecision(4) << s.avg_time_ms << "\n";
    }
    cout << string(98, '=') << "\n\n";

    // Pastikan folder results/ tersedia
    ensure_directory(results_dir);

    // Write results/benchmark_summary.csv
    ofstream sum_csv(results_dir + "/benchmark_summary.csv");
    sum_csv << "Method,Optimal_Hits,Total_Testcases,Accuracy_Percent,Mean_Gap_Percent,Median_Gap_Percent,Min_Gap_Percent,Max_Gap_Percent,Std_Dev_Gap_Percent,Avg_Time_Ms,Feasible_Runs,Failed_Runs\n";
    for (auto &m : methods) {
        MethodStats s = stats[m.first];
        sum_csv << s.name << ","
                << s.optimal_hits << ","
                << s.total_runs << ","
                << fixed << setprecision(2) << s.accuracy_percent << ","
                << s.mean_gap << ","
                << s.median_gap << ","
                << s.min_gap << ","
                << s.max_gap << ","
                << s.std_gap << ","
                << setprecision(4) << s.avg_time_ms << ","
                << s.feasible_runs << ","
                << s.failed_runs << "\n";
    }
    sum_csv.close();

    // Write results/benchmark_details.csv
    ofstream det_csv(results_dir + "/benchmark_details.csv");
    det_csv << "Testcase,Dimensions,Optimal_Cost";
    for (auto &m : methods) {
        det_csv << "," << m.first << "_Cost," << m.first << "_Gap_Percent," << m.first << "_Optimal";
    }
    det_csv << "\n";

    for (auto &rec : records) {
        det_csv << rec.filename << "," << rec.m << "x" << rec.n << "," << fixed << setprecision(2) << rec.optimal_cost;
        for (auto &m : methods) {
            MethodResult r = rec.results[m.first];
            det_csv << "," << fixed << setprecision(2) << r.cost
                    << "," << fixed << setprecision(2) << r.gap_percent
                    << "," << (r.is_optimal ? "1" : "0");
        }
        det_csv << "\n";
    }
    det_csv.close();

    // Write results/benchmark_report.txt
    ofstream rpt(results_dir + "/benchmark_report.txt");
    rpt << "=========================================================================\n";
    rpt << "         LAPORAN BENCHMARK METODE TRANSPORTATION PROBLEM (69 TC)         \n";
    rpt << "=========================================================================\n\n";
    rpt << "Metode yang dievaluasi:\n";
    rpt << "1. SERDA (Dynamic Mean-Opportunity Vogel's Approximation Method)\n";
    rpt << "2. IHW   (Inverse Harmonic Weight / RCWMCAM-TOCM)\n";
    rpt << "3. CSM  (Cost-Supply Method v2)\n";
    rpt << "4. RBSM  (Row-Based Substitution Method)\n";
    rpt << "5. SAM   (Supply Average Method)\n";
    rpt << "6. VAM   (Vogel's Approximation Method)\n";
    rpt << "7. THP   (Two Highest Penalties Method)\n";
    rpt << "8. FVAM  (Fractional Vogel's Approximation Method)\n\n";

    rpt << "RINGKASAN HASIL BENCHMARK:\n";
    rpt << string(98, '-') << "\n";
    rpt << setw(8)  << left  << "Method"
        << setw(14) << right << "Optimal Hits"
        << setw(16) << right << "Total TC"
        << setw(14) << right << "Accuracy(%)"
        << setw(14) << right << "Mean Gap(%)"
        << setw(14) << right << "Median Gap(%)"
        << setw(14) << right << "Max Gap(%)"
        << setw(14) << right << "Avg Time(ms)" << "\n";
    rpt << string(98, '-') << "\n";

    for (auto &m : methods) {
        MethodStats s = stats[m.first];
        rpt << setw(8)  << left  << s.name
            << setw(14) << right << (to_string(s.optimal_hits) + "/" + to_string(s.total_runs))
            << setw(16) << right << s.total_runs
            << setw(14) << right << fixed << setprecision(2) << s.accuracy_percent
            << setw(14) << right << fixed << setprecision(2) << s.mean_gap
            << setw(14) << right << fixed << setprecision(2) << s.median_gap
            << setw(14) << right << fixed << setprecision(2) << s.max_gap
            << setw(14) << right << fixed << setprecision(4) << s.avg_time_ms << "\n";
    }
    rpt << string(98, '-') << "\n\n";

    rpt << "DETAIL PER TESTCASE (Optimal vs Method Costs):\n";
    rpt << setw(14) << left << "Testcase"
        << setw(10) << right << "Opt_Cost"
        << setw(10) << right << "SERDA"
        << setw(10) << right << "IHW"
        << setw(10) << right << "CSM"
        << setw(10) << right << "RBSM"
        << setw(10) << right << "SAM"
        << setw(10) << right << "VAM"
        << setw(10) << right << "THP"
        << setw(10) << right << "FVAM" << "\n";
    rpt << string(94, '-') << "\n";

    for (auto &rec : records) {
        rpt << setw(14) << left << rec.filename
            << setw(10) << right << fixed << setprecision(0) << rec.optimal_cost;
        for (auto &m : methods) {
            MethodResult r = rec.results[m.first];
            string marker = r.is_optimal ? "*" : "";
            rpt << setw(9) << right << fixed << setprecision(0) << r.cost << marker;
        }
        rpt << "\n";
    }
    rpt << "\nCatatan: Tanda '*' menandakan bahwa metode berhasil mencapai biaya optimal.\n";
    rpt.close();

    cout << "Laporan berhasil dibuat di folder " << results_dir << "/:\n";
    cout << "  - " << results_dir << "/benchmark_summary.csv\n";
    cout << "  - " << results_dir << "/benchmark_details.csv\n";
    cout << "  - " << results_dir << "/benchmark_report.txt\n\n";

    return 0;
}

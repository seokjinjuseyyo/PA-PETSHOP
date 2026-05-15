//   SISTEM MANAJEMEN TOKO PERLENGKAPAN HEWAN "WHINGKY"
//   File     : main.cpp
//   Deskripsi: Entry point -- variabel global, utilitas,
//              pencarian, pengurutan, tampilan, inisialisasi

#include "toko.h"
#include <cstdio>

//  DEFINISI VARIABEL GLOBAL
Produk   daftarProduk[MAX_PRODUK];
int      jumlahProduk = 0;

Pengguna daftarPengguna[MAX_USER];
int      jumlahPengguna = 0;

Riwayat  daftarRiwayat[MAX_RIWAYAT];
int      jumlahRiwayat = 0;


//  FITUR: Sleep lintas platform
static void sleepMs(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}


//  FITUR: Clear terminal lintas platform
void clearLayar() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


//  FITUR: Input Password (karakter disembunyikan jadi '*')
string inputPassword(const string& prompt) {
    while (true) {
        cout << CYAN << "  " << prompt << RESET;
        string pass = "";

#ifdef _WIN32
        char ch;
        while (true) {
            ch = (char)_getch();
            if (ch == '\r' || ch == '\n') break;
            if (ch == '\b' || ch == 127) {
                if (!pass.empty()) { pass.pop_back(); cout << "\b \b" << flush; }
            } else if ((unsigned char)ch >= 32) {
                pass += ch;
                cout << '*' << flush;
            }
        }
        cout << "\n";
#else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            if (c == 127 || c == '\b') {
                if (!pass.empty()) { pass.pop_back(); cout << "\b \b" << flush; }
            } else if (c >= 32 && c < 127) {
                pass += (char)c;
                cout << '*' << flush;
            }
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        cout << "\n";
#endif

        if (pass.empty()) { cetakPesan("err", "Password tidak boleh kosong."); continue; }
        return pass;
    }
}
void animasiLoading(const string& pesan, int langkah = 3) {
    const char spinner[] = {'|', '/', '-', '\\'};
    for (int i = 0; i < langkah * 4; i++) {
        cout << "\r  " << BCYAN << spinner[i % 4] << RESET
             << "  " << CYAN << pesan << RESET << "   " << flush;
        sleepMs(80);
    }
    cout << "\r" << string(pesan.size() + 10, ' ') << "\r";
}


//  FITUR: Animasi ketik teks karakter per karakter
void animasiKetik(const string& teks, int delayMs = 18) {
    for (char c : teks) {
        cout << c << flush;
        sleepMs(delayMs);
    }
}


//  FITUR: Penanganan Error
void lemparError(const string& pesan) {
    throw runtime_error(pesan);
}


//  FITUR: Input Aman -- Angka Bulat (Integer)
//         Menolak huruf, float, dan karakter aneh.
//         Loop sampai input valid, TIDAK infinite jika EOF.
int inputAngkaAman() {
    while (true) {
        cout << CYAN << " > " << RESET;

        // Jika stream sudah EOF / rusak, reset dan beri pesan
        if (cin.eof()) {
            cin.clear();
            cetakPesan("err", "Tidak ada input. Masukkan angka.");
            continue;
        }

        string baris;
        if (!getline(cin, baris)) {
            cin.clear();
            cetakPesan("err", "Gagal membaca input. Coba lagi.");
            continue;
        }

        // Buang spasi di awal & akhir
        size_t awal = baris.find_first_not_of(" \t\r\n");
        if (awal == string::npos) {
            cetakPesan("err", "Input tidak boleh kosong. Masukkan angka.");
            continue;
        }
        baris = baris.substr(awal);
        size_t akhir = baris.find_last_not_of(" \t\r\n");
        if (akhir != string::npos) baris = baris.substr(0, akhir + 1);

        // Validasi: hanya digit (boleh diawali '-' untuk negatif)
        bool valid = true;
        size_t mulai = 0;
        if (!baris.empty() && (baris[0] == '-' || baris[0] == '+')) mulai = 1;
        if (mulai >= baris.size()) valid = false;
        for (size_t i = mulai; i < baris.size() && valid; i++) {
            if (!isdigit((unsigned char)baris[i])) valid = false;
        }

        if (!valid) {
            cetakPesan("err", "Input kamu tidak valid. Masukkan angka bulat.");
            continue;
        }

        // Cek overflow
        try {
            long long ll = stoll(baris);
            if (ll > 2147483647LL || ll < -2147483648LL) {
                cetakPesan("err", "Angkanya terlalu besar/kecil.");
                continue;
            }
            return (int)ll;
        } catch (...) {
            cetakPesan("err", "Angka terlalu besar. Coba lagi.");
        }
    }
}


//  FITUR: Input Angka dalam Rentang [minVal, maxVal]
int inputAngkaRentang(int minVal, int maxVal) {
    while (true) {
        int val = inputAngkaAman();
        if (val >= minVal && val <= maxVal) return val;
        cetakPesan("err", "Masukkan angka antara " + to_string(minVal)
                         + " hingga " + to_string(maxVal) + ".");
    }
}


//  FITUR: Input Aman -- Angka Desimal (Double)
double inputDoubleAman() {
    while (true) {
        cout << CYAN << " > " << RESET;

        if (cin.eof()) { cin.clear(); cetakPesan("err", "Tidak ada input."); continue; }

        string baris;
        if (!getline(cin, baris)) {
            cin.clear();
            cetakPesan("err", "Gagal membaca input. Coba lagi.");
            continue;
        }

        size_t awal = baris.find_first_not_of(" \t\r\n");
        if (awal == string::npos) {
            cetakPesan("err", "Input tidak boleh kosong.");
            continue;
        }
        baris = baris.substr(awal);

        // Validasi: digit, titik, koma (sebagai desimal), tanda minus
        bool valid = true;
        int titikCount = 0;
        size_t mulai = 0;
        if (!baris.empty() && (baris[0] == '-' || baris[0] == '+')) mulai = 1;
        if (mulai >= baris.size()) valid = false;
        for (size_t i = mulai; i < baris.size() && valid; i++) {
            if (baris[i] == '.' || baris[i] == ',') { titikCount++; if (titikCount > 1) valid = false; }
            else if (!isdigit((unsigned char)baris[i])) valid = false;
        }

        if (!valid) {
            cetakPesan("err", "Input tidak valid. Masukkan angka (contoh: 15000 atau 15000.5).");
            continue;
        }

        // Ganti koma dengan titik
        replace(baris.begin(), baris.end(), ',', '.');

        try {
            double val = stod(baris);
            return val;
        } catch (...) {
            cetakPesan("err", "Angka tidak valid. Coba lagi.");
        }
    }
}


//  FITUR: Input Aman -- Teks / String
string inputTeksAman(const string& prompt) {
    while (true) {
        cout << CYAN << "  " << prompt << RESET;
        string teks;
        if (!getline(cin, teks)) {
            cin.clear();
            cetakPesan("err", "Gagal membaca input. Coba lagi.");
            continue;
        }
        // Buang spasi di tepi
        size_t awal = teks.find_first_not_of(" \t\r\n");
        if (awal == string::npos) {
            cetakPesan("err", "Input tidak boleh kosong atau spasi saja.");
            continue;
        }
        teks = teks.substr(awal);
        size_t akhir = teks.find_last_not_of(" \t\r\n");
        if (akhir != string::npos) teks = teks.substr(0, akhir + 1);

        if (teks.empty()) {
            cetakPesan("err", "Input tidak boleh kosong.");
            continue;
        }
        return teks;
    }
}


//  FITUR: Validasi Data
void validasiStok(int stok) {
    if (stok < 0) lemparError("Stok tidak boleh negatif");
}

void validasiHarga(double harga) {
    if (harga <= 0) lemparError("Harga harus lebih dari 0");
}

void validasiPassword(const string& password) {
    if ((int)password.length() < 4)
        lemparError("Password minimal 4 karakter");
}


//  FITUR: Timestamp
string getTimestamp() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[80];
    sprintf(buffer, "%02d-%02d-%04d %02d:%02d:%02d",
            ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year,
            ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return string(buffer);
}


//  FITUR: Utilitas UI -- Teks Rata Tengah
//         CATATAN: hanya bekerja dengan karakter ASCII 1-byte
string pusatTeks(const string& teks, int lebar) {
    int panjang = (int)teks.length();
    if (panjang >= lebar) return teks.substr(0, lebar);
    int kiriPad  = (lebar - panjang) / 2;
    int kananPad = lebar - panjang - kiriPad;
    return string(kiriPad, ' ') + teks + string(kananPad, ' ');
}

void cetakGarisH(int lebar, char simbol, const string& warna) {
    cout << warna << "+" << string(lebar, simbol) << "+" << RESET << "\n";
}

void cetakHeader(const string& judul, int lebar, const string& warna) {
    cetakGarisH(lebar, '=', warna);
    cout << warna << "|" << BOLD << BWHITE
         << pusatTeks(judul, lebar)
         << RESET << warna << "|" << RESET << "\n";
    cetakGarisH(lebar, '=', warna);
}

void cetakPesan(const string& tipe, const string& pesan) {
    if (tipe == "ok")
        cout << BGREEN  << "  [OK]  " << RESET << GREEN  << pesan << RESET << "\n";
    else if (tipe == "err")
        cout << BRED    << "  [!!]  " << RESET << RED    << pesan << RESET << "\n";
    else if (tipe == "warn")
        cout << BYELLOW << "  [/!]  " << RESET << YELLOW << pesan << RESET << "\n";
    else
        cout << BCYAN   << "  [>>]  " << RESET << CYAN   << pesan << RESET << "\n";
}


//  FITUR: Tampilan Tabel Produk
static string namaKategori(KategoriHewan k) {
    switch (k) {
        case KUCING: return "Kucing";
        case ANJING: return "Anjing";
        case REPTIL: return "Reptil";
        case TERNAK: return "Ternak";
        default:     return "?";
    }
}
static string namaSubKat(SubKategori s) {
    switch (s) {
        case OBAT:         return "Obat";
        case MAKANAN:      return "Makanan";
        case AKSESORIS:    return "Aksesoris";
        case PERLENGKAPAN: return "Perlengkapan";
        default:           return "?";
    }
}

void tampilkanTabelProduk(const Produk arr[], int n) {
    if (n == 0) {
        cout << "\n";
        cetakGarisH(W_ID+W_NAMA+W_KAT+W_SUBKAT+W_HARGA+W_STOK+13, '-', YELLOW);
        cout << YELLOW << "|"
             << pusatTeks("Tidak ada produk yang ditemukan.",
                          W_ID+W_NAMA+W_KAT+W_SUBKAT+W_HARGA+W_STOK+13)
             << "|" << RESET << "\n";
        cetakGarisH(W_ID+W_NAMA+W_KAT+W_SUBKAT+W_HARGA+W_STOK+13, '-', YELLOW);
        return;
    }

    auto garis = [&]() {
    cout << BCYAN
             << "+" << string(W_ID+3,'-')
             << "+" << string(W_NAMA+2,'-')
             << "+" << string(W_KAT+2,'-')
             << "+" << string(W_SUBKAT+2,'-')
             << "+" << string(W_HARGA+2,'-')
             << "+" << string(W_STOK+2,'-')
             << "+" << RESET << "\n";
    };

    cout << "\n";
    garis();
    cout << BCYAN << "|" << RESET
         << BG_BLUE << BWHITE << setw(W_ID+3)    << right << " ID "    << RESET << BCYAN << "|" << RESET
         << BG_BLUE << BWHITE << " " << left << setw(W_NAMA+1)   << "Nama Produk"   << RESET << BCYAN << "|" << RESET
         << BG_BLUE << BWHITE << " " << left << setw(W_KAT+1)    << "Kategori"      << RESET << BCYAN << "|" << RESET
         << BG_BLUE << BWHITE << " " << left << setw(W_SUBKAT+1) << "Sub Kategori"  << RESET << BCYAN << "|" << RESET
         << BG_BLUE << BWHITE << " " << left << setw(W_HARGA+1)  << "Harga"         << RESET << BCYAN << "|" << RESET
         << BG_BLUE << BWHITE << " " << left << setw(W_STOK+1)   << "Stok"          << RESET << BCYAN << "|" << RESET << "\n";
    garis();

    for (int i = 0; i < n; i++) {
        const Produk& p = arr[i];
        string hargaStr = "Rp" + to_string((long long)p.harga);
        string stokStr  = to_string(p.stok);

        string namaDisplay = p.nama;
        if ((int)namaDisplay.size() > W_NAMA) namaDisplay = namaDisplay.substr(0, W_NAMA-1) + ".";

        cout << BCYAN << "|" << RESET
             << BWHITE  << setw(W_ID+2)    << right << p.id           << " " << RESET << BCYAN << "|" << RESET
             << " " << left << setw(W_NAMA+1)   << namaDisplay                             << BCYAN << "|" << RESET
             << " " << left << setw(W_KAT+1)    << namaKategori(p.kategori)                << BCYAN << "|" << RESET
             << " " << left << setw(W_SUBKAT+1) << namaSubKat(p.subKategori)               << BCYAN << "|" << RESET
             << BGREEN  << " " << left << setw(W_HARGA+1)  << hargaStr                    << RESET << BCYAN << "|" << RESET;

        // Warna stok: merah jika <= 5, kuning jika <= 10, hijau jika > 10
        if (p.stok <= 5)       cout << BRED;
        else if (p.stok <= 10) cout << BYELLOW;
        else                   cout << BGREEN;
        cout << " " << left << setw(W_STOK+1) << stokStr << RESET << BCYAN << "|" << RESET << "\n";
        garis();
    }
}


//  FITUR: Tampilan Keranjang Belanja
void tampilkanKeranjang(const vector<ItemKeranjang>& keranjang) {
    if (keranjang.empty()) {
        cetakPesan("warn", "Keranjang belanja masih kosong.");
        return;
    }

    const int W_NO  = 2;
    const int W_NM  = 27;
    const int W_JML = 6;
    const int W_SUB = 14;

    auto garis = [&]() {
        cout << GREEN << "+" << string(W_NO+2,'-') << "+" << string(W_NM+1,'-')
             << "+" << string(W_JML+3,'-') << "+" << string(W_SUB+2,'-')
             << "+" << RESET << "\n";
    };

    cout << "\n";
    garis();
    cout << GREEN << "|" << RESET
         << BG_GREEN << BWHITE << setw(W_NO) << right << " No " << RESET << GREEN << "|" << RESET
         << BG_GREEN << BWHITE << " " << left << setw(W_NM)  << "Nama Produk"  << RESET << GREEN << "|" << RESET
         << BG_GREEN << BWHITE << " " << left << setw(W_JML+2) << " Jumlah "          << RESET << GREEN << "|" << RESET
         << BG_GREEN << BWHITE << " " << left << setw(W_SUB+1) << "  Subtotal"     << RESET << GREEN << "|" << RESET << "\n";
    garis();

    double total = 0;
    for (size_t i = 0; i < keranjang.size(); i++) {
        string nm = keranjang[i].nama;
        if ((int)nm.size() > W_NM) nm = nm.substr(0, W_NM-1) + ".";
        string subStr = "Rp" + to_string((long long)keranjang[i].subtotal);

        cout << GREEN << "|" << RESET
             << setw(W_NO+1) << right << (i+1) << " " << GREEN << "|" << RESET
             << " " << left << setw(W_NM)  << nm     << GREEN << "|" << RESET
             << " " << left << setw(W_JML+2) << keranjang[i].jumlah << GREEN << "|" << RESET
             << BGREEN << " " << left << setw(W_SUB+1) << subStr << RESET << GREEN << "|" << RESET << "\n";
        garis();
        total += keranjang[i].subtotal;
    }

    string totalStr = "Rp" + to_string((long long)total);
    cout << GREEN << "|" << RESET
         << BYELLOW << pusatTeks("TOTAL BELANJA", W_NO+W_NM+W_JML+8) << RESET
         << GREEN << "|" << RESET
         << BGREEN << " " << left << setw(W_SUB+1) << totalStr << RESET << GREEN << "|" << RESET << "\n";
    cout << BGREEN << "+" << string(W_NO+W_NM+W_JML+8,'=') << "+" << string(W_SUB+2,'=') << "+"
         << RESET << "\n";
}


//  FITUR: Pencarian -- Binary Search berdasarkan ID
int cariBinaryByID(int idTarget) {
    int low = 0, high = jumlahProduk - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if      (daftarProduk[mid].id == idTarget) return mid;
        else if (daftarProduk[mid].id <  idTarget) low  = mid + 1;
        else                                        high = mid - 1;
    }
    return -1;
}


//  FITUR: Pencarian -- Linear Search berdasarkan Keyword
vector<int> cariLinearKeyword(const string& keyword) {
    vector<int> hasil;
    string kw = keyword;
    transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
    for (int i = 0; i < jumlahProduk; i++) {
        string nama = daftarProduk[i].nama;
        transform(nama.begin(), nama.end(), nama.begin(), ::tolower);
        if (nama.find(kw) != string::npos) hasil.push_back(i);
    }
    return hasil;
}


//  FITUR: Pengurutan
void bubbleSortHarga(Produk* arr, int n, UrutanHarga urutan) {
    bool swapped;
    for (int i = 0; i < n - 1; i++) {
        swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            bool perluTukar = (urutan == ASC) ?
                              (arr[j].harga > arr[j+1].harga) :
                              (arr[j].harga < arr[j+1].harga);
            if (perluTukar) { swap(arr[j], arr[j+1]); swapped = true; }
        }
        if (!swapped) break;
    }
}

void selectionSortNama(Produk* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++)
            if (arr[j].nama < arr[minIdx].nama) minIdx = j;
        if (minIdx != i) swap(arr[i], arr[minIdx]);
    }
}

void insertionSortStok(Produk* arr, int n) {
    for (int i = 1; i < n; i++) {
        Produk key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].stok > key.stok) { arr[j+1] = arr[j]; j--; }
        arr[j+1] = key;
    }
}

void filterKategoriDanSort(KategoriHewan kat, UrutanHarga urutan) {
    Produk temp[MAX_PRODUK];
    int count = 0;
    for (int i = 0; i < jumlahProduk; i++)
        if (daftarProduk[i].kategori == kat) temp[count++] = daftarProduk[i];
    bubbleSortHarga(temp, count, urutan);
    tampilkanTabelProduk(temp, count);
}


//  FITUR: Inisialisasi Data Awal
void init_data() {
    daftarPengguna[0] = {1, "admin",    "admin123",   "admin", "01-01-2025 00:00:00"};
    daftarPengguna[1] = {2, "user",     "user123",    "user",  "01-01-2025 00:00:00"};
    daftarPengguna[2] = {3, "syarifah", "2509106007", "admin", "01-01-2025 00:00:00"};
    jumlahPengguna = 3;

    daftarProduk[0]  = {1,  "Whiskas Kitten",       KUCING, MAKANAN,      45000,  20};
    daftarProduk[1]  = {2,  "Royal Canin Cat",       KUCING, MAKANAN,     125000,  15};
    daftarProduk[2]  = {3,  "Cat Litter Premium",    KUCING, PERLENGKAPAN, 35000,  30};
    daftarProduk[3]  = {4,  "Sisir Bulu Kucing",     KUCING, AKSESORIS,    15000,  25};
    daftarProduk[4]  = {5,  "Kalung Anti Kutu",      KUCING, OBAT,         55000,  18};
    daftarProduk[5]  = {6,  "Vitamin Cat Plus",      KUCING, OBAT,         40000,  22};
    daftarProduk[6]  = {7,  "Scratching Post",       KUCING, PERLENGKAPAN, 85000,  10};
    daftarProduk[7]  = {8,  "Mainan Tikus Elektrik", KUCING, AKSESORIS,    65000,  12};
    daftarProduk[8]  = {9,  "Shampoo Kucing",        KUCING, PERLENGKAPAN, 28000,  35};
    daftarProduk[9]  = {10, "Cat Carrier",           KUCING, PERLENGKAPAN,150000,   8};
    daftarProduk[10] = {11, "Grooming Kit",          KUCING, AKSESORIS,    95000,  14};
    daftarProduk[11] = {12, "Pro Plan Cat",          KUCING, MAKANAN,     135000,  16};

    daftarProduk[12] = {13, "Pedigree Adult",        ANJING, MAKANAN,     120000,  15};
    daftarProduk[13] = {14, "Royal Canin Dog",       ANJING, MAKANAN,     185000,  12};
    daftarProduk[14] = {15, "Kalung Anjing Premium", ANJING, AKSESORIS,    35000,  25};
    daftarProduk[15] = {16, "Tali Anjing",           ANJING, AKSESORIS,    45000,  20};
    daftarProduk[16] = {17, "Rumah Anjing",          ANJING, PERLENGKAPAN,350000,   5};
    daftarProduk[17] = {18, "Shampoo Anjing",        ANJING, PERLENGKAPAN, 32000,  28};
    daftarProduk[18] = {19, "Vitamin Dog Plus",      ANJING, OBAT,         50000,  18};
    daftarProduk[19] = {20, "Sikat Gigi Anjing",     ANJING, AKSESORIS,    25000,  30};
    daftarProduk[20] = {21, "Mainan Anjing Karet",   ANJING, AKSESORIS,    22000,  40};
    daftarProduk[21] = {22, "Dog Bed",               ANJING, PERLENGKAPAN,175000,  10};
    daftarProduk[22] = {23, "Grooming Set",          ANJING, AKSESORIS,   125000,   8};
    daftarProduk[23] = {24, "Dog Treats",            ANJING, MAKANAN,      35000,  45};

    daftarProduk[24] = {25, "UVB Lamp Reptile",      REPTIL, PERLENGKAPAN,250000,   5};
    daftarProduk[25] = {26, "Terrarium Glass",       REPTIL, PERLENGKAPAN,180000,   8};
    daftarProduk[26] = {27, "Pakan Jangkrik",        REPTIL, MAKANAN,      15000,  50};
    daftarProduk[27] = {28, "Substrate Reptile",     REPTIL, PERLENGKAPAN, 45000,  20};
    daftarProduk[28] = {29, "Thermometer Digital",   REPTIL, AKSESORIS,    35000,  15};
    daftarProduk[29] = {30, "Vitamin Reptile",       REPTIL, OBAT,         55000,  12};
    daftarProduk[30] = {31, "Tempat Minum Reptil",   REPTIL, AKSESORIS,    25000,  25};
    daftarProduk[31] = {32, "Heating Pad",           REPTIL, PERLENGKAPAN, 85000,  10};
    daftarProduk[32] = {33, "Dekorasi Terrarium",    REPTIL, AKSESORIS,    65000,  15};
    daftarProduk[33] = {34, "Pakan Ulat Hongkong",   REPTIL, MAKANAN,      20000,  40};
    daftarProduk[34] = {35, "Spray Bottle",          REPTIL, PERLENGKAPAN, 18000,  30};
    daftarProduk[35] = {36, "Cleaning Kit Reptil",   REPTIL, PERLENGKAPAN, 45000,  18};

    daftarProduk[36] = {37, "Pakan Ayam Broiler",    TERNAK, MAKANAN,      85000,  50};
    daftarProduk[37] = {38, "Vitamin Ternak Plus",   TERNAK, OBAT,         55000,  40};
    daftarProduk[38] = {39, "Kandang Portable",      TERNAK, PERLENGKAPAN,450000,   5};
    daftarProduk[39] = {40, "Tempat Minum Otomatis", TERNAK, PERLENGKAPAN,125000,  15};
    daftarProduk[40] = {41, "Pakan Bebek",           TERNAK, MAKANAN,      75000,  45};
    daftarProduk[41] = {42, "Obat Cacing Ternak",    TERNAK, OBAT,         35000,  35};
    daftarProduk[42] = {43, "Timbangan Ternak",      TERNAK, AKSESORIS,   275000,   8};
    daftarProduk[43] = {44, "Pakan Kambing",         TERNAK, MAKANAN,      95000,  30};
    daftarProduk[44] = {45, "Suplemen Susu",         TERNAK, OBAT,         65000,  25};
    daftarProduk[45] = {46, "Alat Suntik Veteriner", TERNAK, PERLENGKAPAN, 45000,  20};
    daftarProduk[46] = {47, "Pakan Ikan",            TERNAK, MAKANAN,      55000,  60};
    daftarProduk[47] = {48, "Jaring Ternak",         TERNAK, AKSESORIS,    85000,  12};

    jumlahProduk = 48;
}


//  ============================================================
//  FITUR: Animasi "TERIMA KASIH" -- ASCII kucing + judul + lirik
//  Diadaptasi dari animasi_terima_kasih.cpp
//  ============================================================

//  Fungsi warna lokal (tidak konflik karena scope-nya static/lokal)
static void _col(const char* code) { cout << code; }

//  Judul blok "TERIMA KASIH"
static const vector<string> _TITLE = {
    "  \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2557\u2588\u2588\u2588\u2557   \u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2588\u2557    \u2588\u2588\u2557  \u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2557\u2588\u2588\u2557  \u2588\u2588\u2557         ",
    "     \u2588\u2588\u2554\u2550\u2550\u255d\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255d\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557   \u2588\u2588\u2551 \u2588\u2588\u2554\u255d\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255d\u2588\u2588\u2551\u2588\u2588\u2551  \u2588\u2588\u2551         ",
    "     \u2588\u2588\u2551   \u2588\u2588\u2588\u2588\u2588\u2557  \u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255d\u2588\u2588\u2551\u2588\u2588\u2554\u2588\u2588\u2588\u2588\u2554\u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2551   \u2588\u2588\u2588\u2588\u2588\u2554\u255d \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2551         ",
    "     \u2588\u2588\u2551   \u2588\u2588\u2554\u2550\u2550\u255d  \u2588\u2588\u2554\u2550\u2550\u2588\u2557 \u2588\u2588\u2551\u2588\u2588\u2551\u255a\u2588\u2588\u2554\u255d\u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2551   \u2588\u2588\u2554\u2550\u2588\u2588\u2557 \u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2551\u255a\u2550\u2550\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2551         ",
    "     \u2588\u2588\u2551   \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2551  \u2588\u2588\u2557\u2588\u2588\u2551\u2588\u2588\u2551 \u255a\u2550\u255d \u2588\u2588\u2551\u2588\u2588\u2551  \u2588\u2588\u2551   \u2588\u2588\u2551  \u2588\u2588\u2557\u2588\u2588\u2551  \u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2551\u2588\u2588\u2551\u2588\u2588\u2551  \u2588\u2588\u2551         ",
    "     \u255a\u2550\u255d   \u255a\u2550\u2550\u2550\u2550\u2550\u2550\u255d\u255a\u2550\u255d  \u255a\u2550\u255d\u255a\u2550\u255d\u255a\u2550\u255d     \u255a\u2550\u255d\u255a\u2550\u255d  \u255a\u2550\u255d   \u255a\u2550\u255d  \u255a\u2550\u255d\u255a\u2550\u255d  \u255a\u2550\u255d\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u255d\u255a\u2550\u255d\u255a\u2550\u255d  \u255a\u2550\u255d         ",
};

//  Lirik Kicau Mania
static const vector<string> _LYRICS = {
    "  Kicau....  Kicau.....  Kicau Mania  (Hemmmmm)  ",
    "  Kicau....  Kicau.....  Kicau Mania  (Hemmmmm)  ",
    "  Kicau....  Kicau.....  Kicau Mania  (Hemmmmm)  ",
    "  Kicau....  Kicau.....  Kicau Mania  (Hemmmmm)  ",
};

//  16 Frame ASCII Kucing
static const int _CAT_N = 16;
static const int _CAT_L = 26;
static const char* _CAT[16][26] = {
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@%%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%%#**##*****%%@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@#########%%%%##*+*#***%@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@#+**#%%%#%#%####%#*+==%@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@#+***####%#**###*++=#@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@#+*#####*+=+*#**+==@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#######=--=++*##+#@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%@%*###*---==+**=+@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#+=+*#*---+#*=-:#@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@%+-----=======----*@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@=-----=+++++==-----#@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@*===-=+**+++==----:=@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@##**++++=====----:-%@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@#**+=========-=+%@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%@#*#*+=====+*+=+@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@#*##*+===++++--@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%=+******###=-:*@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%=+*%@@@@@@@#--:*@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#+=*#@@@@@@@@@#+=#@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%==*%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%##********#@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@%*####****##%####*++#%%@@@@@@@@@@@@@@@@@@@","@@@@@@@@%*+*###%##%%%#######**++%@@@@@@@@@@@@@@@@@","@@@@@@@@@@#+**####%%#*#####**++=*@@@@@@@@@@@@@@@@@","@@@@@@@@@@@#+*######*+*##**++*+=%@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%*###%*=--=******+-*@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%@%####+----==*###*+%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%+#%#+---=++**+==%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#+=*##+===+**---:-#@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@%+-----=====---:----*@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@#-=----=++++==-----:+@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@%**=:--=+++++=-----:-%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@+-===+++++=------%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@***+=++++++=---*@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%*#*+++++***=-+@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%=+**+++***=-:*@%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@*=**#%%%@%+--=#@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%+=*#@@@@@@%+-:-*@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%==*%@@@@@@@@%*==*@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%#%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@%%%%%@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%#*#****+*#%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%%%%#***######**+***##%@@@@@@@@@@@@@@","@@@@@@@@@@@@***#############%##*+=-#@@@@@@@@@@@@@@","@@@@@@@@@@@@%++*########*####*+++==%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@*+**####*++*#***+++-*@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#+####*=--=****##+=%@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%**##+----==+##*=*@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%@#*##+---=+**==--=%@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@*=+#*=-=+++-:----+%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#=-==+++===---------#@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@+:=+*#**+++=-------:+@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#--=+++++====------:=@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@#*#**+=======-----:+@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@***+=======+==-=%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@%*##*+=====+***+=%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@%+***+==-==+++=--%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@#==+###***###=---%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@*-=+#@@@@@@@@@#----#@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@++*%@@@@@@@@@@@#--::#@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%*+*%@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@%%@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@%%##*****#%%%@%%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@#***######*++***+=*@@@@@@@@@@@@","@@@@@@@@@@@@@@#####**#########%#*+++-#@@@@@@@@@@@@","@@@@@@@@@@@@@@*=+*########*####*+++==@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#+++*####*++*##***++-+@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%++**##*=--=+***##*=#@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%##****-:----+*#*==#@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@#****-:--+*#+=---=%@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%+*##+--=++=------+%@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@#+==+++===----------+@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%==+++****++=---------#@@@@@@@@@@","@@@@@@@@@@@@@@@@@@==+++***+=====------:+@@@@@@@@@@","@@@@@@@@@@@@@@@@@@#==+++===---====-----#@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%%%#*+=-------==+=-+@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@%***+=----=+**+=#@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@%****+----=++*+-%@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@#*++==---====-*@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@#---+#*#*--:=@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@%+--=*%@@@@*=-:=%@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@%++*%@@@@@@@#+=-*@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@%@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%##*##*****%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@#**##****#####**+=*##++#@@@@@@@@@@@@@","@@@@@@@@@@@@@#=++*#############*+++-*@@@@@@@@@@@@@","@@@@@@@@@@@@@@*+++*#####*+*###**++==@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@*+***##*==-+*****++-#@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@**#*##+---=++**##=+@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@#****=::---=**#*+%@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%+*##=:--+*#+==-=%@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%+=-=++=--====-:--=%@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@#::-==++====--------*@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@-:--=*#+=====--------%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@+===++*+----==------:+@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%%@#++------==-----:=@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@***=------==----:*@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%*#*+=-----=+++==-%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@*+##*+=---==****+=@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@*-++++=---==++++-*@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@#===+#%##*+*#*=---%@%@@@@@@@@@@@@@","@@@@@@@@@@@@@@%=-=+#%@@@@@@@@@#--:=%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%**#%@@@@@@@@@@@@*--:+@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%%%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@%%#####%%@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@%###%%****####*++*#%%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@#++**###########*+++***+#@@@@@@@@@@@@@@@","@@@@@@@@@@@*+**#######**#####*++=-#@@@@@@@@@@@@@@@","@@@@@@@@@@@%++**#####*++###**+++-#@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#+*#####=--=+****++==@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%##**#*-:--=++*##*-%@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%@%****=::---=**#*+#@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%+*##+---+##+==-+@%@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%#*=-=====-===--:-:+@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@#-::-==+++++==-------#@@@@@@@@@@@@@@@@@@","@@@@@@@@@@+:--=+***++++=------:=%@@@@@@@@@@@@@@@@@","@@@@@@@@@@%=-==+***++===-------:#@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%%%#*+=+=======-----:#@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%**+=========---:=@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%*#*+======+**+-+@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@#*##*+====++**=-%@%@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%+***++==+**==--#@%@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@+=+*#%##%@@*--:=%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%+=+*#@@@@@@@@*--:-#@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%==+#%@@@@@@@@@@#=--+@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%%%@@@@@@@@@@@@@@%%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%##**#****+*%@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@%*####***###%###*++*###%%@@@@@@@@@@@@@@@@@","@@@@@@@@%*+**##%###########**+==*@@@@@@@@@@@@@@@@@","@@@@@@@@@@*+**#####%#**####*+++-#@@@@@@@@@@@@@@@@@","@@@@@@@@@@@*+*#####*+=+##**+++==@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%*#####*---=+*****+-#@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@#####+-----=+###+*@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%@#*#%#+---=+**+=-=@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%+=***=-==+*+-:::=%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@%+-----:::-===------*@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@=====--::::-++=----:=%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@++++++=-:--+*+=-----:*@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@###**+==++++++-----:*@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@#**++++++++==----%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%***++++++***+==%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%*#*+++++++*++-#@%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%++**++++**==-+@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#=+*#%%%@%+--+@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%+=+*%@@@@@#--:+@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@+=*#%@@@@@@@*=-+@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%%@@@@@@@@@@@%%@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%%#####*#@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%%@%#***####**+*#####%@@@@@@@@@@@@@@@","@@@@@@@@@@@#**##*####%%######*++==%@@@@@@@@@@@@@@@","@@@@@@@@@@@%++*##%###%#*#####*++=+@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@*+**#####+++##***++=+@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@*+#####=--=+***#**=%@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#####*-----=+*##+*@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%###*----+**+=--*@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@#=+**=-==+*=::---#@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@*----=++==--------=%@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@*:--=+***++==------:+@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@+---+***++++=--------%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%+++**++++=====------%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%%**++===========-+@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%*##*+=====+***+-#@%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@#+###*++===++++-:+@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@++****+++*###=---*%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@*==*%@@%%%@@@@%+--:-*@@@@@@@@@@@@","@@@@@@@@@@@@@@@@*-+*%@@@@@@@@@@@@*==-+%@@@@@@@@@@@","@@@@@@@@@@@@@@@@%#%@@@@@@@@@@@@@@@@%%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@%%%%%%%@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@%###****++*##%##%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@#***########**++==#@@@@@@@@@@@@@","@@@@@@@@@@@@@#**##*############*+++=#@@@@@@@@@@@@@","@@@@@@@@@@@@@%++**######*+###**++*+-%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#++*####+==+*******==@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@#+*###*----+++*#%#+*@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%#****-:----+*#*===*%@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@**#*=:--+##+-----=*%@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%@%+**+--===----==--=*@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@#---=======---------#@@@@@@@@@@","@@@@@@@@@@@@@@@@@%=::-++**++==---------+@@@@@@@@@@","@@@@@@@@@@@@@@@@@%:::-++**======--------%@@@@@@@@@","@@@@@@@@@@@@@@@@@@#==+++++==--========-+@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@%%@#**=-----==+++=-#@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@#*#*+----==***+=%@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@%*+++=---==+++==@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@%+-=+**+**==-:#@%@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@#=-=+#@@@@@#=--=%@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@+-+#@@@@@@@@#=-:=@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@%%@@@@@@@@@@@%*=*@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@%%####***#%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@%#***#####**++*###**@@@@@@@@@@@@","@@@@@@@@@@@@@#####***##%%########*++=+@@@@@@@@@@@@","@@@@@@@@@@@@@*=++*######%#*#####*+++-*@%@@@@@@@@@@","@@@@@@@@@@@@@@#=+++*#####+++##***+++-%@@@@@@@@@@@@","@@@@@@@@@@@@@@@#=+***##*=--=+******=+@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@#+***##*----==+*#%*=#@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@#****+-:---=+**+--#@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%@%+*##*---=*#*-----+%@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@*=++=---==--------*@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@+:-----====-------=%@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@*::--==+++=--------:+@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%-::--=+++++==--------%@@@@@@@@@@","@@@@@@@@@@@@@@@@@%-:-==+=========---=:+@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@%+-+**+=---===++==-=%@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@#=+**=---==+***==@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@+-==++===+++==-%@%@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@%+-=+%@%%%%*--:+@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@+--=*%@@@@@@@+--:*@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@#*#%@@@@@@@@@@+--:#@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#++%@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%%######***%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%###%%****#####**++##%###@@@@@@@@@@@@@","@@@@@@@@@@@@%+=+*##############**++==%@@@@@@@@@@@@","@@@@@@@@@@@@@%+=++*######**####*+++-*@@@@@@@@@@@@@","@@@@@@@@@@@@@@%+++**###*+=+##**+++=-%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%+***##*---=******+-*@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%#****=::--==+###++%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@#+***=:--=+***+=-#@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#+=-+##+---+**=----#@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@+-=++++++===--------*@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%==+++*#*++++==-------%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@+=++++**=====-------:+@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%##**+==---==------:=@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%*#*=-----==-----:=@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@**##*=----==++==--%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@++##*+----=+****-+@%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%==+***+====+++=-:=@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%+-=+*%@@%%##%%%%+--:+@@@@@@@@@@@@@@","@@@@@@@@@@@@@@#=+*%@@@@@@@@@@@@@+--:*@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%@@@@@@@@@@@@@@@%*++%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%%%%%%%@@%@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@%%%@%####*#***++*%@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@%**###***#######**=+##%%@@@@@@@@@@@@@@@@@@","@@@@@@@@@#+*###%############**+++%@@@@@@@@@@@@@@@@","@@@@@@@@@@#+**#######**#####*++==@@@@@@@@@@@@@@@@@","@@@@@@@@@@@#+*#####*+==*#***+++-%@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@######*----+*****+-+@%@@@@@@@@@@@@@@@@","@@@@@@@@@@@%@%##**+-:---=+*#%*=%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@#*###+::--=+***=-#@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@%++==++**=--+**=---=%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@#--=====+*+==---:----#@@@@@@@@@@@@@@@@@@@","@@@@@@@@@+-====+*****+=------:+@@@@@@@@@@@@@@@@@@@","@@@@@@@@@%***+++****++=----:::=@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%%%#*++++++++-----:-%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@#**++++++++----:=@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@#*#*++++++**+=-+%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%+***++++****=+@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#=+****#+===*%@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#=+*%@@@*=-=#@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@*-+#%@@@@%*--:+@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%#%@@@@@@@@#+=+%@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@%%%%####@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%%%%%##***##***++#%%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%***#############*+**++#@@@@@@@@@@@@@@@","@@@@@@@@@@@@%++*#######**###%#+===*@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%++*#####*=+##**++++=%@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%+*####=---+****++-*@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%####*-:--==+*#%*=%@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%*##+-:--=+***+=#@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%%*+*#*---+*#+----#@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@*=------::-+=-:----+@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#-==---:::::=+=-----=%@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#+++++=-:::-==---::-:*@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%##%#*+===+==-----::+@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%**+++====-----:+@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%*#*++====+=----#@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@#*##++===+****=:*@%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@#=+**++==+++=--:-%@%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@#==*#%###%%%#+--:+%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@*-=*%@@@@@@@@@@*--:-#@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@**%@@@@@@@@@@@@@#=-::#@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%#+*%@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%#**###**#%@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@###%%#####%%%##**####*%@@@@@@@@@@@@@@@@","@@@@@@@@@@@#++*##%###########*+=+@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#+++*######**##**++-#@%@@@@@@@@@@@@@@@","@@@@@@@@@@@@@#+**####*==+*****==@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%######=--==+*#*+%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%#***##*---=+**+-#@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@+-==+*##*=-=+#+---%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%-=++***======-----#@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%-=++***+==++==----=@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@*=++*+++=====-----:*@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%*+*+========----:*@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%*#*+=========-=#@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@#*##*+=====+++=%@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%+***+=====++=-%@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%@%=+**#***###+-:+@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@*=+*%@@@@@@@#--:#@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#==+#%@@@@@@@@@*++%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@#*#%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@%%%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@%%%%#****#****+#@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@#*###***##%%%###*++##%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@%++##%%%#%%######%#*+++%@@@@@@@@@@@@@@@@@","@@@@@@@@@@@*+*####%%#*#####*+++=%@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@**#####*+=*##**++*=*@%@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@####%*---=+*****+=%@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@*###+-----=*##*+%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%++#%%+--=+**+==-+@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@#+=-=++++======::-:=@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@%=-=-----=+++=-------%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@*++=--:-=+*++=-----:*@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@%%*===++++++-----:+@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%**+=+++++=----:*@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%*#*++++++**+--+%@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@***++++++*++-*@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%=+****##+=-+@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@%=+*#@@@@*=--*@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@#-=*%@@@@@@*--:*@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@#*%@@@@@@@@@#**%@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
    {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@%%##*****#@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%%%#****####**++###%@@@@@@@@@@@@@@@@","@@@@@@@@@@@%******############**+=+%@@@@@@@@@@@@@@","@@@@@@@@@@@%*+*###############*++=+@@@@@@@@@@@@@@@","@@@@@@@@@@@@@%++**####*++*#***+++-#@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%*+####*=--+******=+@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@#*###+---===*##*=#@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@**##+---=+**+=--%@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%++##*--=+*+-:---+@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@%*===+++===---------#@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@*-++*##*++++=------:=@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@*-+++***+++==--------%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@#****+========------%@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@%**+==========--+@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@#*#**=====+****=#@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@*+***+====++++=:*@%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@*=+*##*++*##+--:=@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%+-=*%@@@@@@@@@+---+%@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@%=+*%@@@@@@@@%@@+--:=@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@%%@@@@@@@@@@@@@%#++*%@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@","@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"},
};

//  Fungsi utama animasi: tampilkan selama `durasiDetik` detik
void animasiTerimakasih() {
    int frame     = 0;
    int tick      = 0;
    int lyricIdx  = 0;
    int lyricTick = 0;

    //  Jalankan animasi selama ~5 detik (≈ 115 iterasi × 130ms)
    const int TOTAL_ITER = 115;
    for (int iter = 0; iter < TOTAL_ITER; iter++) {
        clearLayar();

        //  Judul TERIMA KASIH -- warna bergantian hijau/cyan
        cout << "\n";
        for (int i = 0; i < (int)_TITLE.size(); i++) {
            _col((i + tick) % 2 == 0 ? "\033[92m" : "\033[96m");
            cout << _TITLE[i] << "\n";
            _col(RESET);
        }

        //  Separator
        _col("\033[96m");
        cout << "\n  \xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\n\n";
        _col(RESET);

        //  Frame kucing ASCII -- warna berputar
        const char* catColors[] = {"\033[93m","\033[96m","\033[95m","\033[92m","\033[97m","\033[91m","\033[94m"};
        _col(catColors[tick % 7]);
        cout << "\n";
        int fi = frame % _CAT_N;
        for (int i = 0; i < _CAT_L; i++)
            cout << "      " << _CAT[fi][i] << "\n";
        _col(RESET);
        cout << "\n";

        //  Separator
        _col("\033[96m");
        cout << "  \xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7";
        _col(RESET);

        //  Lirik Kicau Mania
        cout << "\n\n";
        _col("\033[96m");
        cout << "  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
        _col(RESET);
        for (int i = 0; i < (int)_LYRICS.size(); i++) {
            if (i == lyricIdx) {
                const char* lyrColors[] = {"\033[93m","\033[95m","\033[96m","\033[92m"};
                _col(lyrColors[tick % 4]);
                cout << "  \xE2\x99\xAA " << _LYRICS[i] << " \xE2\x99\xAA\n";
            } else {
                cout << "\033[90m    " << _LYRICS[i] << "\n";
            }
            _col(RESET);
        }
        _col("\033[96m");
        cout << "  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
        _col(RESET);
        cout << "\n";

        sleepMs(130);
        frame++;
        lyricTick++;
        if (lyricTick >= 18) { lyricTick = 0; lyricIdx = (lyricIdx + 1) % (int)_LYRICS.size(); }
        if (frame % 5 == 0) tick++;
    }
    clearLayar();
}


//  ENTRY POINT
int main() {
    // Aktifkan UTF-8 di Windows (opsional, tidak wajib karena tidak pakai emoji)
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
    init_data();
    menuUtama();
    return 0;
}

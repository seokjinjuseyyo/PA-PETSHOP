//   SISTEM MANAJEMEN TOKO PERLENGKAPAN HEWAN "WHINGKY"
//   File     : toko.h
//   Deskripsi: Header utama -- deklarasi konstanta, enum,
//              struct, variabel global, dan prototipe fungsi

#ifndef TOKO_H
#define TOKO_H

#include <iostream>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <ctime>
#include <limits>
#include <cctype>
#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
#else
  #include <unistd.h>
  #include <termios.h>
#endif
using namespace std;

//  KODE WARNA ANSI (Terminal Color Codes)
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"

#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"

#define BRED        "\033[1;31m"
#define BGREEN      "\033[1;32m"
#define BYELLOW     "\033[1;33m"
#define BBLUE       "\033[1;34m"
#define BMAGENTA    "\033[1;35m"
#define BCYAN       "\033[1;36m"
#define BWHITE      "\033[1;37m"

#define BG_BLUE     "\033[44m"
#define BG_CYAN     "\033[46m"
#define BG_GREEN    "\033[42m"
#define BG_YELLOW   "\033[43m"
#define BG_RED      "\033[41m"

//  KONSTANTA KAPASITAS SISTEM
const int MAX_PRODUK  = 200;
const int MAX_USER    = 100;
const int MAX_RIWAYAT = 500;

//  LEBAR KOLOM TABEL PRODUK
const int W_ID      = 4;
const int W_NAMA    = 25;
const int W_KAT     = 14;
const int W_SUBKAT  = 16;
const int W_HARGA   = 13;
const int W_STOK    = 6;

//  ENUMERASI
enum KategoriHewan { KUCING = 1, ANJING, REPTIL, TERNAK };
enum UrutanHarga   { ASC = 1, DESC };
enum SubKategori   { OBAT = 1, MAKANAN, AKSESORIS, PERLENGKAPAN };

//  STRUCT
struct Produk {
    int            id;
    string         nama;
    KategoriHewan  kategori;
    SubKategori    subKategori;
    double         harga;
    int            stok;
};

struct Pengguna {
    int    id;
    string username;
    string password;
    string role;
    string tanggalDaftar;
};

struct ItemKeranjang {
    int    idProduk;
    string nama;
    int    jumlah;
    double subtotal;
};

struct Riwayat {
    string        user;
    ItemKeranjang items[50];
    int           jumlahItem;
    double        total;
    string        tanggal;
    double        uangBayar;
    double        kembalian;
};

//  VARIABEL GLOBAL
extern Produk   daftarProduk[MAX_PRODUK];
extern int      jumlahProduk;

extern Pengguna daftarPengguna[MAX_USER];
extern int      jumlahPengguna;

extern Riwayat  daftarRiwayat[MAX_RIWAYAT];
extern int      jumlahRiwayat;

//  PROTOTIPE FUNGSI: Utilitas & Validasi
void   clearLayar();
string inputPassword(const string& prompt);
void   lemparError(const string& pesan);
int    inputAngkaAman();
int    inputAngkaRentang(int minVal, int maxVal);
double inputDoubleAman();
string inputTeksAman(const string& prompt);
void   validasiStok(int stok);
void   validasiHarga(double harga);
void   validasiPassword(const string& password);
string getTimestamp();

//  PROTOTIPE FUNGSI: Animasi
void animasiLoading(const string& pesan, int langkah);
void animasiKetik(const string& teks, int delayMs);
void animasiTerimakasih();   // Animasi ASCII kucing + judul "TERIMA KASIH"

//  PROTOTIPE FUNGSI: Tampilan Tabel & UI
void   tampilkanTabelProduk(const Produk arr[], int n);
void   tampilkanKeranjang(const vector<ItemKeranjang>& keranjang);
string pusatTeks(const string& teks, int lebar);
void   cetakHeader(const string& judul, int lebar, const string& warna);
void   cetakGarisH(int lebar, char simbol, const string& warna);
void   cetakPesan(const string& tipe, const string& pesan);

//  PROTOTIPE FUNGSI: Pencarian & Pengurutan
int         cariBinaryByID(int idTarget);
vector<int> cariLinearKeyword(const string& keyword);
void        bubbleSortHarga(Produk* arr, int n, UrutanHarga urutan);
void        selectionSortNama(Produk* arr, int n);
void        insertionSortStok(Produk* arr, int n);
void        filterKategoriDanSort(KategoriHewan kat, UrutanHarga urutan);

//  PROTOTIPE FUNGSI: Menu & Alur Program
void init_data();
void menuUtama();
void menuLogin();
void menuRegister();
void menuAdmin();
void menuUser(const string& username);
void prosesPembelianUser(const string& username);
void tampilkanRiwayat(const string& username);

#endif // TOKO_H

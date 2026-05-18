#include "toko.h"


//  FITUR: Menu Utama
void menuUtama() {
    // Animasi sambutan saat pertama kali buka
    cout << "\n\n";
    animasiKetik(BCYAN "  Memuat sistem Toko Whingky..." RESET "\n", 25);
    animasiLoading("Inisialisasi data", 4);
    cout << "\n";

    int pilihan;
    do {
        cout << "\n";
        cout << BCYAN
             << "  +======================================================================+\n"
             << "  |" << RESET;
        cout << BG_BLUE << BWHITE << pusatTeks("TOKO PERLENGKAPAN HEWAN", 70) << RESET;
        cout << BCYAN << "|\n"
             << "  |" << RESET;
        cout << BG_BLUE << BYELLOW << pusatTeks("~ W H I N G K Y ~", 70) << RESET;
        cout << BCYAN << "|\n"
             << "  |" << RESET << string(70,' ') << BCYAN << "|\n"
             << "  +======================================================================+\n"
             << "  |" << RESET
             << "  " << BGREEN  << "[ 1 ]" << RESET << "  Masuk ke Akun" << string(48,' ') << BCYAN << "|\n"
             << "  |" << RESET
             << "  " << BYELLOW << "[ 2 ]" << RESET << "  Daftar Akun Baru" << string(45,' ') << BCYAN << "|\n"
             << "  |" << RESET
             << "  " << BRED    << "[ 3 ]" << RESET << "  Keluar Program" << string(47,' ') << BCYAN << "|\n"
             << "  +======================================================================+\n" << RESET;

        cout << "\n" << BYELLOW << "  Pilih Menu [1-3]" << RESET;
        pilihan = inputAngkaRentang(1, 3);

        switch(pilihan) {
            case 1:
                animasiLoading("Membuka halaman login", 2);
                clearLayar();
                menuLogin();
                break;
            case 2:
                animasiLoading("Membuka halaman registrasi", 2);
                clearLayar();
                menuRegister();
                break;
            case 3:
                animasiLoading("Menyiapkan animasi perpisahan", 2);
                animasiTerimakasih();
                break;
        }
    } while (pilihan != 3);
}


//  FITUR: Login (maks. 3 percobaan)
void menuLogin() {
    string user, pass;
    int  coba  = 0;
    bool sukses = false;

    clearLayar();
    cout << "\n"
         << BBLUE << "  +========================================+\n"
         << "  |" << RESET;
    cout << BG_BLUE << BWHITE << pusatTeks("MASUK KE AKUN", 40) << RESET;
    cout << BBLUE << "|\n"
         << "  +========================================+\n" << RESET;

    // Tanya preferensi tampil password
    cout << BYELLOW << "  Tampilkan password saat mengetik?\n" << RESET
         << "  [ 1 ] Ya (terlihat)   [ 2 ] Tidak (bintang *)\n";
    cout << BYELLOW << "  Pilih [1-2]" << RESET;
    int tampilPass = inputAngkaRentang(1, 2);

    while (coba < 3 && !sukses) {
        try {
            user = inputTeksAman("Username : ");
            if (tampilPass == 1)
                pass = inputTeksAman("Password : ");
            else
                pass = inputPassword("Password : ");

            animasiLoading("Memverifikasi akun", 2);

            for (int i = 0; i < jumlahPengguna; i++) {
                if (daftarPengguna[i].username == user &&
                    daftarPengguna[i].password == pass) {
                    sukses = true;
                    cout << "\n";
                    cetakPesan("ok", string("Login berhasil. Selamat datang, ")
                               + BWHITE + user + RESET + GREEN + "!");
                    animasiLoading("Memuat panel", 2);
                    clearLayar();
                    if (daftarPengguna[i].role == "admin") {
                        menuAdmin();
                    } else {
                        menuUser(user);
                    }
                    break;
                }
            }
            if (!sukses) {
                coba++;
                cetakPesan("err", string("Username atau password salah. Sisa percobaan: ")
                           + BYELLOW + to_string(3 - coba) + RESET);
            }
        } catch (const exception& e) {
            cetakPesan("err", e.what());
        }
    }

    if (!sukses) {
        cetakPesan("warn", "Terlalu banyak percobaan gagal. Kembali ke menu utama.");
    }
}


//  FITUR: Registrasi Akun Baru
void menuRegister() {
    clearLayar();
    cout << "\n"
         << BGREEN << "  +========================================+\n"
         << "  |" << RESET;
    cout << BG_GREEN << BWHITE << pusatTeks("DAFTAR AKUN BARU", 40) << RESET;
    cout << BGREEN << "|\n"
         << "  +========================================+\n" << RESET;

    // Tanya preferensi tampil password
    cout << BYELLOW << "  Tampilkan password saat mengetik?\n" << RESET
         << "  [ 1 ] Ya (terlihat)   [ 2 ] Tidak (bintang *)\n";
    cout << BYELLOW << "  Pilih [1-2]" << RESET;
    int tampilPass = inputAngkaRentang(1, 2);

    while (true) {
        try {
            if (jumlahPengguna >= MAX_USER)
                lemparError("Kapasitas pengguna penuh");

            string username = inputTeksAman("Username baru                : ");

            // Cek duplikat
            for (int i = 0; i < jumlahPengguna; i++) {
                if (daftarPengguna[i].username == username)
                    lemparError("Username '" + username + "' sudah digunakan");
            }

            string password;
            if (tampilPass == 1)
                password = inputTeksAman("Password (min. 4 karakter)   : ");
            else
                password = inputPassword("Password (min. 4 karakter)   : ");

            try {
                validasiPassword(password);  
            } catch (const exception& e) {
                cetakPesan("err", e.what());
                cout << YELLOW << "  [ 1 ] Coba lagi   [ 2 ] Batalkan\n" << RESET;
                cout << BYELLOW << "  Pilih [1-2]" << RESET;
                int p = inputAngkaRentang(1, 2);
                if (p == 2) { cetakPesan("warn", "Registrasi dibatalkan."); return; }
                continue;
            }

            animasiLoading("Menyimpan akun baru", 2);
            daftarPengguna[jumlahPengguna].id           = jumlahPengguna + 1;
            daftarPengguna[jumlahPengguna].username     = username;
            daftarPengguna[jumlahPengguna].password     = password;
            daftarPengguna[jumlahPengguna].role         = "user";
            daftarPengguna[jumlahPengguna].tanggalDaftar = getTimestamp();
            jumlahPengguna++;

            cetakPesan("ok", "Registrasi berhasil! Silakan masuk dengan akun baru Anda.");
            return;

        } catch (const exception& e) {
            cetakPesan("err", e.what());
            cout << YELLOW << "  [ 1 ] Coba lagi   [ 2 ] Batalkan\n" << RESET;
            cout << BYELLOW << "  Pilih [1-2]" << RESET;
            int p = inputAngkaRentang(1, 2);
            if (p == 2) { cetakPesan("warn", "Registrasi dibatalkan."); return; }
        }
    }
}
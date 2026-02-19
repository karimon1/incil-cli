/*
 * incil-tui — Türkçe Yeni Ahit TUI görüntüleyici
 * Derleme: gcc -o incil-tui incil_tui.c -lncurses
 * Bağımlılık: ncurses
 */

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define MAX_KITAP    16
#define MAX_BOLUM   200
#define MAX_METIN  1024

#define DATA_PATH   "%s/.local/share/incil/turkish.xml"

typedef struct {
    const char *kod;
    const char *ad;
} Kitap;

static const Kitap KITAPLAR[MAX_KITAP] = {
    {"MAT", "Matta"},
    {"MAR", "Markos"},
    {"LUK", "Luka"},
    {"JOH", "Yuhanna"},
    {"ACT", "Elçilerin İşleri"},
    {"ROM", "Romalılar"},
    {"GAL", "Galatlar"},
    {"EPH", "Efesliler"},
    {"PHI", "Filipililer"},
    {"COL", "Koloseliler"},
    {"HEB", "İbraniler"},
    {"JAM", "Yakup"},
    {"JUD", "Yahuda"},
    {"TIT", "Titus"},
    {"PHM", "Filimona"},
    {"REV", "Vahiy"},
};

typedef struct {
    int  bolum;
    int  ayet;
    char metin[MAX_METIN];
} Ayet;

static Ayet  *g_ayetler  = NULL;
static int    g_ayet_say = 0;
static int    g_ayet_cap = 0;
static char   g_data_file[512];

/* ── XML parse ───────────────────────────────────────────── */
static int veri_yukle(const char *kitap_kodu)
{
    FILE *fp = fopen(g_data_file, "r");
    if (!fp) return -1;

    free(g_ayetler);
    g_ayetler  = NULL;
    g_ayet_say = 0;
    g_ayet_cap = 0;

    /* Aranacak prefix: b.MAT. */
    char aranan[32];
    snprintf(aranan, sizeof(aranan), "b.%s.", kitap_kodu);
    size_t aranan_len = strlen(aranan);

    char line[MAX_METIN * 2];
    int  in_verse  = 0;
    int  cur_bolum = 0, cur_ayet = 0;

    while (fgets(line, sizeof(line), fp)) {
        /* Baştaki boşluk/tab'ı atla */
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;

        if (!in_verse) {
            /* <seg id="b.MAT.3.16" type="verse"> satırını ara */
            char *seg = strstr(p, "<seg id=\"");
            if (!seg) continue;

            /* "<seg id=\"" = 9 karakter → hemen arkası "b.XXX..." */
            const char *id_start = seg + 9;

            /* Kitap kodu eşleşiyor mu? */
            if (strncmp(id_start, aranan, aranan_len) != 0) continue;

            /* b.MAT.BOLUM.AYET → noktaları say */
            /* id_start = "b.MAT.3.16\"..." */
            const char *s = id_start;
            s = strchr(s, '.');       /* b. → geç */
            if (!s) continue;
            s = strchr(s + 1, '.');   /* MAT. → geç */
            if (!s) continue;
            cur_bolum = atoi(s + 1);
            s = strchr(s + 1, '.');   /* BOLUM. → geç */
            if (!s) continue;
            cur_ayet = atoi(s + 1);

            in_verse = 1;

        } else {
            /* Bir sonraki satır = ayet metni (</seg> değilse) */
            if (strstr(p, "</seg>") || strstr(p, "<seg")) {
                in_verse = 0;
                continue;
            }

            /* Sondaki \n \r ve boşlukları temizle */
            int len = (int)strlen(p);
            while (len > 0 && (p[len-1] == '\n' || p[len-1] == '\r' ||
                                p[len-1] == ' '  || p[len-1] == '\t'))
                p[--len] = '\0';

            if (len == 0) { in_verse = 0; continue; }

            /* Kapasiteyi genişlet */
            if (g_ayet_say >= g_ayet_cap) {
                g_ayet_cap = g_ayet_cap ? g_ayet_cap * 2 : 256;
                g_ayetler  = realloc(g_ayetler, (size_t)g_ayet_cap * sizeof(Ayet));
                if (!g_ayetler) { fclose(fp); return -1; }
            }

            g_ayetler[g_ayet_say].bolum = cur_bolum;
            g_ayetler[g_ayet_say].ayet  = cur_ayet;
            strncpy(g_ayetler[g_ayet_say].metin, p, MAX_METIN - 1);
            g_ayetler[g_ayet_say].metin[MAX_METIN - 1] = '\0';
            g_ayet_say++;
            in_verse = 0;
        }
    }

    fclose(fp);
    return g_ayet_say;
}

/* ── Bölüm listesi ───────────────────────────────────────── */
static int bolum_listesi(int *bolumler, int max)
{
    int say = 0, prev = -1;
    for (int i = 0; i < g_ayet_say && say < max; i++) {
        if (g_ayetler[i].bolum != prev) {
            bolumler[say++] = g_ayetler[i].bolum;
            prev = g_ayetler[i].bolum;
        }
    }
    return say;
}

/* ── Metni word-wrap ile yaz ─────────────────────────────── */
static int metin_yaz(WINDOW *win, int y, int x, int genislik, const char *metin)
{
    char buf[MAX_METIN];
    strncpy(buf, metin, MAX_METIN - 1);
    buf[MAX_METIN - 1] = '\0';

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    (void)max_x;

    int cx = x, cy = y, satirlar = 1;
    char *tok = strtok(buf, " ");
    while (tok) {
        int tlen = (int)strlen(tok);
        if (cx + tlen >= x + genislik) {
            cy++; cx = x; satirlar++;
            if (cy >= max_y - 1) break;
        }
        mvwprintw(win, cy, cx, "%s ", tok);
        cx += tlen + 1;
        tok = strtok(NULL, " ");
    }
    return satirlar;
}

/* ── Ana TUI ─────────────────────────────────────────────── */
int main(void)
{
    setlocale(LC_ALL, "");

    const char *home = getenv("HOME");
    if (!home) { fprintf(stderr, "HOME bulunamadı\n"); return 1; }
    snprintf(g_data_file, sizeof(g_data_file), DATA_PATH, home);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_CYAN,   -1);
        init_pair(2, COLOR_YELLOW, -1);
        init_pair(3, COLOR_WHITE,  -1);
        init_pair(4, COLOR_BLACK,  COLOR_CYAN);
        init_pair(5, COLOR_GREEN,  -1);
        init_pair(6, COLOR_BLACK,  COLOR_YELLOW);
    }

    int SATIR, SUTUN;
    getmaxyx(stdscr, SATIR, SUTUN);

    int menu_w   = 26;
    int icerik_x = menu_w + 1;

    WINDOW *header_win  = newwin(2, SUTUN, 0, 0);
    WINDOW *menu_win    = newwin(SATIR - 3, menu_w, 2, 0);
    WINDOW *icerik_win  = newwin(SATIR - 3, SUTUN - icerik_x, 2, icerik_x);
    WINDOW *status_win  = newwin(1, SUTUN, SATIR - 1, 0);

    int mod         = 0;   /* 0=kitap 1=bölüm */
    int kitap_sec   = 0;
    int bolum_sec   = 0;
    int ayet_scroll = 0;
    int menu_scroll = 0;

    int bolumler[MAX_BOLUM];
    int bolum_say = 0;

    int ch, yenile = 1;

    while (1) {
        getmaxyx(stdscr, SATIR, SUTUN);
        int icerik_w = SUTUN - icerik_x;
        int gorsel_h = SATIR - 5;

        if (yenile) {
            /* ── Header ── */
            werase(header_win);
            wbkgd(header_win, COLOR_PAIR(4));
            mvwprintw(header_win, 0, 2,
                      "incil-tui | Türkçe Yeni Ahit  "
                      "[↑↓] Gezin  [Enter] Seç  [Bksp] Geri  [q] Çık");
            wrefresh(header_win);

            /* ── Menü ── */
            werase(menu_win);
            box(menu_win, 0, 0);

            if (mod == 0) {
                mvwprintw(menu_win, 0, 2, " Kitaplar ");
                for (int i = 0; i < MAX_KITAP; i++) {
                    int y = i - menu_scroll + 1;
                    if (y < 1 || y > gorsel_h) continue;
                    if (i == kitap_sec) {
                        wattron(menu_win, COLOR_PAIR(6) | A_BOLD);
                        mvwprintw(menu_win, y, 1, " %-*s", menu_w - 3,
                                  KITAPLAR[i].ad);
                        wattroff(menu_win, COLOR_PAIR(6) | A_BOLD);
                    } else {
                        mvwprintw(menu_win, y, 2, "%-*s", menu_w - 4,
                                  KITAPLAR[i].ad);
                    }
                }
            } else {
                char baslik[48];
                snprintf(baslik, sizeof(baslik), " %s ", KITAPLAR[kitap_sec].ad);
                mvwprintw(menu_win, 0, 2, "%s", baslik);
                for (int i = 0; i < bolum_say; i++) {
                    int y = i - menu_scroll + 1;
                    if (y < 1 || y > gorsel_h) continue;
                    char label[24];
                    snprintf(label, sizeof(label), "%d. Bölüm", bolumler[i]);
                    if (i == bolum_sec) {
                        wattron(menu_win, COLOR_PAIR(6) | A_BOLD);
                        mvwprintw(menu_win, y, 1, " %-*s", menu_w - 3, label);
                        wattroff(menu_win, COLOR_PAIR(6) | A_BOLD);
                    } else {
                        mvwprintw(menu_win, y, 2, "%-*s", menu_w - 4, label);
                    }
                }
            }
            wrefresh(menu_win);

            /* ── İçerik ── */
            werase(icerik_win);
            box(icerik_win, 0, 0);

            if (mod == 0) {
                wattron(icerik_win, COLOR_PAIR(1) | A_BOLD);
                mvwprintw(icerik_win, 2, 3, "%s", KITAPLAR[kitap_sec].ad);
                wattroff(icerik_win, COLOR_PAIR(1) | A_BOLD);
                mvwprintw(icerik_win, 4, 3,
                          "Bir kitap seçmek için Enter'a basın.");
            } else {
                int hedef_bolum = bolumler[bolum_sec];
                wattron(icerik_win, COLOR_PAIR(1) | A_BOLD);
                mvwprintw(icerik_win, 0, 2, " %s — %d. Bölüm ",
                          KITAPLAR[kitap_sec].ad, hedef_bolum);
                wattroff(icerik_win, COLOR_PAIR(1) | A_BOLD);

                int cy = 1 - ayet_scroll;
                for (int i = 0; i < g_ayet_say; i++) {
                    if (g_ayetler[i].bolum != hedef_bolum) continue;
                    if (cy >= 1 && cy < SATIR - 4) {
                        wattron(icerik_win, COLOR_PAIR(5) | A_BOLD);
                        mvwprintw(icerik_win, cy, 2, "%d:%d  ",
                                  g_ayetler[i].bolum, g_ayetler[i].ayet);
                        wattroff(icerik_win, COLOR_PAIR(5) | A_BOLD);
                        int satirlar = metin_yaz(icerik_win, cy, 8,
                                                 icerik_w - 10,
                                                 g_ayetler[i].metin);
                        cy += satirlar + 1;
                    } else {
                        /* Yaklaşık yükseklik hesapla */
                        int tahmini = (int)strlen(g_ayetler[i].metin)
                                      / (icerik_w - 11) + 2;
                        cy += tahmini;
                    }
                }
            }
            wrefresh(icerik_win);

            /* ── Status ── */
            werase(status_win);
            wbkgd(status_win, COLOR_PAIR(4));
            if (mod == 1)
                mvwprintw(status_win, 0, 2, " %s %d. Bölüm  |  "
                          "↑↓ kaydır  PgUp/PgDn hızlı kaydır",
                          KITAPLAR[kitap_sec].ad, bolumler[bolum_sec]);
            else
                mvwprintw(status_win, 0, 2,
                          " %d kitap  |  Enter ile seçin",
                          MAX_KITAP);
            wrefresh(status_win);

            yenile = 0;
        }

        ch = getch();
        yenile = 1;

        if (ch == 'q' || ch == 'Q') break;

        if (mod == 0) {
            if (ch == KEY_UP && kitap_sec > 0) {
                kitap_sec--;
                if (kitap_sec < menu_scroll) menu_scroll = kitap_sec;
            } else if (ch == KEY_DOWN && kitap_sec < MAX_KITAP - 1) {
                kitap_sec++;
                if (kitap_sec >= menu_scroll + gorsel_h)
                    menu_scroll = kitap_sec - gorsel_h + 1;
            } else if (ch == '\n' || ch == KEY_ENTER) {
                veri_yukle(KITAPLAR[kitap_sec].kod);
                bolum_say   = bolum_listesi(bolumler, MAX_BOLUM);
                bolum_sec   = 0;
                ayet_scroll = 0;
                menu_scroll = 0;
                mod = 1;
            }
        } else {
            if (ch == KEY_BACKSPACE || ch == 127 || ch == 'b') {
                mod = 0;
                menu_scroll = kitap_sec > 3 ? kitap_sec - 3 : 0;
            } else if (ch == KEY_UP) {
                if (ayet_scroll > 0) { ayet_scroll -= 2; }
                else if (bolum_sec > 0) {
                    bolum_sec--;
                    ayet_scroll = 0;
                    if (bolum_sec < menu_scroll) menu_scroll = bolum_sec;
                }
            } else if (ch == KEY_DOWN) {
                if (bolum_sec < bolum_say - 1) {
                    bolum_sec++;
                    ayet_scroll = 0;
                    if (bolum_sec >= menu_scroll + gorsel_h)
                        menu_scroll = bolum_sec - gorsel_h + 1;
                } else {
                    ayet_scroll += 2;
                }
            } else if (ch == KEY_PPAGE) {
                ayet_scroll -= gorsel_h;
                if (ayet_scroll < 0) ayet_scroll = 0;
            } else if (ch == KEY_NPAGE) {
                ayet_scroll += gorsel_h;
            }
        }
    }

    delwin(header_win);
    delwin(menu_win);
    delwin(icerik_win);
    delwin(status_win);
    endwin();
    free(g_ayetler);
    return 0;
}

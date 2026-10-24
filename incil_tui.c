/*
 * incil-tui — Türkçe Kutsal Kitap (Yorumsuz Türkçe Çeviri) TUI görüntüleyici
 * Derleme: make   (ya da: gcc -o incil-tui incil_tui.c -lncursesw)
 * Bağımlılık: ncurses (geniş karakter desteğiyle)
 *
 * Veri: incil ile aynı ytc.tsv dosyası
 *   Kitap ⇥ kısaltma ⇥ kitap no ⇥ bölüm ⇥ ayet ⇥ metin
 */

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>

#define MAX_KITAP   66
#define MAX_SATIR 8192
#define VERI_ADI  "ytc.tsv"

typedef struct {
    int   kitap;          /* 0..65 */
    int   bolum;
    int   ayet;
    char *metin;
} Ayet;

typedef struct {
    char ad[64];
    char kisa[16];
    int  ilk;             /* ilk ayetin g_ayetler içindeki sırası */
    int  bolum_say;
} Kitap;

static Ayet  *g_ayetler  = NULL;
static int    g_ayet_say = 0;
static Kitap  g_kitaplar[MAX_KITAP];
static int    g_kitap_say = 0;

/* Ekrana basılacak sarılmış satırlar (seçili bölüm için) */
typedef struct {
    int         ayet;     /* satır bir ayetin ilk satırıysa ayet no, değilse 0 */
    const char *bas;
    int         uzunluk;  /* bayt */
} Satir;

static Satir *g_satirlar = NULL;
static int    g_satir_say = 0, g_satir_cap = 0;

/* ── Veri dosyasını bul ──────────────────────────────────── */
static const char *veri_yolu(void)
{
    static char aday[5][1024];
    const char *env  = getenv("INCIL_VERI");
    const char *xdg  = getenv("XDG_DATA_HOME");
    const char *home = getenv("HOME");

    if (env)  snprintf(aday[0], sizeof(aday[0]), "%s", env);
    if (xdg)  snprintf(aday[1], sizeof(aday[1]), "%s/incil/%s", xdg, VERI_ADI);
    if (home) snprintf(aday[2], sizeof(aday[2]), "%s/.local/share/incil/%s", home, VERI_ADI);
    snprintf(aday[3], sizeof(aday[3]), "/usr/share/incil/%s", VERI_ADI);
    snprintf(aday[4], sizeof(aday[4]), "data/%s", VERI_ADI);

    for (int i = 0; i < 5; i++) {
        FILE *fp;
        if (!aday[i][0] || !(fp = fopen(aday[i], "r"))) continue;
        fclose(fp);
        return aday[i];
    }
    return NULL;
}

/* ── TSV yükle ───────────────────────────────────────────── */
static int veri_yukle(const char *yol)
{
    FILE *fp = fopen(yol, "r");
    if (!fp) return -1;

    char line[MAX_SATIR];
    int cap = 0;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';

        char *alan[6];
        char *p = line;
        int i;
        for (i = 0; i < 6 && p; i++) {
            alan[i] = p;
            p = (i < 5) ? strchr(p, '\t') : NULL;
            if (p) *p++ = '\0';
        }
        if (i < 6) continue;

        int k = atoi(alan[2]) - 1;
        if (k < 0 || k >= MAX_KITAP) continue;

        if (g_ayet_say >= cap) {
            cap = cap ? cap * 2 : 32768;
            Ayet *yeni = realloc(g_ayetler, (size_t)cap * sizeof(Ayet));
            if (!yeni) { fclose(fp); return -1; }
            g_ayetler = yeni;
        }
        Ayet *a = &g_ayetler[g_ayet_say];
        a->kitap = k;
        a->bolum = atoi(alan[3]);
        a->ayet  = atoi(alan[4]);
        a->metin = strdup(alan[5]);

        Kitap *kt = &g_kitaplar[k];
        if (!kt->ad[0]) {
            snprintf(kt->ad,   sizeof(kt->ad),   "%s", alan[0]);
            snprintf(kt->kisa, sizeof(kt->kisa), "%s", alan[1]);
            kt->ilk = g_ayet_say;
            if (k + 1 > g_kitap_say) g_kitap_say = k + 1;
        }
        if (a->bolum > kt->bolum_say) kt->bolum_say = a->bolum;
        g_ayet_say++;
    }
    fclose(fp);
    return g_ayet_say;
}

static void satir_ekle(int ayet, const char *bas, int uzunluk)
{
    if (g_satir_say >= g_satir_cap) {
        g_satir_cap = g_satir_cap ? g_satir_cap * 2 : 512;
        g_satirlar  = realloc(g_satirlar, (size_t)g_satir_cap * sizeof(Satir));
        if (!g_satirlar) { endwin(); exit(1); }
    }
    g_satirlar[g_satir_say++] = (Satir){ ayet, bas, uzunluk };
}

/* ── Seçili bölümü verilen genişliğe göre satırlara böl ──── */
static void bolum_sar(int kitap, int bolum, int genislik)
{
    g_satir_say = 0;
    if (genislik < 10) genislik = 10;

    for (int i = g_kitaplar[kitap].ilk;
         i < g_ayet_say && g_ayetler[i].kitap == kitap; i++) {
        if (g_ayetler[i].bolum != bolum) continue;

        const char *p = g_ayetler[i].metin;
        int ilk = 1;
        while (*p) {
            /* genişliğe sığan en uzun kelime dizisini bul */
            const char *son_bosluk = NULL, *q = p;
            int kr = 0;
            while (*q && kr < genislik) {
                if (*q == ' ') son_bosluk = q;
                if (((unsigned char)*q & 0xC0) != 0x80) kr++;
                q++;
            }
            /* yarım kalan çok baytlı karakteri tamamla */
            while (*q && ((unsigned char)*q & 0xC0) == 0x80) q++;

            const char *bitis = (*q && *q != ' ' && son_bosluk) ? son_bosluk : q;
            satir_ekle(ilk ? g_ayetler[i].ayet : 0, p, (int)(bitis - p));
            ilk = 0;
            p = bitis;
            while (*p == ' ') p++;
        }
        satir_ekle(-1, "", 0);    /* ayetler arası boşluk */
    }
}

/* ── Yardımcılar ─────────────────────────────────────────── */
static void kisalt_yaz(WINDOW *w, int y, int x, int gen, const char *s)
{
    /* s'yi gen karakterle sınırlayıp yaz */
    int n = (int)strlen(s), b = 0, kr = 0;
    while (b < n && kr < gen) {
        b++;
        while (b < n && ((unsigned char)s[b] & 0xC0) == 0x80) b++;
        kr++;
    }
    mvwaddnstr(w, y, x, s, b);
    for (; kr < gen; kr++) waddch(w, ' ');
}

enum { RENK_BASLIK = 1, RENK_SECILI, RENK_NO, RENK_GRUP, RENK_SOLUK };

/* ── Ana TUI ─────────────────────────────────────────────── */
int main(void)
{
    setlocale(LC_ALL, "");

    const char *yol = veri_yolu();
    if (!yol) {
        fprintf(stderr, "Veri dosyası (%s) bulunamadı.\n"
                        "Önce bir kez `incil` çalıştırın; veri otomatik indirilir.\n",
                VERI_ADI);
        return 1;
    }
    if (veri_yukle(yol) <= 0) {
        fprintf(stderr, "Veri okunamadı: %s\n", yol);
        return 1;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    set_escdelay(25);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(RENK_BASLIK, COLOR_BLACK,  COLOR_CYAN);
        init_pair(RENK_SECILI, COLOR_BLACK,  COLOR_YELLOW);
        init_pair(RENK_NO,     COLOR_GREEN,  -1);
        init_pair(RENK_GRUP,   COLOR_YELLOW, -1);
        init_pair(RENK_SOLUK,  COLOR_CYAN,   -1);
    }

    const int menu_w = 24;
    int mod       = 0;     /* 0 = kitap listesi, 1 = bölüm listesi */
    int kitap_sec = 39;    /* Matta */
    int bolum_sec = 0;     /* 0 tabanlı */
    int menu_kay  = 0;
    int metin_kay = 0;
    int sar_gen   = -1, sar_kitap = -1, sar_bolum = -1;

    for (;;) {
        int H, W;
        getmaxyx(stdscr, H, W);
        int govde_h  = H - 2;            /* üst ve alt çubuk hariç */
        int liste_h  = govde_h - 2;      /* çerçeve hariç */
        int metin_w  = W - menu_w;
        int metin_ic = metin_w - 4 - 5;  /* çerçeve + ayet no sütunu */
        if (liste_h < 1 || metin_ic < 10) {
            erase();
            mvaddstr(0, 0, "Pencere çok küçük");
            refresh();
            if (getch() == 'q') break;
            continue;
        }

        Kitap *kt = &g_kitaplar[kitap_sec];
        int bolum = bolum_sec + 1;
        if (sar_gen != metin_ic || sar_kitap != kitap_sec || sar_bolum != bolum) {
            bolum_sar(kitap_sec, bolum, metin_ic);
            sar_gen = metin_ic; sar_kitap = kitap_sec; sar_bolum = bolum;
        }
        int metin_h = govde_h - 2;
        int max_kay = g_satir_say - metin_h;
        if (max_kay < 0) max_kay = 0;
        if (metin_kay > max_kay) metin_kay = max_kay;
        if (metin_kay < 0) metin_kay = 0;

        int liste_say = mod == 0 ? g_kitap_say : kt->bolum_say;
        int secili    = mod == 0 ? kitap_sec   : bolum_sec;
        if (secili < menu_kay) menu_kay = secili;
        if (secili >= menu_kay + liste_h) menu_kay = secili - liste_h + 1;

        erase();

        /* ── Üst çubuk ── */
        attron(COLOR_PAIR(RENK_BASLIK));
        mvhline(0, 0, ' ', W);
        kisalt_yaz(stdscr, 0, 1, W - 2,
                   "incil-tui · Türkçe Kutsal Kitap (YTC)   "
                   "↑↓/jk gezin  Enter seç  ⌫ geri  n/p bölüm  "
                   "PgUp/PgDn kaydır  r rastgele  q çık");
        attroff(COLOR_PAIR(RENK_BASLIK));

        /* ── Sol menü ── */
        WINDOW *menu = derwin(stdscr, govde_h, menu_w, 1, 0);
        box(menu, 0, 0);
        if (mod == 0) {
            mvwaddstr(menu, 0, 2, " Kitaplar ");
        } else {
            char baslik[96];
            snprintf(baslik, sizeof(baslik), " %s ", kt->ad);
            mvwaddstr(menu, 0, 2, baslik);
        }
        for (int i = 0; i < liste_h && menu_kay + i < liste_say; i++) {
            int idx = menu_kay + i;
            char etiket[96];
            if (mod == 0)
                snprintf(etiket, sizeof(etiket), "%s", g_kitaplar[idx].ad);
            else
                snprintf(etiket, sizeof(etiket), "%d. Bölüm", idx + 1);
            if (idx == secili) {
                wattron(menu, COLOR_PAIR(RENK_SECILI) | A_BOLD);
                mvwaddch(menu, i + 1, 1, ' ');
                kisalt_yaz(menu, i + 1, 2, menu_w - 3, etiket);
                wattroff(menu, COLOR_PAIR(RENK_SECILI) | A_BOLD);
            } else {
                if (mod == 0 && idx >= 39) wattron(menu, COLOR_PAIR(RENK_SOLUK));
                kisalt_yaz(menu, i + 1, 2, menu_w - 3, etiket);
                if (mod == 0 && idx >= 39) wattroff(menu, COLOR_PAIR(RENK_SOLUK));
            }
        }
        delwin(menu);

        /* ── Metin ── */
        WINDOW *metin = derwin(stdscr, govde_h, metin_w, 1, menu_w);
        box(metin, 0, 0);
        {
            char baslik[128];
            snprintf(baslik, sizeof(baslik), " %s %d ", kt->ad, bolum);
            wattron(metin, A_BOLD);
            mvwaddstr(metin, 0, 2, baslik);
            wattroff(metin, A_BOLD);
        }
        for (int i = 0; i < metin_h && metin_kay + i < g_satir_say; i++) {
            Satir *s = &g_satirlar[metin_kay + i];
            if (s->ayet > 0) {
                wattron(metin, COLOR_PAIR(RENK_NO) | A_BOLD);
                mvwprintw(metin, i + 1, 2, "%3d", s->ayet);
                wattroff(metin, COLOR_PAIR(RENK_NO) | A_BOLD);
            }
            if (s->uzunluk > 0)
                mvwaddnstr(metin, i + 1, 7, s->bas, s->uzunluk);
        }
        if (g_satir_say > metin_h) {
            char yuzde[16];
            snprintf(yuzde, sizeof(yuzde), " %d%% ", max_kay ? metin_kay * 100 / max_kay : 100);
            mvwaddstr(metin, govde_h - 1, metin_w - (int)strlen(yuzde) - 2, yuzde);
        }
        delwin(metin);

        /* ── Alt çubuk ── */
        attron(COLOR_PAIR(RENK_BASLIK));
        mvhline(H - 1, 0, ' ', W);
        {
            char durum[256];
            if (mod == 0)
                snprintf(durum, sizeof(durum), " %s · %s  |  %d kitap, %d ayet",
                         kitap_sec < 39 ? "Eski Antlaşma" : "Yeni Antlaşma",
                         kt->ad, g_kitap_say, g_ayet_say);
            else
                snprintf(durum, sizeof(durum), " %s %d / %d  |  Yorumsuz Türkçe Çeviri · CC BY-ND 4.0",
                         kt->ad, bolum, kt->bolum_say);
            kisalt_yaz(stdscr, H - 1, 0, W, durum);
        }
        attroff(COLOR_PAIR(RENK_BASLIK));
        refresh();

        /* ── Tuşlar ── */
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;

        switch (ch) {
        case KEY_UP: case 'k':
            if (mod == 0) { if (kitap_sec > 0) { kitap_sec--; bolum_sec = 0; metin_kay = 0; } }
            else if (bolum_sec > 0) { bolum_sec--; metin_kay = 0; }
            break;
        case KEY_DOWN: case 'j':
            if (mod == 0) { if (kitap_sec < g_kitap_say - 1) { kitap_sec++; bolum_sec = 0; metin_kay = 0; } }
            else if (bolum_sec < kt->bolum_say - 1) { bolum_sec++; metin_kay = 0; }
            break;
        case KEY_HOME: case 'g':
            if (mod == 0) kitap_sec = 0; else bolum_sec = 0;
            metin_kay = 0;
            break;
        case KEY_END: case 'G':
            if (mod == 0) kitap_sec = g_kitap_say - 1; else bolum_sec = kt->bolum_say - 1;
            metin_kay = 0;
            break;
        case '\n': case KEY_ENTER: case KEY_RIGHT: case 'l':
            if (mod == 0) { mod = 1; menu_kay = 0; }
            break;
        case KEY_BACKSPACE: case 127: case 8: case KEY_LEFT: case 'h': case 27:
            if (mod == 1) { mod = 0; menu_kay = 0; }
            break;
        case 'n':   /* sonraki bölüm (kitap sonunda bir sonraki kitaba geç) */
            if (bolum_sec < kt->bolum_say - 1) bolum_sec++;
            else if (kitap_sec < g_kitap_say - 1) { kitap_sec++; bolum_sec = 0; }
            metin_kay = 0;
            break;
        case 'p':
            if (bolum_sec > 0) bolum_sec--;
            else if (kitap_sec > 0) { kitap_sec--; bolum_sec = g_kitaplar[kitap_sec].bolum_say - 1; }
            metin_kay = 0;
            break;
        case KEY_NPAGE: case ' ': case 'J':
            metin_kay += ch == 'J' ? 1 : metin_h - 1;
            break;
        case KEY_PPAGE: case 'b': case 'K':
            metin_kay -= ch == 'K' ? 1 : metin_h - 1;
            break;
        case 'r': {
            static int tohum = 0;
            if (!tohum) { srand((unsigned)time(NULL)); tohum = 1; }
            Ayet *a = &g_ayetler[rand() % g_ayet_say];
            kitap_sec = a->kitap; bolum_sec = a->bolum - 1; mod = 1;
            /* ayeti ekranın üstüne getir */
            bolum_sar(kitap_sec, a->bolum, metin_ic);
            sar_gen = metin_ic; sar_kitap = kitap_sec; sar_bolum = a->bolum;
            metin_kay = 0;
            for (int i = 0; i < g_satir_say; i++)
                if (g_satirlar[i].ayet == a->ayet) { metin_kay = i; break; }
            break;
        }
        case KEY_RESIZE:
            sar_gen = -1;
            break;
        }
    }

    endwin();
    for (int i = 0; i < g_ayet_say; i++) free(g_ayetler[i].metin);
    free(g_ayetler);
    free(g_satirlar);
    return 0;
}

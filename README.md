# incil-cli

Türkçe Kutsal Kitap'ı terminalden okuyun. Eski ve Yeni Antlaşma'nın 66 kitabı, internet bağlantısı gerekmeden.
[Luke Smith'in komut satırı İncilleri](https://lukesmith.xyz/articles/command-line-bibles/)nden ve [bible-terminal](https://github.com/Neclonesoul/bible-terminal)'den esinlenildi.

![bash](https://img.shields.io/badge/bash-4.0%2B-green) ![platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-blue) ![license](https://img.shields.io/badge/license-MIT-yellow)

Metin olarak eBible.org'un **[Yorumsuz Türkçe Çeviri (YTC)](https://ebible.org/details.php?id=turytc)**'sini kullanır. Çeviri modern Türkçedir, tüm Kutsal Kitap'ı kapsar ve düzenli olarak güncellenir.

## Hızlı Kurulum

```bash
git clone https://github.com/karimon1/incil-cli.git
cd incil-cli
./install.sh
```

`install.sh` şunları yapar:

- `incil` scriptini `~/.local/bin` dizinine kopyalar.
- Veriyi `~/.local/share/incil/ytc.tsv` dosyasına kopyalar.
- `cc` ve ncurses varsa `incil-tui`'yi derler.
- Gerekirse `PATH`'e ekleme yapar (bash, zsh, fish).

Bunun yerine `make install` de kullanılabilir. Kurulum yeri `PREFIX=/usr/local` ile değiştirilebilir.

> Yalnızca `incil` scriptini kopyalarsanız, veri ilk çalıştırmada (~4,5 MB) otomatik indirilir.

## Kullanım

```bash
incil                      # rastgele ayet
incil yuh 3:16             # tek ayet
incil yuh 3:16-18          # ayet aralığı
incil yuh 3:16,18,20       # seçili ayetler
incil yuh 3:16-4:3         # bölümler arası aralık
incil mez 23               # bölümün tamamı
incil mez 1-3              # birden fazla bölüm
incil rut                  # kitabın tamamı
incil 1 kor 13             # numaralı kitaplar: 1kor, 1ko, "1 Korintliler"
incil yuh 3:16; rom 8:28   # birden fazla referans
incil /sevgi               # tüm Kutsal Kitap'ta arama
incil yuh/ışık             # yalnızca Yuhanna'da arama
incil -i                   # etkileşimli mod (incil> istemi)
incil -l                   # kitaplar ve kısaltmalar
incil -h                   # yardım
```

> Bash'te `;` komut ayırıcıdır. Birden fazla referans verirken tırnak kullanın: `incil "yuh 3:16; rom 8:28"`

- **Kitap adları:** Büyük/küçük harf ve Türkçe karakter ayrımı yapılmaz. `çık`, `cik`, `Çıkış` ve `ÇIKIŞ` aynı kitabı gösterir. Kitap adının baş harfleri yeter (`yarat`, `mezm`). Eski kısaltmalar da çalışır (`isi`, `phi`, `flm`, `yak` …).
- **Arama:** Türkçe karakter ve büyük/küçük harf ayrımı yapmaz. Düzenli ifade (regex) kabul eder: `incil '/ışık|karanlık'`.
- **Çıktı:** Terminalde renkli ve sarılmış olarak gösterilir. Uzun çıktılar `less` ile sayfalanır. Bir boruya (pipe) yönlendirildiğinde `Kitap B:A<TAB>metin` biçiminde düz satırlar verir, bu yüzden `grep`, `wc` ve `fzf` ile birlikte kullanılabilir:

```bash
incil /sevgi | wc -l
incil mez | fzf
```

### Ortam değişkenleri

| Değişken      | Açıklama                                        |
|---------------|-------------------------------------------------|
| `INCIL_PAGER` | Sayfalayıcı (varsayılan `less -RF`). `-n` ile kapatılır. |
| `NO_COLOR`    | Tanımlıysa renk kullanılmaz                     |
| `INCIL_VERI`  | Başka bir TSV veri dosyası yolu                 |

## TUI

```bash
incil-tui
```

| Tuş             | İşlev                                |
|-----------------|--------------------------------------|
| `↑↓` / `j k`    | kitap / bölüm seç                    |
| `Enter` / `→`   | kitabı aç                            |
| `⌫` / `←` / `Esc` | kitap listesine dön                |
| `n` / `p`       | sonraki / önceki bölüm (kitaplar arası geçer) |
| `PgDn` `Space` / `PgUp` `b` | metni sayfa sayfa kaydır  |
| `J` / `K`       | metni satır satır kaydır             |
| `g` / `G`       | ilk / son kitap ya da bölüm          |
| `r`             | rastgele ayet                        |
| `q`             | çıkış                                |

## Kitap Kısaltmaları

`incil -l` komutu kitapların tam listesini gösterir.

| Eski Antlaşma |  | Yeni Antlaşma |  |
|---|---|---|---|
| `yar` Yaratılış | `ezg` Solomon'un Ezgisi | `mat` Matta | `1ti` 1 Timoteos |
| `çık` Çıkış | `yşa` Yeşaya | `mar` Markos | `2ti` 2 Timoteos |
| `lev` Levililer | `yer` Yeremya | `luk` Luka | `tit` Titus |
| `say` Sayılar | `ağı` Ağıtlar | `yuh` Yuhanna | `flm` Filimon |
| `yas` Yasa'nın Tekrarı | `hez` Hezekiel | `elç` Elçilerin İşleri | `ibr` İbraniler |
| `yşu` Yeşu | `dan` Daniel | `rom` Romalılar | `yak` Yakov |
| `hak` Hâkimler | `hoş` Hoşea | `1ko` 1 Korintliler | `1pe` 1 Petrus |
| `rut` Rut | `yoe` Yoel | `2ko` 2 Korintliler | `2pe` 2 Petrus |
| `1sa` `2sa` Samuel | `amo` Amos | `gal` Galatyalılar | `1yu` 1 Yuhanna |
| `1kr` `2kr` Krallar | `ova` Ovadya | `efe` Efesliler | `2yu` 2 Yuhanna |
| `1ta` `2ta` Tarihler | `yon` Yona | `flp` Filipililer | `3yu` 3 Yuhanna |
| `ezr` Ezra | `mik` Mika | `kol` Koloseliler | `yah` Yahuda |
| `neh` Nehemya | `nah` Nahum | `1se` 1 Selanikliler | `vah` Vahiy |
| `est` Ester | `hab` Habakkuk | `2se` 2 Selanikliler | |
| `iyo` İyov | `sef` Sefanya | | |
| `mez` Mezmurlar | `hag` Hagay | | |
| `özd` Özdeyişler | `zek` Zekariya | | |
| `vai` Vaiz | `mal` Malaki | | |

## Gereksinimler

- `bash` 4.0 veya üstü, `awk` (gawk, mawk ya da BSD awk)
- `less` (isteğe bağlı, sayfalama için)
- TUI için: C derleyicisi ve `ncurses` (geniş karakter desteğiyle, ör. `ncursesw`)

## Veri

`data/ytc.tsv` dosyasında her satır bir ayettir:

```
Kitap ⇥ kısaltma ⇥ kitap no ⇥ bölüm ⇥ ayet ⇥ metin
```

Veri, eBible.org'daki son sürümden yeniden üretilebilir:

```bash
scripts/ytc-tsv.sh
```

## Lisans

- **Kod:** MIT. Ayrıntılar için [LICENSE](LICENSE) dosyasına bakın.
- **Kutsal Kitap metni:** *Yorumsuz Türkçe Çeviri (YTC)*, Copyright © 2023-2025 İsmail Serinken and eBible.org. [Creative Commons Attribution-No Derivatives 4.0](https://creativecommons.org/licenses/by-nd/4.0/) lisanslıdır. Metin hiçbir değişiklik yapılmadan, yalnızca biçimi dönüştürülerek dağıtılır. Kaynak: <https://ebible.org/details.php?id=turytc>. Çeviri hatalarını turytc@eBible.org adresine bildirebilirsiniz.

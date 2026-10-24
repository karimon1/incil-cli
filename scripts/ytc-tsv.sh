#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
#  ytc-tsv.sh — eBible.org'dan Yorumsuz Türkçe Çeviri'yi (YTC) indirip
#  incil'in kullandığı TSV biçimine dönüştürür.
#
#  KULLANIM:
#    scripts/ytc-tsv.sh                 → data/ytc.tsv dosyasını yeniden üret
#    scripts/ytc-tsv.sh turytc_vpl.txt  → indirmeden, yerel VPL dosyasından üret
#
#  TSV sütunları:  Kitap ⇥ kısaltma ⇥ kitap no ⇥ bölüm ⇥ ayet ⇥ metin
#  Metnin kendisine dokunulmaz (CC BY-ND 4.0: kelime ve noktalama değiştirilemez).
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail

URL="https://ebible.org/Scriptures/turytc_vpl.zip"
OUT="$(cd "$(dirname "$0")/.." && pwd)/data/ytc.tsv"

if [[ $# -ge 1 ]]; then
  VPL="$1"
else
  TMP=$(mktemp -d)
  trap 'rm -rf "$TMP"' EXIT
  printf 'İndiriliyor: %s\n' "$URL" >&2
  curl -fsSL "$URL" -o "$TMP/vpl.zip"
  unzip -q "$TMP/vpl.zip" -d "$TMP"
  VPL="$TMP/turytc_vpl.txt"
fi

# eBible VPL kodu ⇥ Türkçe ad (YTC'nin kendi başlıkları) ⇥ kısaltma
KITAPLAR="GEN	Yaratılış	yar
EXO	Çıkış	çık
LEV	Levililer	lev
NUM	Sayılar	say
DEU	Yasa'nın Tekrarı	yas
JOS	Yeşu	yşu
JDG	Hâkimler	hak
RUT	Rut	rut
1SA	1 Samuel	1sa
2SA	2 Samuel	2sa
1KI	1 Krallar	1kr
2KI	2 Krallar	2kr
1CH	1 Tarihler	1ta
2CH	2 Tarihler	2ta
EZR	Ezra	ezr
NEH	Nehemya	neh
EST	Ester	est
JOB	İyov	iyo
PSA	Mezmurlar	mez
PRO	Özdeyişler	özd
ECC	Vaiz	vai
SOL	Solomon'un Ezgisi	ezg
ISA	Yeşaya	yşa
JER	Yeremya	yer
LAM	Ağıtlar	ağı
EZE	Hezekiel	hez
DAN	Daniel	dan
HOS	Hoşea	hoş
JOE	Yoel	yoe
AMO	Amos	amo
OBA	Ovadya	ova
JON	Yona	yon
MIC	Mika	mik
NAH	Nahum	nah
HAB	Habakkuk	hab
ZEP	Sefanya	sef
HAG	Hagay	hag
ZEC	Zekariya	zek
MAL	Malaki	mal
MAT	Matta	mat
MAR	Markos	mar
LUK	Luka	luk
JOH	Yuhanna	yuh
ACT	Elçilerin İşleri	elç
ROM	Romalılar	rom
1CO	1 Korintliler	1ko
2CO	2 Korintliler	2ko
GAL	Galatyalılar	gal
EPH	Efesliler	efe
PHI	Filipililer	flp
COL	Koloseliler	kol
1TH	1 Selanikliler	1se
2TH	2 Selanikliler	2se
1TI	1 Timoteos	1ti
2TI	2 Timoteos	2ti
TIT	Titus	tit
PHM	Filimon	flm
HEB	İbraniler	ibr
JAM	Yakov	yak
1PE	1 Petrus	1pe
2PE	2 Petrus	2pe
1JO	1 Yuhanna	1yu
2JO	2 Yuhanna	2yu
3JO	3 Yuhanna	3yu
JUD	Yahuda	yah
REV	Vahiy	vah"

mkdir -p "$(dirname "$OUT")"

printf '%s\n' "$KITAPLAR" | LC_ALL=C awk '
  NR == FNR { split($0, k, "\t"); ad[k[1]] = k[2]; kisa[k[1]] = k[3]; no[k[1]] = NR; next }
  {
    sub(/\r$/, "")
    if (!($1 in ad)) { print "Bilinmeyen kitap kodu: " $1 > "/dev/stderr"; exit 1 }
    metin = $0
    sub(/^[^ ]+ [^ ]+ ?/, "", metin)
    if (metin == "") next          # WEB tabanında boş bırakılan ayetler (ör. Luka 17:36)
    split($2, ba, ":")
    printf "%s\t%s\t%d\t%d\t%d\t%s\n", ad[$1], kisa[$1], no[$1], ba[1], ba[2], metin
  }
' - "$VPL" > "$OUT.tmp"

mv "$OUT.tmp" "$OUT"
printf '%s: %d ayet yazıldı\n' "$OUT" "$(wc -l < "$OUT")" >&2

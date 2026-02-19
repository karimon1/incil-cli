# incil-cli

Türkçe Yeni Ahit'ten terminal üzerinde ayet görüntüleyici.

![bash](https://img.shields.io/badge/bash-4.0%2B-green) ![platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-blue) ![license](https://img.shields.io/badge/license-MIT-yellow)


## Hızlı Kurulum
```bash
git clone https://github.com/karimon1/incil-cli.git
cd incil-cli
./install.sh
```
## Kurulum

**1. Repoyu klonla:**
```bash
git clone https://github.com/KULLANICI_ADIN/incil-cli.git
cd incil-cli
```

**2. Script'i kopyala ve çalıştırılabilir yap:**
```bash
cp incil ~/.local/bin/incil
chmod +x ~/.local/bin/incil
```

**3. `~/.local/bin` PATH'inde değilse ekle:**

_Bash:_
```bash
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc && source ~/.bashrc
```

_Fish:_
```bash
fish_add_path ~/.local/bin
```

**4. Veri otomatik indirilir, test et:**
```bash
incil
```

> İlk çalıştırmada `~/.local/share/incil/turkish.xml` (~5MB) otomatik olarak indirilir. Sonraki çalıştırmalarda ağ bağlantısı gerekmez.


## Kullanım

```bash
incil                # rastgele ayet (tüm Yeni Ahit)
incil mat 3:16       # Matta 3:16
incil yuh 1          # Yuhanna 1. bölümün tamamı
incil rom 8          # Romalılar 8. bölüm
incil -l             # kitap listesi
incil -h             # yardım
```

## Kitap Kısaltmaları

| Kısaltma | Kitap               |
|----------|---------------------|
| `mat`    | Matta               |
| `mar`    | Markos              |
| `luk`    | Luka                |
| `yuh`    | Yuhanna             |
| `isi`    | Elçilerin İşleri    |
| `rom`    | Romalılar           |
| `gal`    | Galatlar            |
| `efe`    | Efesliler           |
| `phi`    | Filipililer         |
| `kol`    | Koloseliler         |
| `tit`    | Titus               |
| `flm`    | Filimona            |
| `ibr`    | İbraniler           |
| `yak`    | Yakup               |
| `yah`    | Yahuda              |
| `vah`    | Vahiy               |

## Gereksinimler

- `bash` 4.0+
- `curl` veya `wget`
- `grep`, `sed`, `shuf` (standart coreutils)

## Veri Kaynağı

[christos-c/bible-corpus](https://github.com/christos-c/bible-corpus) — Creative Commons lisanslı çok dilli İncil derlemesi.

## Lisans

MIT — ayrıntılar için [LICENSE](LICENSE) dosyasına bak.

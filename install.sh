#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────
#  incil-cli kurulum scripti
#  Kullanım: ./install.sh
# ─────────────────────────────────────────────────────────────

set -e

cd "$(dirname "$0")"

INSTALL_DIR="$HOME/.local/bin"
DATA_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/incil"

printf '\n\033[1;37m incil-cli kurulumu\033[0m\n'
printf ' \033[1;30m────────────────────────────────\033[0m\n\n'

# ── Dizinler ─────────────────────────────────────────────────
mkdir -p "$INSTALL_DIR" "$DATA_DIR"

# ── Script ve veri ───────────────────────────────────────────
install -m755 incil "$INSTALL_DIR/incil"
printf ' \033[1;32m✓\033[0m Script kopyalandı: %s/incil\n' "$INSTALL_DIR"

install -m644 data/ytc.tsv "$DATA_DIR/ytc.tsv"
printf ' \033[1;32m✓\033[0m Veri kopyalandı: %s/ytc.tsv\n' "$DATA_DIR"

if [[ -f "$DATA_DIR/turkish.xml" ]]; then
  printf ' \033[1;33m⚠\033[0m  Eski veri dosyası artık kullanılmıyor, silebilirsiniz:\n'
  printf '     rm %s/turkish.xml\n' "$DATA_DIR"
fi

# ── TUI (isteğe bağlı: derleyici ve ncurses varsa) ───────────
if command -v "${CC:-cc}" &>/dev/null && make -s incil-tui 2>/dev/null; then
  install -m755 incil-tui "$INSTALL_DIR/incil-tui"
  printf ' \033[1;32m✓\033[0m TUI derlendi: %s/incil-tui\n' "$INSTALL_DIR"
else
  printf ' \033[1;33m⚠\033[0m  incil-tui derlenemedi (cc ve ncurses gerekli), atlandı\n'
fi

# ── PATH kontrolü ve shell yapılandırması ────────────────────
add_to_path() {
  local config_file="$1"
  local line='export PATH="$HOME/.local/bin:$PATH"'
  if [[ -f "$config_file" ]] && grep -q '\.local/bin' "$config_file"; then
    printf ' \033[1;32m✓\033[0m PATH zaten tanımlı: %s\n' "$config_file"
  else
    printf '\n%s\n' "$line" >> "$config_file"
    printf ' \033[1;32m✓\033[0m PATH eklendi: %s\n' "$config_file"
  fi
}

if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
  # Fish
  if [[ -f "$HOME/.config/fish/config.fish" ]]; then
    if grep -q '\.local/bin' "$HOME/.config/fish/config.fish"; then
      printf ' \033[1;32m✓\033[0m PATH zaten tanımlı: ~/.config/fish/config.fish\n'
    else
      printf '\nfish_add_path ~/.local/bin\n' >> "$HOME/.config/fish/config.fish"
      printf ' \033[1;32m✓\033[0m PATH eklendi: ~/.config/fish/config.fish\n'
    fi
  fi
  # Bash
  [[ -f "$HOME/.bashrc" ]] && add_to_path "$HOME/.bashrc"
  # Zsh
  [[ -f "$HOME/.zshrc" ]]  && add_to_path "$HOME/.zshrc"
else
  printf ' \033[1;32m✓\033[0m PATH zaten ayarlı\n'
fi

# ── Bitti ─────────────────────────────────────────────────────
printf '\n \033[1;32m✓ Kurulum tamamlandı!\033[0m\n\n'
printf ' Deneyin:\n'
printf '   \033[0;36mincil yuh 3:16\033[0m\n'
printf '   \033[0;36mincil -i\033[0m          (etkileşimli mod)\n'
printf '   \033[0;36mincil-tui\033[0m         (görsel arayüz)\n\n'

#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────
#  incil-cli kurulum scripti
#  Kullanım: ./install.sh
# ─────────────────────────────────────────────────────────────

set -e

BINARY="incil"
INSTALL_DIR="$HOME/.local/bin"

printf '\n\033[1;37m incil-cli kurulumu\033[0m\n'
printf ' \033[1;30m────────────────────────────────\033[0m\n\n'

# ── ~/.local/bin dizini yoksa oluştur ────────────────────────
if [[ ! -d "$INSTALL_DIR" ]]; then
  mkdir -p "$INSTALL_DIR"
  printf ' \033[1;32m✓\033[0m Dizin oluşturuldu: %s\n' "$INSTALL_DIR"
fi

# ── Script'i kopyala ─────────────────────────────────────────
cp "$BINARY" "$INSTALL_DIR/$BINARY"
chmod +x "$INSTALL_DIR/$BINARY"
printf ' \033[1;32m✓\033[0m Script kopyalandı: %s/%s\n' "$INSTALL_DIR" "$BINARY"

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

if ! echo "$PATH" | grep -q "$HOME/.local/bin"; then
  # Fish
  if [[ -f "$HOME/.config/fish/config.fish" ]]; then
    FISH_LINE='fish_add_path ~/.local/bin'
    if grep -q '\.local/bin' "$HOME/.config/fish/config.fish"; then
      printf ' \033[1;32m✓\033[0m PATH zaten tanımlı: ~/.config/fish/config.fish\n'
    else
      printf '\n%s\n' "$FISH_LINE" >> "$HOME/.config/fish/config.fish"
      printf ' \033[1;32m✓\033[0m PATH eklendi: ~/.config/fish/config.fish\n'
    fi
  fi
  # Bash
  [[ -f "$HOME/.bashrc" ]]  && add_to_path "$HOME/.bashrc"
  # Zsh
  [[ -f "$HOME/.zshrc" ]]   && add_to_path "$HOME/.zshrc"
else
  printf ' \033[1;32m✓\033[0m PATH zaten ayarlı\n'
fi

# ── jq kontrolü ──────────────────────────────────────────────
if ! command -v jq &>/dev/null; then
  printf '\n \033[1;33m⚠\033[0m  jq kurulu değil (isteğe bağlı)\n'
fi

# ── Bitti ─────────────────────────────────────────────────────
printf '\n \033[1;32m✓ Kurulum tamamlandı!\033[0m\n\n'
printf ' Yeni terminalde çalıştır:\n'
printf '   \033[0;36mincil\033[0m\n\n'
printf ' Mevcut terminalde hemen denemek için:\n'

if [[ -f "$HOME/.config/fish/config.fish" ]]; then
  printf '   \033[0;36msource ~/.config/fish/config.fish && incil\033[0m\n\n'
else
  printf '   \033[0;36msource ~/.bashrc && incil\033[0m\n\n'
fi

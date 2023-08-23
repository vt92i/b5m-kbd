#pragma once

#define check_bit(v, b) ((bool)((v >> b) & 1))
#define set_bit(v, b) (v |= (1 << b))
#define unset_bit(v, b) (v &= ~(1 << b))

static int ec_check_bit(u8 addr, u8 bit, bool *output) {
  int result;
  u8 stored;

  result = ec_read(addr, &stored);
  if (result < 0) return result;

  *output = check_bit(stored, bit);

  return 0;
}

static int ec_set_bit(u8 addr, u8 bit) {
  int result;
  u8 stored;

  result = ec_read(addr, &stored);
  if (result < 0) return result;

  set_bit(stored, bit);

  return ec_write(addr, stored);
}

static int ec_unset_bit(u8 addr, u8 bit) {
  int result;
  u8 stored;

  result = ec_read(addr, &stored);
  if (result < 0) return result;

  unset_bit(stored, bit);

  return ec_write(addr, stored);
}

static int ec_read_seq(u8 addr, u8 *buf, u8 len) {
  int result;
  for (u8 i = 0; i < len; i++) {
    result = ec_read(addr + i, buf + i);
    if (result < 0) return result;
  }
  return 0;
}

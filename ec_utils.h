#pragma once

#define check_bit(v, b) ((bool)((v >> b) & 1))
#define set_bit(v, b) (v |= (1 << b))
#define unset_bit(v, b) (v &= ~(1 << b))

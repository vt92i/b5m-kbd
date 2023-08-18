#include <linux/acpi.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>

#define MSI_EC_FW_VERSION_ADDRESS 0xA0
#define MSI_EC_FW_VERSION_LENGTH 12

#define MSI_EC_KB_MICMUTE_LED_ADDRESS 0x2B
#define MSI_EC_KB_MICMUTE_LED_BIT 2

#define MSI_EC_KB_VOLUMEMUTE_LED_ADDRESS 0x2C
#define MSI_EC_KB_VOLUMEMUTE_LED_BIT 2

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

static int ec_get_firmware_version(u8 buf[MSI_EC_FW_VERSION_LENGTH + 1]) {
  int result;

  memset(buf, 0, MSI_EC_FW_VERSION_LENGTH + 1);
  result = ec_read_seq(MSI_EC_FW_VERSION_ADDRESS, buf, MSI_EC_FW_VERSION_LENGTH);
  if (result < 0) return result;

  return MSI_EC_FW_VERSION_LENGTH + 1;
}

static int __init hello_init(void) {
  int result;
  bool micmute_led;
  u8 buf[MSI_EC_FW_VERSION_LENGTH + 1];

  // Get the firmware version
  result = ec_get_firmware_version(buf);
  if (result < 0) return result;

  printk(KERN_INFO "MSI EC Firmware Version: %s\n", buf);

  // Turn on the micmute LED
  result = ec_set_bit(MSI_EC_KB_MICMUTE_LED_ADDRESS, MSI_EC_KB_MICMUTE_LED_BIT);
  if (result < 0) return result;

  result = ec_check_bit(MSI_EC_KB_MICMUTE_LED_ADDRESS, MSI_EC_KB_MICMUTE_LED_BIT, &micmute_led);
  if (result < 0) return result;

  printk(KERN_INFO "MSI EC Keyboard Micmute LED: %s\n", micmute_led ? "on" : "off");

  // Turn on the volumemute LED
  result = ec_set_bit(MSI_EC_KB_VOLUMEMUTE_LED_ADDRESS, MSI_EC_KB_VOLUMEMUTE_LED_BIT);
  if (result < 0) return result;

  result = ec_check_bit(MSI_EC_KB_VOLUMEMUTE_LED_ADDRESS, MSI_EC_KB_VOLUMEMUTE_LED_BIT, &micmute_led);
  if (result < 0) return result;

  printk(KERN_INFO "MSI EC Keyboard Volumemute LED: %s\n", micmute_led ? "on" : "off");

  return 0;
}

static void __exit hello_exit(void) { printk(KERN_INFO "Goodbye, world!\n"); }

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fila <fe@vt92i.dev>");
MODULE_DESCRIPTION("MSI Modern B5M Keyboard Backlight Kernel Module");
MODULE_VERSION("0.24");

module_init(hello_init);
module_exit(hello_exit);

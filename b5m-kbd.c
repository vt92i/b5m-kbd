#include <acpi/battery.h>
#include <linux/acpi.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/leds.h>
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

static enum led_brightness audiomute_led_get(struct led_classdev *led_cdev) {
  bool audiomute_led_status;
  int result;

  result = ec_check_bit(MSI_EC_KB_VOLUMEMUTE_LED_ADDRESS, MSI_EC_KB_VOLUMEMUTE_LED_BIT, &audiomute_led_status);
  if (result < 0) return result;

  return audiomute_led_status ? LED_ON : LED_OFF;
}

static void audiomute_led_set(struct led_classdev *led_cdev, enum led_brightness brightness) {
  int result;

  if (brightness == LED_OFF) {
    result = ec_unset_bit(MSI_EC_KB_VOLUMEMUTE_LED_ADDRESS, MSI_EC_KB_VOLUMEMUTE_LED_BIT);
    if (result < 0) return;
  } else {
    result = ec_set_bit(MSI_EC_KB_VOLUMEMUTE_LED_ADDRESS, MSI_EC_KB_VOLUMEMUTE_LED_BIT);
    if (result < 0) return;
  }
}

static struct led_classdev audiomute_led_cdev = {
    .name = "platform::audiomute",
    .default_trigger = "audio-mute",
    .max_brightness = LED_ON,
    .brightness = LED_OFF,
    .brightness_get = audiomute_led_get,
    .brightness_set = audiomute_led_set,
};

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
  // Declare variables
  int result;
  bool micmute_led;
  u8 buf[MSI_EC_FW_VERSION_LENGTH + 1];

  // Register the audiomute LED
  result = led_classdev_register(NULL, &audiomute_led_cdev);
  if (result < 0)
    printk(KERN_ERR "Failed to register audiomute LED\n");
  else
    printk(KERN_INFO "Registered audiomute LED\n");

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
  // result = ec_set_bit(MSI_EC_KB_VOLUMEMUTE_LED_ADDRESS, MSI_EC_KB_VOLUMEMUTE_LED_BIT);
  // if (result < 0) return result;

  result = ec_check_bit(MSI_EC_KB_VOLUMEMUTE_LED_ADDRESS, MSI_EC_KB_VOLUMEMUTE_LED_BIT, &micmute_led);
  if (result < 0) return result;

  printk(KERN_INFO "MSI EC Keyboard Volumemute LED: %s\n", micmute_led ? "on" : "off");

  return 0;
}

static void __exit hello_exit(void) {
  led_classdev_unregister(&audiomute_led_cdev);
  ec_unset_bit(MSI_EC_KB_MICMUTE_LED_ADDRESS, MSI_EC_KB_MICMUTE_LED_BIT);
  ec_unset_bit(MSI_EC_KB_VOLUMEMUTE_LED_ADDRESS, MSI_EC_KB_VOLUMEMUTE_LED_BIT);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fila <fe@vt92i.dev>");
MODULE_DESCRIPTION("MSI Modern 14 - B5M Keyboard Backlight and LEDs Kernel Module");
MODULE_VERSION("0.24");

module_init(hello_init);
module_exit(hello_exit);
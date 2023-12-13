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

#include "ec_address.h"
#include "ec_utils.h"

static int ec_get_firmware_version(u8 buf[EC_FW_VERSION_LENGTH + 1]) {
  int result;

  memset(buf, 0, EC_FW_VERSION_LENGTH + 1);
  result = ec_read_seq(EC_FW_VERSION_ADDRESS, buf, EC_FW_VERSION_LENGTH);
  if (result < 0) return result;

  return EC_FW_VERSION_LENGTH + 1;
}

static int audiomute_led_set(struct led_classdev *led_cdev, enum led_brightness brightness) {
  int result = brightness == LED_OFF ? ec_unset_bit(EC_KBD_AUDIOMUTE_LED_ADDRESS, EC_KBD_AUDIOMUTE_LED_BIT)
                                     : ec_set_bit(EC_KBD_AUDIOMUTE_LED_ADDRESS, EC_KBD_AUDIOMUTE_LED_BIT);
  return result < 0 ? result : 0;
}

static struct led_classdev audiomute_led_cdev = {
    .name = "platform::audiomute",
    .default_trigger = "audio-mute",
    .max_brightness = LED_ON,
    .brightness_set_blocking = audiomute_led_set,
};

static int micmute_led_set(struct led_classdev *led_cdev, enum led_brightness brightness) {
  int result = brightness == LED_OFF ? ec_unset_bit(EC_KBD_MICMUTE_LED_ADDRESS, EC_KBD_MICMUTE_LED_BIT)
                                     : ec_set_bit(EC_KBD_MICMUTE_LED_ADDRESS, EC_KBD_MICMUTE_LED_BIT);
  return result < 0 ? result : 0;
}

static struct led_classdev micmute_led_cdev = {
    .name = "platform::micmute",
    .default_trigger = "audio-micmute",
    .max_brightness = LED_ON,
    .brightness_set_blocking = micmute_led_set,
};

static int __init hello_init(void) {
  int result;
  u8 buf[EC_FW_VERSION_LENGTH + 1];

  result = ec_get_firmware_version(buf);
  if (result < 0) return result;

  printk(KERN_INFO "b5m-kbd: %s (EC Firmware Version)\n", buf);

  // Cooler Boost
  result = ec_set_bit(0x98, 7);
  if (result < 0) return result;

  // for (u8 i = 0x0; i <= 0xF; i++) {
  //   u8 value;
  //   u8 address_base = i * 0x10;
  //   printk(KERN_INFO "MSI B5M - EC - 0x%02X\n", address_base);
  //   for (u8 j = 0x0; j <= 0xF; j++) {
  //     result = ec_read(address_base + j, &value);
  //     if (result < 0) return result;
  //     printk(KERN_INFO "\tMSI B5M - EC 0x%02X: 0x%02X (%d)\n", address_base + j, value, value);
  //   }
  // }

  result = led_classdev_register(NULL, &audiomute_led_cdev);
  if (result < 0) return result;

  result = led_classdev_register(NULL, &micmute_led_cdev);
  if (result < 0) return result;

  return 0;
}

static void __exit hello_exit(void) {
  led_classdev_unregister(&audiomute_led_cdev);
  led_classdev_unregister(&micmute_led_cdev);
  ec_unset_bit(0x98, 7);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fila <fe@vt92i.dev>");
MODULE_DESCRIPTION("MSI Modern 14 - B5M Keyboard Backlight and LEDs Kernel Module");
MODULE_VERSION("0.24");

module_init(hello_init);
module_exit(hello_exit);

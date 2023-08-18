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

#define MSI_EC_FW_VERSION_ADDRESS 0xa0
#define MSI_EC_FW_VERSION_LENGTH 12

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fila");
MODULE_DESCRIPTION("MSI Modern 14 B5M KDB Driver");
MODULE_VERSION("0.24");

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
  u8 buf[MSI_EC_FW_VERSION_LENGTH + 1];

  result = ec_get_firmware_version(buf);
  if (result < 0) return result;

  printk(KERN_INFO "MSI EC Firmware Version: %s\n", buf);

  printk(KERN_INFO "Hello, world!\n");
  return 0;
}

static void __exit hello_exit(void) { printk(KERN_INFO "Goodbye, world!\n"); }

module_init(hello_init);
module_exit(hello_exit);

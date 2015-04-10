/*
 * Please note that the name are used in mkflash script. Therefore
 * don't change them.  If you want to add different partitions, you
 * will need to modify mkflash script as well so that the end image
 * is what you include here!
 *
 * Also, the 7th item is always the size, so please don't add extra
 * spaces in the name or other items.
 * 
 *  - Alan
 */

static struct mtd_partition sl2312_partitions[] = {
	{ name: "RedBoot",     	 offset: 0x00000000, size: 0x00020000, },
	{ name: "Kernel",      	 offset: 0x00020000, size: 0x00300000, },
	{ name: "Ramdisk",     	 offset: 0x00320000, size: 0x00600000, },
	{ name: "Application", 	 offset: 0x00920000, size: 0x00600000, },
	{ name: "VCTL", 	 offset: 0x00F20000, size: 0x00020000, },
	{ name: "CurConf", 	 offset: 0x00F40000, size: 0x000A0000, },
	{ name: "FIS directory", offset: 0x00FE0000, size: 0x00020000, }
};

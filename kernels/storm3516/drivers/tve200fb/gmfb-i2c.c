#include <linux/i2c.h>
#if 0
unsigned char TV_CS4954_init_NTSC[11][2] = {
 {0x00, 0x2b}, {0x01, 0x02}, {0x02, 0x00}, {0x03, 0x04},
 {0x04, 0x07}, {0x05, 0x58}, {0x10, 0x1C}, {0x11, 0x3E},
 {0x12, 0xF8}, {0x13, 0xE0}, {0x14, 0x43}
};

 #else
unsigned char TV_CS4954_init_NTSC[11][2] = {
	{0x00, 0x2b}, {0x01, 0x02}, {0x02, 0x00}, {0x03, 0x10},
	{0x04, 0x07}, {0x05, 0x60}, {0x10, 0x1C}, {0x11, 0x3E},
	{0x12, 0xF8}, {0x13, 0xE0}, {0x14, 0x43}
};
#endif


unsigned char TV_init_NTSC[8][2] = {
	{0x27, 0x00}, {0x3a, 0x93}, {0x6b, 0x12}, {0x5f, 0x3f},
	{0x62, 0x46}, {0x2e, 0x00}, {0x6f, 0x14}, {0x7d, 0x00},
};

unsigned char TV_NTSC[21][2] = {
	{0x3a, 0x13}, {0x28, 0x19}, {0x29, 0x1d}, {0x5a, 0x88},
	{0x5d, 0x2a}, {0x5b, 0x76}, {0x5e, 0x2e}, {0x5c, 0xa5},
	{0x5f, 0x2e}, {0x61, 0x11}, {0x62, 0x3f}, {0x66, 0x21},
	{0x65, 0xf0}, {0x64, 0x7c}, {0x63, 0x1f}, {0x6e, 0x90},
	{0x7c, 0xc0}, {0x7a, 0x1a}, {0x7b, 0x01}, {0x6c, 0xf9},
	{0x6d, 0x00},
};

unsigned char AnalogLCD_NTSC[208][2] = {
	{0x20, 0x00}, {0x21, 0xC0}, {0x22, 0x02}, {0x23, 0x06},
	{0x24, 0x02}, {0x25, 0x04}, {0x26, 0x28}, {0x27, 0xD2},
	{0x28, 0x0D}, {0x29, 0x83}, {0x2A, 0x00}, {0x2B, 0x41},
	{0x2C, 0x00}, {0x2D, 0x00}, {0x2E, 0x00}, {0x2F, 0x00},
	{0x30, 0x04}, {0x31, 0x0C}, {0x32, 0xE0}, {0x33, 0x01},
	{0x34, 0x90}, {0x35, 0x01}, {0x36, 0x04}, {0x37, 0x45},
	{0x38, 0x40}, {0x39, 0x01}, {0x3A, 0x00}, {0x3B, 0x00},
	{0x3C, 0x00}, {0x3D, 0x00}, {0x3E, 0x00}, {0x3F, 0x00},
	{0x40, 0xAD}, {0x41, 0x00}, {0x42, 0x02}, {0x43, 0x08},
	{0x44, 0x7F}, {0x45, 0x78}, {0x46, 0x22}, {0x47, 0x07},
	{0x48, 0x77}, {0x49, 0x01}, {0x4A, 0x30}, {0x4B, 0xE2},
	{0x4C, 0x3B}, {0x4D, 0x7A}, {0x4E, 0x3D}, {0x4F, 0x08},
	{0x50, 0x7B}, {0x51, 0x37}, {0x52, 0x80}, {0x53, 0x8C},
	{0x54, 0x00}, {0x55, 0xFF}, {0x56, 0x10}, {0x57, 0x3A},
	{0x58, 0x20}, {0x59, 0x00}, {0x5A, 0x03}, {0x5B, 0x00},
	{0x5C, 0x00}, {0x5D, 0x00}, {0x5E, 0x00}, {0x5F, 0x00},
	{0x60, 0x00}, {0x61, 0x12}, {0x62, 0xD2}, {0x63, 0xC0},
	{0x64, 0x00}, {0x65, 0x59}, {0x66, 0x59}, {0x67, 0x33},
	{0x68, 0x00}, {0x69, 0x20}, {0x6A, 0x00}, {0x6B, 0x00},
	{0x6C, 0x2E}, {0x6D, 0x00}, {0x6E, 0x00}, {0x6F, 0x00},
	{0x70, 0x00}, {0x71, 0x00}, {0x72, 0x00}, {0x73, 0x00},
	{0x74, 0x00}, {0x75, 0x00}, {0x76, 0x00}, {0x77, 0x00},
	{0x78, 0x00}, {0x79, 0x00}, {0x7A, 0x00}, {0x7B, 0x00},
	{0x7C, 0x00}, {0x7D, 0x00}, {0x7E, 0x00}, {0x7F, 0x00},
	{0x80, 0x0F}, {0x81, 0x00}, {0x82, 0x00}, {0x83, 0x00},
	{0x84, 0x00}, {0x85, 0x03}, {0x86, 0xF1}, {0x87, 0xFC},
	{0x88, 0x10}, {0x89, 0xE0}, {0x8A, 0x82}, {0x8B, 0x1E},
	{0x8C, 0x80}, {0x8D, 0x36}, {0x8E, 0x00}, {0x8F, 0x18},
	{0x90, 0x10}, {0x91, 0xAA}, {0x92, 0x06}, {0x93, 0x00},
	{0x94, 0x00}, {0x95, 0x00}, {0x96, 0x00}, {0x97, 0x00},
	{0x98, 0x00}, {0x99, 0x00}, {0x9A, 0x00}, {0x9B, 0x00},
	{0x9C, 0x00}, {0x9D, 0x00}, {0x9E, 0x00}, {0x9F, 0x00},
	{0xA0, 0x00}, {0xA1, 0x00}, {0xA2, 0x00}, {0xA3, 0x00},
	{0xA4, 0x00}, {0xA5, 0x00}, {0xA6, 0x00}, {0xA7, 0x00},
	{0xA8, 0x00}, {0xA9, 0x00}, {0xAA, 0x00}, {0xAB, 0x00},
	{0xAC, 0x00}, {0xAD, 0x00}, {0xAE, 0x00}, {0xAF, 0x00},
	{0xB0, 0x40}, {0xB1, 0x00}, {0xB2, 0x00}, {0xB3, 0x00},
	{0xB4, 0x00}, {0xB5, 0x00}, {0xB6, 0x00}, {0xB7, 0x00},
	{0xB8, 0x00}, {0xB9, 0x11}, {0xBA, 0x01}, {0xBB, 0x11},
	{0xBC, 0x1C}, {0xBD, 0x02}, {0xBE, 0x20}, {0xBF, 0x18},
	{0xC0, 0x13}, {0xC1, 0x66}, {0xC2, 0x10}, {0xC3, 0x86},
	{0xC4, 0x01}, {0xC5, 0x00}, {0xC6, 0x00}, {0xC7, 0x00},
	{0xC8, 0x00}, {0xC9, 0x00}, {0xCA, 0x00}, {0xCB, 0x00},
	{0xCC, 0x00}, {0xCD, 0x00}, {0xCE, 0x00}, {0xCF, 0x00},
	{0xD0, 0x1E}, {0xD1, 0x02}, {0xD2, 0xE0}, {0xD3, 0x01},
	{0xD4, 0x08}, {0xD5, 0x64}, {0xD6, 0x50}, {0xD7, 0x10},
	{0xD8, 0x24}, {0xD9, 0x05}, {0xDA, 0x02}, {0xDB, 0x01},
	{0xDC, 0x00}, {0xDD, 0x00}, {0xDE, 0x00}, {0xDF, 0x00},
	{0xE0, 0x00}, {0xE1, 0x00}, {0xE2, 0x27}, {0xE3, 0x00},
	{0xE4, 0x00}, {0xE5, 0xFF}, {0xE6, 0x00}, {0xE7, 0x00},
	{0xE8, 0x00}, {0xE9, 0x00}, {0xEA, 0x00}, {0xEB, 0x00},
	{0xEC, 0x00}, {0xED, 0x00}, {0xEE, 0x00}, {0xEF, 0x00},
};

unsigned char VGA_NTSC[451][2] = {
	{0x01, 0x00}, {0x02, 0x08}, {0x03, 0x00}, {0x04, 0x00},
	{0x05, 0x00}, {0x06, 0xc0}, {0x07, 0x0b}, {0x08, 0x00},
	{0x09, 0x82}, {0x0a, 0x00}, {0x0b, 0x00}, {0x0c, 0x00},
	{0x0d, 0x00}, {0x0e, 0x00}, {0x0f, 0x01}, {0x10, 0x00},
	{0x11, 0x02}, {0x12, 0x00}, {0x13, 0x00}, {0x14, 0x04},
	{0x16, 0x30}, {0x17, 0x88}, {0x18, 0x00}, {0x19, 0x00},
	{0x1a, 0x00}, {0x1f, 0x1e},

	{0x0e, 0x01}, {0x20, 0x51}, {0x21, 0x00}, {0x22, 0xd0},
	{0x23, 0x02}, {0x24, 0xd0}, {0x25, 0x02}, {0x26, 0x11},
	{0x27, 0x00}, {0x28, 0xf5}, {0x29, 0x00}, {0x2a, 0xf5},	//f5 //f6//fa
	{0x2b, 0x00}, {0x2c, 0x00}, {0x2d, 0x00}, {0x2e, 0x01},
	{0x2f, 0x00}, {0x30, 0x00}, {0x31, 0x01}, {0x32, 0x00},
	{0x33, 0x01}, {0x34, 0x00}, {0x35, 0x00}, {0x36, 0x00},
	{0x37, 0x00}, {0x38, 0x20}, {0x39, 0x40}, {0x3a, 0x02},
	{0x3b, 0x04}, {0x3c, 0x00}, {0x3d, 0x00}, {0x3e, 0x00},
	{0x3f, 0x00}, {0x40, 0x00}, {0x41, 0x00}, {0x42, 0x00},
	{0x43, 0x00}, {0x44, 0x00}, {0x45, 0x00}, {0x46, 0x00},
	{0x47, 0x00}, {0x48, 0x00}, {0x49, 0x00}, {0x4a, 0x00},
	{0x4b, 0x00}, {0x4c, 0x00}, {0x4d, 0x00}, {0x4e, 0x00},
	{0x4f, 0x00}, {0x50, 0x10}, {0x51, 0x00}, {0x52, 0x02},
	{0x53, 0x00}, {0x54, 0x3a}, {0x55, 0x03}, {0x56, 0x03},
	{0x57, 0x00}, {0x58, 0x06}, {0x59, 0x01}, {0x5a, 0xc2},
	{0x5b, 0x00}, {0x5c, 0x0a}, {0x5d, 0x03}, {0x5e, 0x06},
	{0x5f, 0x00}, {0x60, 0x05}, {0x61, 0x01}, {0x62, 0x8e},
	{0x63, 0x03}, {0x64, 0x07}, {0x65, 0x8e}, {0x66, 0x03},
	{0x67, 0x08}, {0x68, 0x01}, {0x69, 0x00}, {0x6a, 0x00},
	{0x6b, 0x00}, {0x6c, 0x00}, {0x6d, 0x00}, {0x6e, 0x00},
	{0x6f, 0x00}, {0x70, 0x00}, {0x71, 0x00}, {0x72, 0x00},
	{0x73, 0x00}, {0x74, 0x00}, {0x75, 0x00}, {0x76, 0x00},
	{0x77, 0x00}, {0x78, 0x00}, {0x79, 0x00}, {0x7a, 0x00},
	{0x7b, 0x00}, {0x7c, 0x00}, {0x7d, 0x00}, {0x7e, 0x00},
	{0x7f, 0x00},

	{0x0e, 0x02}, {0x20, 0x09}, {0x21, 0x00}, {0x22, 0x44},
	{0x23, 0x42}, {0x24, 0x00}, {0x25, 0x00}, {0x26, 0x00},
	{0x27, 0x00}, {0x28, 0x01}, {0x29, 0xef}, {0x2a, 0x47},
	{0x2b, 0x00}, {0x2c, 0x00}, {0x2d, 0x00}, {0x2e, 0x00},
	{0x2f, 0x00}, {0x30, 0x00}, {0x31, 0x00}, {0x32, 0x22},
	{0x33, 0x00}, {0x34, 0x60}, {0x35, 0x5a}, {0x36, 0x00},
	{0x37, 0x00}, {0x38, 0x00}, {0x39, 0x00}, {0x3a, 0x00},
	{0x3b, 0x00}, {0x3c, 0x00}, {0x3d, 0x00}, {0x3e, 0x00},
	{0x3f, 0x00}, {0x40, 0x00}, {0x41, 0x00}, {0x42, 0x00},
	{0x43, 0x00}, {0x44, 0x00}, {0x45, 0x00}, {0x46, 0x00},
	{0x47, 0x00}, {0x48, 0x60}, {0x49, 0x5a}, {0x4a, 0x00},
	{0x4b, 0x00}, {0x4c, 0x00}, {0x4d, 0x00}, {0x4e, 0x00},
	{0x4f, 0x00}, {0x50, 0x00}, {0x51, 0x0f}, {0x52, 0x00},
	{0x53, 0x00}, {0x54, 0x00}, {0x55, 0x00}, {0x56, 0x00},
	{0x57, 0x00}, {0x58, 0x00}, {0x59, 0x00}, {0x5a, 0x00},
	{0x5b, 0x00}, {0x5c, 0x00}, {0x5d, 0x00}, {0x5e, 0x00},
	{0x5f, 0x00}, {0x60, 0x00}, {0x61, 0xf8}, {0x62, 0x6e},
	{0x63, 0x25}, {0x64, 0xb5}, {0x65, 0xbd}, {0x66, 0x5f},
	{0x67, 0x00}, {0x68, 0x00}, {0x69, 0x00}, {0x6a, 0x00},
	{0x6b, 0x00}, {0x6c, 0x00}, {0x6d, 0x00}, {0x6e, 0x00},
	{0x6f, 0x00}, {0x70, 0x00}, {0x71, 0x00}, {0x72, 0x00},
	{0x73, 0x00}, {0x74, 0x00}, {0x75, 0x00}, {0x76, 0x00},
	{0x77, 0x00}, {0x78, 0x00}, {0x79, 0x00}, {0x7a, 0x00},
	{0x7b, 0x00}, {0x7c, 0x00}, {0x7d, 0x00}, {0x7e, 0x00},
	{0x7f, 0x00},

	{0x0e, 0x03}, {0x20, 0x10}, {0x21, 0x05}, {0x22, 0x88},
	{0x23, 0x00}, {0x24, 0x18}, {0x25, 0x01}, {0x26, 0x09},
	{0x27, 0x05}, {0x28, 0x26}, {0x29, 0x03}, {0x2a, 0x06},
	{0x2b, 0x00}, {0x2c, 0x23}, {0x2d, 0x00}, {0x2e, 0x23},
	{0x2f, 0x03}, {0x30, 0xdd}, {0x31, 0x00}, {0x32, 0xe9},
	{0x33, 0x04}, {0x34, 0x23}, {0x35, 0x00}, {0x36, 0x23},
	{0x37, 0x03}, {0x38, 0x00}, {0x39, 0x00}, {0x3a, 0x00},
	{0x3b, 0x00}, {0x3c, 0x00}, {0x3d, 0x00}, {0x3e, 0x00},
	{0x3f, 0x00}, {0x40, 0xd0}, {0x41, 0x02}, {0x42, 0x20},
	{0x43, 0x03}, {0x44, 0xea}, {0x45, 0x01}, {0x46, 0x58},
	{0x47, 0x02}, {0x48, 0x03}, {0x49, 0x00}, {0x4a, 0x80},
	{0x4b, 0x16}, {0x4c, 0x40}, {0x4d, 0x14}, {0x4e, 0x00},
	{0x4f, 0x00}, {0x50, 0x00}, {0x51, 0x00}, {0x52, 0x00},
	{0x53, 0x00}, {0x54, 0x00}, {0x55, 0x00}, {0x56, 0x00},
	{0x57, 0x00}, {0x58, 0x20}, {0x59, 0x01}, {0x5a, 0xff},
	{0x5b, 0xdd}, {0x5c, 0x00}, {0x5d, 0x00}, {0x5e, 0x00},
	{0x5f, 0x00}, {0x60, 0x00}, {0x61, 0x00}, {0x62, 0xff},
	{0x63, 0xff}, {0x64, 0xff}, {0x65, 0xff}, {0x66, 0xff},
	{0x67, 0x00}, {0x68, 0x00}, {0x69, 0x00}, {0x6a, 0xff},
	{0x6b, 0x00}, {0x6c, 0xff}, {0x6d, 0xff}, {0x6e, 0xff},
	{0x6f, 0x00}, {0x70, 0x00}, {0x71, 0xff}, {0x72, 0xff},
	{0x73, 0x00}, {0x74, 0x00}, {0x75, 0x00}, {0x76, 0xff},
	{0x77, 0xff}, {0x78, 0x1a}, {0x79, 0x10}, {0x7a, 0x00},
	{0x7b, 0x00}, {0x7c, 0x00}, {0x7d, 0x00}, {0x7e, 0x00},
	{0x7f, 0x00}, {0x80, 0x25}, {0x81, 0x00}, {0x82, 0x00},
	{0x83, 0x00}, {0x84, 0x02}, {0x85, 0x00}, {0x86, 0x00},
	{0x87, 0x00}, {0x88, 0x16}, {0x89, 0x00}, {0x8a, 0x10},
	{0x8b, 0x00}, {0x8c, 0x00}, {0x8d, 0x00}, {0x8e, 0x00},
	{0x8f, 0x00}, {0x90, 0x3d}, {0x91, 0x47}, {0x92, 0x00},
	{0x93, 0x18}, {0x94, 0x07}, {0x95, 0x00}, {0x96, 0x07},
	{0x97, 0x00}, {0x98, 0x0e}, {0x99, 0x00}, {0x9a, 0x0e},
	{0x9b, 0x00}, {0x9c, 0x1c}, {0x9d, 0x09}, {0x9e, 0x00},
	{0x9f, 0x00}, {0xa0, 0x00}, {0xa1, 0x00}, {0xa2, 0x00},
	{0xa3, 0xac}, {0xa4, 0x07}, {0xa5, 0x00}, {0xa6, 0x07},
	{0xa7, 0x00}, {0xa8, 0x0e}, {0xa9, 0x00}, {0xaa, 0x0e},
	{0xab, 0x00}, {0xac, 0x04}, {0xad, 0x02}, {0xae, 0x01},
	{0xaf, 0x01}, {0xb0, 0x00}, {0xb1, 0x00}, {0xb2, 0x00},
	{0xb3, 0x00}, {0xb4, 0x00}, {0xb5, 0x00}, {0xb6, 0x00},
	{0xb7, 0x00}, {0xb8, 0x00}, {0xb9, 0x00}, {0xba, 0x00},
	{0xbb, 0x00}, {0xbc, 0x00}, {0xbd, 0x00}, {0xbe, 0x00},
	{0xbf, 0x00}, {0xc0, 0x00}, {0xc1, 0x00}, {0xc2, 0x00},
	{0xc3, 0x00}, {0xc4, 0x00}, {0xc5, 0x40}, {0xc6, 0x0F},	//10
	{0xc7, 0x00}, {0xc8, 0x01}, {0xc9, 0x00}, {0xca, 0x30},
	{0xcb, 0x00}, {0xcc, 0x00}, {0xcd, 0x00}, {0xce, 0xf0},
	{0xcf, 0x01}, {0xd0, 0x00}, {0xd1, 0x00}, {0xd2, 0x00},
	{0xd3, 0x00}, {0xd4, 0x00}, {0xd5, 0x00}, {0xd6, 0x00},
	{0xd7, 0x00}, {0xd8, 0x00}, {0xd9, 0x00}, {0xda, 0x00},
	{0xdb, 0x00}, {0xdc, 0x00}, {0xdd, 0x00}, {0xde, 0x00},
	{0xdf, 0x00}, {0xe0, 0x00}, {0xe1, 0x00}, {0xe2, 0x00},
	{0xe3, 0x00}, {0xe4, 0x00}, {0xe5, 0x00}, {0xe6, 0x00},
	{0xe7, 0x00}, {0xe8, 0x00}, {0xe9, 0x00}, {0xea, 0x00},
	{0xeb, 0x00}, {0xec, 0x00}, {0xed, 0x00}, {0xee, 0x00},
	{0xef, 0x00}, {0xf0, 0x00}, {0xf1, 0x00}, {0xf2, 0x00},
	{0xf3, 0x00}, {0xf4, 0x00}, {0xf5, 0x00}, {0xf6, 0x00},
	{0xf7, 0x00}, {0xf8, 0x00}, {0xf9, 0x00}, {0xfa, 0x00},
	{0xfb, 0x00}, {0xfc, 0x00}, {0xfd, 0x00}, {0xfe, 0x00},
	{0xff, 0x00},

	//initial issue      
	{0x0e, 0x00}, {0x1b, 0x00}, {0x1c, 0xe1}, {0x1d, 0x14},
	{0x1e, 0xaf}, {0x15, 0x00},
};

#if 0
unsigned char TV_CS4954_init_PAL[11][2] = {
 {0x00, 0x4b}, {0x01, 0x02}, {0x02, 0x00}, {0x03, 0x04},
 {0x04, 0x07}, {0x05, 0x58}, {0x10, 0x15}, {0x11, 0x96},
 {0x12, 0x15}, {0x13, 0x13}, {0x14, 0x54}
};
#else
unsigned char TV_CS4954_init_PAL[11][2] = {
	{0x00, 0x4b}, {0x01, 0x02}, {0x02, 0x00}, {0x03, 0x10},
	{0x04, 0x07}, {0x05, 0x60}, {0x10, 0x15}, {0x11, 0x96},
	{0x12, 0x15}, {0x13, 0x13}, {0x14, 0x54}
};
#endif

unsigned char TV_init_PAL[8][2] = {
	{0x27, 0x00}, {0x3a, 0x93}, {0x6b, 0x12}, {0x5f, 0x3f},
	{0x62, 0x46}, {0x2e, 0x00}, {0x6f, 0x14}, {0x7d, 0x00},
};

unsigned char TV_PAL[21][2] = {
	{0x3a, 0x13}, {0x28, 0x21}, {0x29, 0x1d}, {0x5a, 0x00},
	{0x5d, 0x2a}, {0x5b, 0x7d}, {0x5e, 0x2e}, {0x5c, 0xaf},
	{0x5f, 0x35}, {0x61, 0x02}, {0x62, 0x2f}, {0x66, 0x2a},
	{0x65, 0x09}, {0x64, 0x8a}, {0x63, 0xcb}, {0x6e, 0xa0},
	{0x7c, 0xc0}, {0x7a, 0x1b}, {0x7b, 0x30}, {0x6c, 0x05},
	{0x6d, 0x20},
};

unsigned char AnalogLCD_PAL[208][2] = {
	{0x20, 0x00}, {0x21, 0xC1}, {0x22, 0x02}, {0x23, 0x06},
	{0x24, 0x02}, {0x25, 0x04}, {0x26, 0x29}, {0x27, 0xD3},
	{0x28, 0x0F}, {0x29, 0x93}, {0x2A, 0x00}, {0x2B, 0x4E},
	{0x2C, 0x00}, {0x2D, 0x00}, {0x2E, 0x00}, {0x2F, 0x00},
	{0x30, 0x04}, {0x31, 0x0C}, {0x32, 0xE0}, {0x33, 0x01},
	{0x34, 0x90}, {0x35, 0x01}, {0x36, 0x04}, {0x37, 0x45},
	{0x38, 0x40}, {0x39, 0x01}, {0x3A, 0x00}, {0x3B, 0x00},
	{0x3C, 0x00}, {0x3D, 0x00}, {0x3E, 0x00}, {0x3F, 0x00},
	{0x40, 0xAD}, {0x41, 0x00}, {0x42, 0x02}, {0x43, 0x08},
	{0x44, 0x7F}, {0x45, 0x78}, {0x46, 0x22}, {0x47, 0x09},
	{0x48, 0x19}, {0x49, 0x00}, {0x4A, 0x30}, {0x4B, 0x08},
	{0x4C, 0x46}, {0x4D, 0xA0}, {0x4E, 0x48}, {0x4F, 0x00},
	{0x50, 0x7B}, {0x51, 0x37}, {0x52, 0x80}, {0x53, 0x8C},
	{0x54, 0x00}, {0x55, 0xFF}, {0x56, 0x10}, {0x57, 0x3A},
	{0x58, 0x20}, {0x59, 0x01}, {0x5A, 0x03}, {0x5B, 0x00},
	{0x5C, 0x02}, {0x5D, 0x00}, {0x5E, 0x00}, {0x5F, 0x00},
	{0x60, 0x00}, {0x61, 0x12}, {0x62, 0x07}, {0x63, 0xB0},
	{0x64, 0x00}, {0x65, 0x5F}, {0x66, 0x5F}, {0x67, 0x33},
	{0x68, 0x00}, {0x69, 0x20}, {0x6A, 0x00}, {0x6B, 0x00},
	{0x6C, 0x2E}, {0x6D, 0x00}, {0x6E, 0x00}, {0x6F, 0x00},
	{0x70, 0x00}, {0x71, 0x00}, {0x72, 0x00}, {0x73, 0x00},
	{0x74, 0x00}, {0x75, 0x00}, {0x76, 0x00}, {0x77, 0x00},
	{0x78, 0x00}, {0x79, 0x00}, {0x7A, 0x00}, {0x7B, 0x00},
	{0x7C, 0x00}, {0x7D, 0x00}, {0x7E, 0x00}, {0x7F, 0x00},
	{0x80, 0x0F}, {0x81, 0x00}, {0x82, 0xE5}, {0x83, 0x00},
	{0x84, 0x00}, {0x85, 0x03}, {0x86, 0xF1}, {0x87, 0xFC},
	{0x88, 0x10}, {0x89, 0xE0}, {0x8A, 0x82}, {0x8B, 0x1E},
	{0x8C, 0x80}, {0x8D, 0x36}, {0x8E, 0x00}, {0x8F, 0x18},
	{0x90, 0x10}, {0x91, 0xAA}, {0x92, 0x06}, {0x93, 0x00},
	{0x94, 0x00}, {0x95, 0x00}, {0x96, 0x00}, {0x97, 0x00},
	{0x98, 0x00}, {0x99, 0x00}, {0x9A, 0x00}, {0x9B, 0x00},
	{0x9C, 0x00}, {0x9D, 0x00}, {0x9E, 0x00}, {0x9F, 0x00},
	{0xA0, 0x00}, {0xA1, 0x00}, {0xA2, 0x00}, {0xA3, 0x00},
	{0xA4, 0x00}, {0xA5, 0x00}, {0xA6, 0x00}, {0xA7, 0x00},
	{0xA8, 0x00}, {0xA9, 0x00}, {0xAA, 0x00}, {0xAB, 0x00},
	{0xAC, 0x00}, {0xAD, 0x00}, {0xAE, 0x00}, {0xAF, 0x00},
	{0xB0, 0x40}, {0xB1, 0x00}, {0xB2, 0x00}, {0xB3, 0x00},
	{0xB4, 0x00}, {0xB5, 0x00}, {0xB6, 0x00}, {0xB7, 0x00},
	{0xB8, 0x00}, {0xB9, 0x11}, {0xBA, 0x01}, {0xBB, 0x11},
	{0xBC, 0x1C}, {0xBD, 0x02}, {0xBE, 0x20}, {0xBF, 0x18},
	{0xC0, 0x13}, {0xC1, 0x66}, {0xC2, 0x10}, {0xC3, 0x86},
	{0xC4, 0x01}, {0xC5, 0x00}, {0xC6, 0x00}, {0xC7, 0x00},
	{0xC8, 0x00}, {0xC9, 0x00}, {0xCA, 0x00}, {0xCB, 0x00},
	{0xCC, 0x00}, {0xCD, 0x00}, {0xCE, 0x00}, {0xCF, 0x00},
	{0xD0, 0x1E}, {0xD1, 0x02}, {0xD2, 0xE0}, {0xD3, 0x01},
	{0xD4, 0x08}, {0xD5, 0x64}, {0xD6, 0x50}, {0xD7, 0x10},
	{0xD8, 0x24}, {0xD9, 0x05}, {0xDA, 0x02}, {0xDB, 0x01},
	{0xDC, 0x00}, {0xDD, 0x00}, {0xDE, 0x00}, {0xDF, 0x00},
	{0xE0, 0x00}, {0xE1, 0x00}, {0xE2, 0x27}, {0xE3, 0x00},
	{0xE4, 0x00}, {0xE5, 0xFF}, {0xE6, 0x00}, {0xE7, 0x00},
	{0xE8, 0x00}, {0xE9, 0x00}, {0xEA, 0x00}, {0xEB, 0x00},
	{0xEC, 0x00}, {0xED, 0x00}, {0xEE, 0x00}, {0xEF, 0x00},
};

unsigned char VGA_PAL[451][2] = {
	{0x01, 0x00}, {0x02, 0x08}, {0x03, 0x00}, {0x04, 0x00},
	{0x05, 0x00}, {0x06, 0xc0}, {0x07, 0x0b}, {0x08, 0x00},
	{0x09, 0x82}, {0x0a, 0x00}, {0x0b, 0x00}, {0x0c, 0x00},
	{0x0d, 0x00}, {0x0e, 0x00}, {0x0f, 0x01}, {0x10, 0x00},
	{0x11, 0x02}, {0x12, 0x00}, {0x13, 0x00}, {0x14, 0x04},
	{0x16, 0x38}, {0x17, 0x88}, {0x18, 0x00}, {0x19, 0x00},
	{0x1a, 0x00}, {0x1f, 0x1e},

	{0x0e, 0x01}, {0x20, 0x51}, {0x21, 0x00}, {0x22, 0xd0},
	{0x23, 0x02}, {0x24, 0xd0}, {0x25, 0x02}, {0x26, 0x11},
	{0x27, 0x00}, {0x28, 0x2e}, {0x29, 0x01}, {0x2a, 0x2e},	//ken //f5 //f6//fa
	{0x2b, 0x01}, {0x2c, 0x00}, {0x2d, 0x00}, {0x2e, 0x01},
	{0x2f, 0x00}, {0x30, 0x00}, {0x31, 0x01}, {0x32, 0x00},
	{0x33, 0x01}, {0x34, 0x00}, {0x35, 0x00}, {0x36, 0x00},
	{0x37, 0x00}, {0x38, 0x20}, {0x39, 0x40}, {0x3a, 0x02},
	{0x3b, 0x04}, {0x3c, 0x00}, {0x3d, 0x00}, {0x3e, 0x00},
	{0x3f, 0x00}, {0x40, 0x00}, {0x41, 0x00}, {0x42, 0x00},
	{0x43, 0x00}, {0x44, 0x00}, {0x45, 0x00}, {0x46, 0x00},
	{0x47, 0x00}, {0x48, 0x00}, {0x49, 0x00}, {0x4a, 0x00},
	{0x4b, 0x00}, {0x4c, 0x00}, {0x4d, 0x00}, {0x4e, 0x00},
	{0x4f, 0x00}, {0x50, 0x10}, {0x51, 0x00}, {0x52, 0x02},
	{0x53, 0x00}, {0x54, 0x3a}, {0x55, 0x03}, {0x56, 0x03},
	{0x57, 0x00}, {0x58, 0x06}, {0x59, 0x01}, {0x5a, 0xc2},
	{0x5b, 0x00}, {0x5c, 0x0a}, {0x5d, 0x03}, {0x5e, 0x06},
	{0x5f, 0x00}, {0x60, 0x05}, {0x61, 0x01}, {0x62, 0x8e},
	{0x63, 0x03}, {0x64, 0x07}, {0x65, 0x8e}, {0x66, 0x03},
	{0x67, 0x08}, {0x68, 0x01}, {0x69, 0x00}, {0x6a, 0x00},
	{0x6b, 0x00}, {0x6c, 0x00}, {0x6d, 0x00}, {0x6e, 0x00},
	{0x6f, 0x00}, {0x70, 0x00}, {0x71, 0x00}, {0x72, 0x00},
	{0x73, 0x00}, {0x74, 0x00}, {0x75, 0x00}, {0x76, 0x00},
	{0x77, 0x00}, {0x78, 0x00}, {0x79, 0x00}, {0x7a, 0x00},
	{0x7b, 0x00}, {0x7c, 0x00}, {0x7d, 0x00}, {0x7e, 0x00},
	{0x7f, 0x00},

	{0x0e, 0x02}, {0x20, 0x09}, {0x21, 0x00}, {0x22, 0x44},
	{0x23, 0x42}, {0x24, 0x00}, {0x25, 0x00}, {0x26, 0x00},
	{0x27, 0x00}, {0x28, 0x01}, {0x29, 0xef}, {0x2a, 0x47},
	{0x2b, 0x00}, {0x2c, 0x00}, {0x2d, 0x00}, {0x2e, 0x00},
	{0x2f, 0x00}, {0x30, 0x00}, {0x31, 0x00}, {0x32, 0x22},
	{0x33, 0x00}, {0x34, 0x60}, {0x35, 0x5a}, {0x36, 0x00},
	{0x37, 0x00}, {0x38, 0x00}, {0x39, 0x00}, {0x3a, 0x00},
	{0x3b, 0x00}, {0x3c, 0x00}, {0x3d, 0x00}, {0x3e, 0x00},
	{0x3f, 0x00}, {0x40, 0x00}, {0x41, 0x00}, {0x42, 0x00},
	{0x43, 0x00}, {0x44, 0x00}, {0x45, 0x00}, {0x46, 0x00},
	{0x47, 0x00}, {0x48, 0x60}, {0x49, 0x5a}, {0x4a, 0x00},
	{0x4b, 0x00}, {0x4c, 0x00}, {0x4d, 0x00}, {0x4e, 0x00},
	{0x4f, 0x00}, {0x50, 0x00}, {0x51, 0x0f}, {0x52, 0x00},
	{0x53, 0x00}, {0x54, 0x00}, {0x55, 0x00}, {0x56, 0x00},
	{0x57, 0x00}, {0x58, 0x00}, {0x59, 0x00}, {0x5a, 0x00},
	{0x5b, 0x00}, {0x5c, 0x00}, {0x5d, 0x00}, {0x5e, 0x00},
	{0x5f, 0x00}, {0x60, 0x00}, {0x61, 0xf8}, {0x62, 0x6e},
	{0x63, 0x25}, {0x64, 0xb5}, {0x65, 0xbd}, {0x66, 0x5f},
	{0x67, 0x00}, {0x68, 0x00}, {0x69, 0x00}, {0x6a, 0x00},
	{0x6b, 0x00}, {0x6c, 0x00}, {0x6d, 0x00}, {0x6e, 0x00},
	{0x6f, 0x00}, {0x70, 0x00}, {0x71, 0x00}, {0x72, 0x00},
	{0x73, 0x00}, {0x74, 0x00}, {0x75, 0x00}, {0x76, 0x00},
	{0x77, 0x00}, {0x78, 0x00}, {0x79, 0x00}, {0x7a, 0x00},
	{0x7b, 0x00}, {0x7c, 0x00}, {0x7d, 0x00}, {0x7e, 0x00},
	{0x7f, 0x00},

	{0x0e, 0x03}, {0x20, 0x10}, {0x21, 0x05}, {0x22, 0x88},
	{0x23, 0x00}, {0x24, 0x18}, {0x25, 0x01}, {0x26, 0x09},
	{0x27, 0x05}, {0x28, 0x26}, {0x29, 0x03}, {0x2a, 0x06},
	{0x2b, 0x00}, {0x2c, 0x23}, {0x2d, 0x00}, {0x2e, 0x23},
	{0x2f, 0x03}, {0x30, 0xdd}, {0x31, 0x00}, {0x32, 0xe9},
	{0x33, 0x04}, {0x34, 0x23}, {0x35, 0x00}, {0x36, 0x23},
	{0x37, 0x03}, {0x38, 0x00}, {0x39, 0x00}, {0x3a, 0x00},
	{0x3b, 0x00}, {0x3c, 0x00}, {0x3d, 0x00}, {0x3e, 0x00},
	{0x3f, 0x00}, {0x40, 0xd0}, {0x41, 0x02}, {0x42, 0x20},
	{0x43, 0x03}, {0x44, 0x42}, {0x45, 0x02}, {0x46, 0x58},
	{0x47, 0x02}, {0x48, 0x03}, {0x49, 0x00}, {0x4a, 0x80},
	{0x4b, 0x16}, {0x4c, 0x15}, {0x4d, 0x18}, {0x4e, 0x00},
	{0x4f, 0x00}, {0x50, 0x00}, {0x51, 0x00}, {0x52, 0x00},
	{0x53, 0x00}, {0x54, 0x00}, {0x55, 0x00}, {0x56, 0x00},
	{0x57, 0x00}, {0x58, 0x20}, {0x59, 0x01}, {0x5a, 0xff},
	{0x5b, 0xdd}, {0x5c, 0x00}, {0x5d, 0x00}, {0x5e, 0x00},
	{0x5f, 0x00}, {0x60, 0x00}, {0x61, 0x00}, {0x62, 0xff},
	{0x63, 0xff}, {0x64, 0xff}, {0x65, 0xff}, {0x66, 0xff},
	{0x67, 0x00}, {0x68, 0x00}, {0x69, 0x00}, {0x6a, 0xff},
	{0x6b, 0x00}, {0x6c, 0xff}, {0x6d, 0xff}, {0x6e, 0xff},
	{0x6f, 0x00}, {0x70, 0x00}, {0x71, 0xff}, {0x72, 0xff},
	{0x73, 0x00}, {0x74, 0x00}, {0x75, 0x00}, {0x76, 0xff},
	{0x77, 0xff}, {0x78, 0x1a}, {0x79, 0x10}, {0x7a, 0x00},
	{0x7b, 0x00}, {0x7c, 0x00}, {0x7d, 0x00}, {0x7e, 0x00},
	{0x7f, 0x00}, {0x80, 0x25}, {0x81, 0x00}, {0x82, 0x00},
	{0x83, 0x00}, {0x84, 0x02}, {0x85, 0x00}, {0x86, 0x00},
	{0x87, 0x00}, {0x88, 0x16}, {0x89, 0x00}, {0x8a, 0x10},
	{0x8b, 0x00}, {0x8c, 0x00}, {0x8d, 0x00}, {0x8e, 0x00},
	{0x8f, 0x00}, {0x90, 0x3d}, {0x91, 0x47}, {0x92, 0x00},
	{0x93, 0x18}, {0x94, 0x07}, {0x95, 0x00}, {0x96, 0x07},
	{0x97, 0x00}, {0x98, 0x0e}, {0x99, 0x00}, {0x9a, 0x0e},
	{0x9b, 0x00}, {0x9c, 0x1c}, {0x9d, 0x09}, {0x9e, 0x00},
	{0x9f, 0x00}, {0xa0, 0x00}, {0xa1, 0x00}, {0xa2, 0x00},
	{0xa3, 0xac}, {0xa4, 0x07}, {0xa5, 0x00}, {0xa6, 0x07},
	{0xa7, 0x00}, {0xa8, 0x0e}, {0xa9, 0x00}, {0xaa, 0x0e},
	{0xab, 0x00}, {0xac, 0x04}, {0xad, 0x02}, {0xae, 0x01},
	{0xaf, 0x01}, {0xb0, 0x00}, {0xb1, 0x00}, {0xb2, 0x00},
	{0xb3, 0x00}, {0xb4, 0x00}, {0xb5, 0x00}, {0xb6, 0x00},
	{0xb7, 0x00}, {0xb8, 0x00}, {0xb9, 0x00}, {0xba, 0x00},
	{0xbb, 0x00}, {0xbc, 0x00}, {0xbd, 0x00}, {0xbe, 0x00},
	{0xbf, 0x00}, {0xc0, 0x00}, {0xc1, 0x00}, {0xc2, 0x00},
	{0xc3, 0x00}, {0xc4, 0x00}, {0xc5, 0x40}, {0xc6, 0x0F},	//10
	{0xc7, 0x00}, {0xc8, 0x01}, {0xc9, 0x00}, {0xca, 0x30},
	{0xcb, 0x00}, {0xcc, 0x00}, {0xcd, 0x00}, {0xce, 0xf0},
	{0xcf, 0x01}, {0xd0, 0x00}, {0xd1, 0x00}, {0xd2, 0x00},
	{0xd3, 0x00}, {0xd4, 0x00}, {0xd5, 0x00}, {0xd6, 0x00},
	{0xd7, 0x00}, {0xd8, 0x00}, {0xd9, 0x00}, {0xda, 0x00},
	{0xdb, 0x00}, {0xdc, 0x00}, {0xdd, 0x00}, {0xde, 0x00},
	{0xdf, 0x00}, {0xe0, 0x00}, {0xe1, 0x00}, {0xe2, 0x00},
	{0xe3, 0x00}, {0xe4, 0x00}, {0xe5, 0x00}, {0xe6, 0x00},
	{0xe7, 0x00}, {0xe8, 0x00}, {0xe9, 0x00}, {0xea, 0x00},
	{0xeb, 0x00}, {0xec, 0x00}, {0xed, 0x00}, {0xee, 0x00},
	{0xef, 0x00}, {0xf0, 0x00}, {0xf1, 0x00}, {0xf2, 0x00},
	{0xf3, 0x00}, {0xf4, 0x00}, {0xf5, 0x00}, {0xf6, 0x00},
	{0xf7, 0x00}, {0xf8, 0x00}, {0xf9, 0x00}, {0xfa, 0x00},
	{0xfb, 0x00}, {0xfc, 0x00}, {0xfd, 0x00}, {0xfe, 0x00},
	{0xff, 0x00},

	//initial issue      
	{0x0e, 0x00}, {0x1b, 0x00}, {0x1c, 0xe1}, {0x1d, 0x14},
	{0x1e, 0xaf}, {0x15, 0x00},
};

unsigned char (*TV_init)[2];
unsigned char (*TV)[2];
unsigned char (*AnalogLCD)[2];
unsigned char (*VGA)[2];

extern int GM_i2c_xfer(struct i2c_msg *msgs, int num, int clockdiv);
#ifdef CONFIG_TV_Encoder
static void tv_init1(void)
{
	int i;
	struct i2c_msg msgs[1];
	//printk("0x%x ",saa7137); 
	for (i = 0; i < 8; i++) {
		msgs[0].addr = 0x88 >> 1;
		msgs[0].flags = 0;
		msgs[0].len = 2;
		msgs[0].buf = &TV_init[i][0];

		if (GM_i2c_xfer(msgs, 1, 50) != 1)
			return;
	}
}

static void tv_init2(void)
{
	int i;
	struct i2c_msg msgs[1];
	//printk("0x%x ",saa7137);
	for (i = 0; i < 21; i++) {
		msgs[0].addr = 0x88 >> 1;
		msgs[0].flags = 0;
		msgs[0].len = 2;
		msgs[0].buf = &TV[i][0];

		if (GM_i2c_xfer(msgs, 1, 50) != 1)
			return;
	}
}

static void tv_CS4954(void)
{
	int i;
	struct i2c_msg msgs[1];
	//printk("0x%x ",saa7137); 
	for (i = 0; i < 11; i++) {
		msgs[0].addr = 0x00 >> 1;
		msgs[0].flags = 0;
		msgs[0].len = 2;
		msgs[0].buf = &TV_init[i][0];
		//printk("0x00 0x%x 0x%x\n",TV_init[i][0],TV_init[i][1]);

		if (GM_i2c_xfer(msgs, 1, 50) != 1)
			return;
	}
}
#endif

#ifdef CONFIG_ANALOG_LCD
static void AnalogLCD_init(void)
{
	int i;
	struct i2c_msg msgs[1];
	//printk("0x%x ",saa7137);
	for (i = 0; i < 208; i++) {
		msgs[0].addr = 0x78 >> 1;
		msgs[0].flags = 0;
		msgs[0].len = 2;
		msgs[0].buf = &AnalogLCD[i];

		if (GM_i2c_xfer(msgs, 1, 50) != 1)
			return;
	}
}
#endif

#ifdef CONFIG_VGA
static void VGA_init(void)
{
	int i;
	struct i2c_msg msgs[1];
	//printk("0x%x ",saa7137);
	for (i = 0; i < 451; i++) {
		msgs[0].addr = 0x60 >> 1;
		msgs[0].flags = 0;
		msgs[0].len = 2;
		msgs[0].buf = &VGA[i];

		if (GM_i2c_xfer(msgs, 1, 50) != 1)
			return;
	}
}
#endif
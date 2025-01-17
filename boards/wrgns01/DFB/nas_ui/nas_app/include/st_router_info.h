#ifndef _ST_ROUTER_INFO_H_
#define _ST_ROUTER_INFO_H_

#include "nas_str_define.h"

//static int PageSegmentNums=5;
//PageSegment
enum {
	ROUTER_INFO,
	INTERNET_STATUS,
	WIRED_STATUS,
	WIRELESS_STATUS,
	HD_SPACE
};

#define INFO_MAX_LIST 32
#define INFO_MAX_VALUE_SIZE 128
typedef struct _InfoValue{
	unsigned char *title;
	unsigned char *name[INFO_MAX_LIST];
	unsigned char value[INFO_MAX_LIST][INFO_MAX_VALUE_SIZE];
	int max;
}InfoValue;

/*STATUS TITLE*/
//_GeneralSeq
enum {
	GENERAL_TITLE,
	GENERAL_MODELNAME,
	GENERAL_HWVERSION,
	GENERAL_FWVERSION,
	GENERAL_UPTIME,
	GENERAL_NEWFW
};

static char *gGeneralTitle[] =
{
	ROUTERINFO_GENERAL_INFO,
	ROUTERINFO_GENERAL_MODELNAME,
	ROUTERINFO_GENERAL_HWVERSION,
	ROUTERINFO_GENERAL_FWVERSION,
	ROUTERINFO_GENERAL_UPTIME,
	ROUTERINFO_GENERAL_NEWFW
};

//_InternetSeq
enum {
	INTERNET_TITLE,
	INTERNET_CONNECT_TYPE,
	INTERNET_CABLE_STATUS,
	INTERNET_NETWORK_STATUS,
	INTERNET_CONNECT_TIME,
	INTERNET_MACADDR,
	INTERNET_IPADDR,
	INTERNET_NETMASK,
	INTERNET_GATEWAY
};

static char *gInternetTitle[] =
{
	ROUTERINFO_INTERNET_STATUS,
	ROUTERINFO_INTERNET_CONNECT_TYPE,
	ROUTERINFO_INTERNET_CABLE_STATUS,
	ROUTERINFO_INTERNET_NETWORK_STATUS,
	ROUTERINFO_INTERNET_CONNECT_TIME,
	ROUTERINFO_INTERNET_MACADDR,
	ROUTERINFO_INTERNET_IPADDR,
	ROUTERINFO_INTERNET_NETMASK,
	ROUTERINFO_INTERNET_GATEWAY	
};

//_WiredSeq
enum {
	WIRED_TITLE,
	WIRED_MACADDR,
	WIRED_IPADDR,
	WIRED_NETMASK,
	WIRED_DHCP_SERVER,
	WIRED_CABLE_P1,
	WIRED_CABLE_P2,
	WIRED_CABLE_P3,
	WIRED_CABLE_P4
};
static char *gWiredTitle[] =
{
	ROUTERINFO_WIRED_STATUS,
	ROUTERINFO_WIRED_MACADDR,
	ROUTERINFO_WIRED_IPADDR,
	ROUTERINFO_WIRED_NETMASK,
	ROUTERINFO_WIRED_DHCP_SERVER,
	ROUTERINFO_WIRED_CABLE_P1,
	ROUTERINFO_WIRED_CABLE_P2,
	ROUTERINFO_WIRED_CABLE_P3,
	ROUTERINFO_WIRED_CABLE_P4
};

//_WirelessSeq
enum {
	WIRELESS_TITLE,
	WIRELESS_RADIO,
	WIRELESS_CHANNEL,
	WIRELESS_HOST_SSID,
	WIRELESS_GUEST_SSID,
	WIRELESS_MODE,
	WIRELESS_HOST_SECURITY,
	WIRELESS_GUEST_SECURITY,
	WIRELESS_MACADDR,
	WIRELESS_WPS_NUMBER
};
static char *gWirelessTitle[] =
{
	ROUTERINFO_WIRELESS_STATUS,
	ROUTERINFO_WIRELESS_RADIO,
	ROUTERINFO_WIRELESS_CHANNEL,
	ROUTERINFO_WIRELESS_HOST_SSID,
	ROUTERINFO_WIRELESS_GUEST_SSID,
	ROUTERINFO_WIRELESS_MODE,
	ROUTERINFO_WIRELESS_HOST_SECURITY,
	ROUTERINFO_WIRELESS_GUEST_SECURITY,
	ROUTERINFO_WIRELESS_MACADDR,
	ROUTERINFO_WIRELESS_WPS_NUMBER
};

//_HDSeq
enum {
	HD_TITLE,
	HD_NAME,
	HD_FORMAT,
	HD_CAPACITY,
	HD_USED,
	HD_AVAILABLE,
	HD_HEALTH,
	HD_TEMP
};
static char *gHDTitle[] =
{
	ROUTERINFO_HD_SPACE,
	ROUTERINFO_HD_NAME,
	ROUTERINFO_HD_FORMAT,
	ROUTERINFO_HD_CAPACITY,
	ROUTERINFO_HD_USED,
	ROUTERINFO_HD_AVAILABLE,
	ROUTERINFO_HD_HEALTH,
	ROUTERINFO_HD_TEMP
};

#endif /* _ST_ROUTER_INFO_H_ */

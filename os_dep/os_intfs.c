/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#define _OS_INTFS_C_

#include <drv_conf.h>

#include <osdep_service.h>
#include <drv_types.h>
#include <xmit_osdep.h>
#include <recv_osdep.h>
#include <hal_intf.h>
#include <rtw_ioctl.h>
#include <rtw_version.h>

#include <usb_osintf.h>

#ifdef CONFIG_BR_EXT
#include <rtw_br_ext.h>
#endif //CONFIG_BR_EXT

#ifdef CONFIG_RF_GAIN_OFFSET
#define		REG_RF_BB_GAIN_OFFSET	0x55
#define		RF_GAIN_OFFSET_MASK		0xfffff
#endif //CONFIG_RF_GAIN_OFFSET

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek Wireless Lan Driver");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_VERSION(DRIVERVERSION);

/* module param defaults */
static int rtw_chip_version = 0x00;
static int rtw_rfintfs = HWPI;
static int rtw_lbkmode = 0;//RTL8712_AIR_TRX;
static int rtw_debug = 1;

static int rtw_network_mode = Ndis802_11IBSS;//Ndis802_11Infrastructure;//infra, ad-hoc, auto
//NDIS_802_11_SSID	ssid;
static int rtw_channel = 1;//ad-hoc support requirement
static int rtw_wireless_mode = WIRELESS_11BG_24N;
static int rtw_vrtl_carrier_sense = AUTO_VCS;
static int rtw_vcs_type = RTS_CTS;//*
static int rtw_rts_thresh = 2347;//*
static int rtw_frag_thresh = 2346;//*
static int rtw_preamble = PREAMBLE_LONG;//long, short, auto
static int rtw_scan_mode = 1;//active, passive
static int rtw_adhoc_tx_pwr = 1;
static int rtw_soft_ap;
//int smart_ps = 1;
#ifdef CONFIG_POWER_SAVING
static int rtw_power_mgnt = 1;
#ifdef CONFIG_IPS_LEVEL_2
static int rtw_ips_mode = IPS_LEVEL_2;
#else
static int rtw_ips_mode = IPS_NORMAL;
#endif
#else
static int rtw_power_mgnt = PS_MODE_ACTIVE;
static int rtw_ips_mode = IPS_NONE;
#endif

static int rtw_smart_ps = 2;

#ifdef CONFIG_TX_EARLY_MODE
static int rtw_early_mode=1;
#endif
module_param(rtw_ips_mode, int, 0644);
MODULE_PARM_DESC(rtw_ips_mode,"The default IPS mode");

static int rtw_radio_enable = 1;
static int rtw_long_retry_lmt = 7;
static int rtw_short_retry_lmt = 7;
static int rtw_busy_thresh = 40;
//int qos_enable = 0; //*
static int rtw_ack_policy = NORMAL_ACK;

static int rtw_software_encrypt;
static int rtw_software_decrypt;

static int rtw_acm_method;// 0:By SW 1:By HW.

static int rtw_wmm_enable = 1;// default is set to enable the wmm.
static int rtw_uapsd_enable;
static int rtw_uapsd_max_sp = NO_LIMIT;
static int rtw_uapsd_acbk_en;
static int rtw_uapsd_acbe_en;
static int rtw_uapsd_acvi_en;
static int rtw_uapsd_acvo_en;

#ifdef CONFIG_80211N_HT
int rtw_ht_enable = 1;
int rtw_cbw40_enable = 3; // 0 :diable, bit(0): enable 2.4g, bit(1): enable 5g
int rtw_ampdu_enable = 1;//for enable tx_ampdu
static int rtw_rx_stbc = 1;// 0: disable, bit(0):enable 2.4g, bit(1):enable 5g, default is set to enable 2.4GHZ for IOT issue with bufflao's AP at 5GHZ
static int rtw_ampdu_amsdu;// 0: disabled, 1:enabled, 2:auto
#endif

static int rtw_lowrate_two_xmit = 1;//Use 2 path Tx to transmit MCS0~7 and legacy mode

//int rf_config = RF_1T2R;  // 1T2R
static int rtw_rf_config = RF_819X_MAX_TYPE;  //auto
static int rtw_low_power;
#ifdef CONFIG_WIFI_TEST
static int rtw_wifi_spec = 1;//for wifi test
#else
static int rtw_wifi_spec;
#endif
static int rtw_channel_plan = RT_CHANNEL_DOMAIN_MAX;

#ifdef CONFIG_BT_COEXIST
static int rtw_btcoex_enable = 1;
static int rtw_bt_iso = 2;// 0:Low, 1:High, 2:From Efuse
static int rtw_bt_sco = 3;// 0:Idle, 1:None-SCO, 2:SCO, 3:From Counter, 4.Busy, 5.OtherBusy
static int rtw_bt_ampdu =1 ;// 0:Disable BT control A-MPDU, 1:Enable BT control A-MPDU.
#endif

static int rtw_AcceptAddbaReq = true;// 0:Reject AP's Add BA req, 1:Accept AP's Add BA req.

static int rtw_antdiv_cfg = 2; // 0:OFF , 1:ON, 2:decide by Efuse config
static int rtw_antdiv_type = 0 ; //0:decide by efuse  1: for 88EE, 1Tx and 1RxCG are diversity.(2 Ant with SPDT), 2:  for 88EE, 1Tx and 2Rx are diversity.( 2 Ant, Tx and RxCG are both on aux port, RxCS is on main port ), 3: for 88EE, 1Tx and 1RxCG are fixed.(1Ant, Tx and RxCG are both on aux port)


#ifdef CONFIG_USB_AUTOSUSPEND
static int rtw_enusbss = 1;//0:disable,1:enable
#else
static int rtw_enusbss;//0:disable,1:enable
#endif

static int rtw_hwpdn_mode=2;//0:disable,1:enable,2: by EFUSE config

#ifdef CONFIG_HW_PWRP_DETECTION
static int rtw_hwpwrp_detect = 1;
#else
static int rtw_hwpwrp_detect; //HW power  ping detect 0:disable , 1:enable
#endif

static int rtw_hw_wps_pbc = 1;

#ifdef CONFIG_80211D
static int rtw_80211d = 0;
#endif

#ifdef CONFIG_REGULATORY_CTRL
static int rtw_regulatory_id =2;
#else
static int rtw_regulatory_id = 0xff;// Regulatory tab id, 0xff = follow efuse's setting
#endif
module_param(rtw_regulatory_id, int, 0644);


static char *ifname = "wlan%d";
module_param(ifname, charp, 0644);
MODULE_PARM_DESC(ifname, "The default name to allocate for first interface");

static char *if2name = "wlan%d";
module_param(if2name, charp, 0644);
MODULE_PARM_DESC(if2name, "The default name to allocate for second interface");

char *rtw_initmac = NULL;  // temp mac address if users want to use instead of the mac address in Efuse

module_param(rtw_initmac, charp, 0644);
module_param(rtw_channel_plan, int, 0644);
module_param(rtw_chip_version, int, 0644);
module_param(rtw_rfintfs, int, 0644);
module_param(rtw_lbkmode, int, 0644);
module_param(rtw_network_mode, int, 0644);
module_param(rtw_channel, int, 0644);
module_param(rtw_wmm_enable, int, 0644);
module_param(rtw_vrtl_carrier_sense, int, 0644);
module_param(rtw_vcs_type, int, 0644);
module_param(rtw_busy_thresh, int, 0644);
#ifdef CONFIG_80211N_HT
module_param(rtw_ht_enable, int, 0644);
module_param(rtw_cbw40_enable, int, 0644);
module_param(rtw_ampdu_enable, int, 0644);
module_param(rtw_rx_stbc, int, 0644);
module_param(rtw_ampdu_amsdu, int, 0644);
#endif

module_param(rtw_lowrate_two_xmit, int, 0644);

module_param(rtw_rf_config, int, 0644);
module_param(rtw_power_mgnt, int, 0644);
module_param(rtw_smart_ps, int, 0644);
module_param(rtw_low_power, int, 0644);
module_param(rtw_wifi_spec, int, 0644);

module_param(rtw_antdiv_cfg, int, 0644);
module_param(rtw_antdiv_type, int, 0644);

module_param(rtw_enusbss, int, 0644);
module_param(rtw_hwpdn_mode, int, 0644);
module_param(rtw_hwpwrp_detect, int, 0644);

module_param(rtw_hw_wps_pbc, int, 0644);

#ifdef CONFIG_TX_EARLY_MODE
module_param(rtw_early_mode, int, 0644);
#endif
#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
char *rtw_adaptor_info_caching_file_path= "/data/misc/wifi/rtw_cache";
module_param(rtw_adaptor_info_caching_file_path, charp, 0644);
MODULE_PARM_DESC(rtw_adaptor_info_caching_file_path, "The path of adapter info cache file");
#endif //CONFIG_ADAPTOR_INFO_CACHING_FILE

static uint rtw_max_roaming_times=2;
module_param(rtw_max_roaming_times, uint, 0644);
MODULE_PARM_DESC(rtw_max_roaming_times,"The max roaming times to try");

#ifdef CONFIG_IOL
int rtw_fw_iol=1;// 0:Disable, 1:enable, 2:by usb speed
module_param(rtw_fw_iol, int, 0644);
MODULE_PARM_DESC(rtw_fw_iol,"FW IOL");
#endif //CONFIG_IOL

#ifdef CONFIG_FILE_FWIMG
char *rtw_fw_file_path= "";
module_param(rtw_fw_file_path, charp, 0644);
MODULE_PARM_DESC(rtw_fw_file_path, "The path of fw image");
#endif //CONFIG_FILE_FWIMG

#ifdef CONFIG_80211D
module_param(rtw_80211d, int, 0644);
MODULE_PARM_DESC(rtw_80211d, "Enable 802.11d mechanism");
#endif

#ifdef CONFIG_BT_COEXIST
module_param(rtw_btcoex_enable, int, 0644);
MODULE_PARM_DESC(rtw_btcoex_enable, "Enable BT co-existence mechanism");
#endif

static uint rtw_notch_filter = RTW_NOTCH_FILTER;
module_param(rtw_notch_filter, uint, 0644);
MODULE_PARM_DESC(rtw_notch_filter, "0:Disable, 1:Enable, 2:Enable only for P2P");
module_param_named(debug, rtw_debug, int, 0444);
MODULE_PARM_DESC(debug, "Set debug level (1-9) (default 1)");

#ifdef CONFIG_PROC_DEBUG
#define RTL8192C_PROC_NAME "rtl819xC"
#define RTL8192D_PROC_NAME "rtl819xD"
static char rtw_proc_name[IFNAMSIZ];
static struct proc_dir_entry *rtw_proc = NULL;
static int	rtw_proc_cnt = 0;

#define RTW_PROC_NAME DRV_NAME

static int netdev_close(struct net_device *pnetdev);

#ifndef create_proc_entry
/* dummy routines */
void rtw_proc_remove_one(struct net_device *dev)
{
}

void rtw_proc_init_one(struct net_device *dev)
{
}

#else	/* create_proc_entry not defined */
void rtw_proc_init_one(struct net_device *dev)
{
	struct proc_dir_entry *dir_dev = NULL;
	struct proc_dir_entry *entry=NULL;
	struct rtw_adapter *padapter = netdev_priv(dev);
	u8 rf_type;

	if(rtw_proc == NULL)
	{
		if(padapter->chip_type == RTL8188C_8192C)
		{
			memcpy(rtw_proc_name, RTL8192C_PROC_NAME, sizeof(RTL8192C_PROC_NAME));
		}
		else if(padapter->chip_type == RTL8192D)
		{
			memcpy(rtw_proc_name, RTL8192D_PROC_NAME, sizeof(RTL8192D_PROC_NAME));
		}
		else if(padapter->chip_type == RTL8723A)
		{
			memcpy(rtw_proc_name, RTW_PROC_NAME, sizeof(RTW_PROC_NAME));
		}
		else if(padapter->chip_type == RTL8188E)
		{
			memcpy(rtw_proc_name, RTW_PROC_NAME, sizeof(RTW_PROC_NAME));
		}
		else
		{
			memcpy(rtw_proc_name, RTW_PROC_NAME, sizeof(RTW_PROC_NAME));
		}

		rtw_proc=create_proc_entry(rtw_proc_name, S_IFDIR, init_net.proc_net);
		if (rtw_proc == NULL) {
			DBG_8723A(KERN_ERR "Unable to create rtw_proc directory\n");
			return;
		}

		entry = create_proc_read_entry("ver_info", S_IFREG | S_IRUGO, rtw_proc, proc_get_drv_version, dev);
		if (!entry) {
			DBG_8723A("Unable to create_proc_read_entry!\n");
			return;
		}
	}



	if(padapter->dir_dev == NULL)
	{
		padapter->dir_dev = create_proc_entry(dev->name,
					  S_IFDIR | S_IRUGO | S_IXUGO,
					  rtw_proc);

		dir_dev = padapter->dir_dev;

		if(dir_dev==NULL)
		{
			if(rtw_proc_cnt == 0)
			{
				if(rtw_proc){
					remove_proc_entry(rtw_proc_name,
							  init_net.proc_net);
					rtw_proc = NULL;
				}
			}

			DBG_8723A("Unable to create dir_dev directory\n");
			return;
		}
	}
	else
	{
		return;
	}

	rtw_proc_cnt++;

	entry = create_proc_read_entry("write_reg", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_write_reg, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_write_reg;

	entry = create_proc_read_entry("read_reg", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_read_reg, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_read_reg;


	entry = create_proc_read_entry("fwstate", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_fwstate, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}


	entry = create_proc_read_entry("sec_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_sec_info, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}


	entry = create_proc_read_entry("mlmext_state", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_mlmext_state, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}


	entry = create_proc_read_entry("qos_option", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_qos_option, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("ht_option", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ht_option, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("rf_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rf_info, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("ap_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ap_info, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("adapter_state", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_adapter_state, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("trx_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_trx_info, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("mac_reg_dump1", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_mac_reg_dump1, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("mac_reg_dump2", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_mac_reg_dump2, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("mac_reg_dump3", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_mac_reg_dump3, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("bb_reg_dump1", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_bb_reg_dump1, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("bb_reg_dump2", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_bb_reg_dump2, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("bb_reg_dump3", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_bb_reg_dump3, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("rf_reg_dump1", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rf_reg_dump1, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	entry = create_proc_read_entry("rf_reg_dump2", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rf_reg_dump2, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}

	rtw_hal_get_hwreg(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));
	if((RF_1T2R == rf_type) ||(RF_1T1R ==rf_type ))	{
		entry = create_proc_read_entry("rf_reg_dump3", S_IFREG | S_IRUGO,
					   dir_dev, proc_get_rf_reg_dump3, dev);
		if (!entry) {
			DBG_8723A("Unable to create_proc_read_entry!\n");
			return;
		}

		entry = create_proc_read_entry("rf_reg_dump4", S_IFREG | S_IRUGO,
					   dir_dev, proc_get_rf_reg_dump4, dev);
		if (!entry) {
			DBG_8723A("Unable to create_proc_read_entry!\n");
			return;
		}
	}

#ifdef CONFIG_AP_MODE

	entry = create_proc_read_entry("all_sta_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_all_sta_info, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
#endif

#ifdef CONFIG_FIND_BEST_CHANNEL
	entry = create_proc_read_entry("best_channel", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_best_channel, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
#endif

	entry = create_proc_read_entry("rx_signal", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rx_signal, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_rx_signal;
#ifdef CONFIG_80211N_HT
	entry = create_proc_read_entry("ht_enable", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ht_enable, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_ht_enable;

	entry = create_proc_read_entry("cbw40_enable", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_cbw40_enable, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_cbw40_enable;

	entry = create_proc_read_entry("ampdu_enable", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ampdu_enable, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_ampdu_enable;

	entry = create_proc_read_entry("rx_stbc", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rx_stbc, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_rx_stbc;
#endif //CONFIG_80211N_HT

	entry = create_proc_read_entry("path_rssi", S_IFREG | S_IRUGO,
					dir_dev, proc_get_two_path_rssi, dev);


	entry = create_proc_read_entry("rssi_disp", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rssi_disp, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_rssi_disp;
#ifdef CONFIG_BT_COEXIST
	entry = create_proc_read_entry("btcoex_dbg", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_btcoex_dbg, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_btcoex_dbg;
#endif /*CONFIG_BT_COEXIST*/

#if defined(DBG_CONFIG_ERROR_DETECT)
	entry = create_proc_read_entry("sreset", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_sreset, dev);
	if (!entry) {
		DBG_8723A("Unable to create_proc_read_entry!\n");
		return;
	}
	entry->write_proc = proc_set_sreset;
#endif /* DBG_CONFIG_ERROR_DETECT */

}

void rtw_proc_remove_one(struct net_device *dev)
{
	struct proc_dir_entry *dir_dev = NULL;
	struct rtw_adapter *padapter = netdev_priv(dev);
	u8 rf_type;

	dir_dev = padapter->dir_dev;
	padapter->dir_dev = NULL;

	if (dir_dev) {

		remove_proc_entry("write_reg", dir_dev);
		remove_proc_entry("read_reg", dir_dev);
		remove_proc_entry("fwstate", dir_dev);
		remove_proc_entry("sec_info", dir_dev);
		remove_proc_entry("mlmext_state", dir_dev);
		remove_proc_entry("qos_option", dir_dev);
		remove_proc_entry("ht_option", dir_dev);
		remove_proc_entry("rf_info", dir_dev);
		remove_proc_entry("ap_info", dir_dev);
		remove_proc_entry("adapter_state", dir_dev);
		remove_proc_entry("trx_info", dir_dev);

		remove_proc_entry("mac_reg_dump1", dir_dev);
		remove_proc_entry("mac_reg_dump2", dir_dev);
		remove_proc_entry("mac_reg_dump3", dir_dev);
		remove_proc_entry("bb_reg_dump1", dir_dev);
		remove_proc_entry("bb_reg_dump2", dir_dev);
		remove_proc_entry("bb_reg_dump3", dir_dev);
		remove_proc_entry("rf_reg_dump1", dir_dev);
		remove_proc_entry("rf_reg_dump2", dir_dev);
		rtw_hal_get_hwreg(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));
		if((RF_1T2R == rf_type) ||(RF_1T1R ==rf_type ))	{
			remove_proc_entry("rf_reg_dump3", dir_dev);
			remove_proc_entry("rf_reg_dump4", dir_dev);
		}
#ifdef CONFIG_AP_MODE
		remove_proc_entry("all_sta_info", dir_dev);
#endif

#ifdef CONFIG_FIND_BEST_CHANNEL
		remove_proc_entry("best_channel", dir_dev);
#endif
		remove_proc_entry("rx_signal", dir_dev);
#ifdef CONFIG_80211N_HT
		remove_proc_entry("cbw40_enable", dir_dev);

		remove_proc_entry("ht_enable", dir_dev);

		remove_proc_entry("ampdu_enable", dir_dev);

		remove_proc_entry("rx_stbc", dir_dev);
#endif //CONFIG_80211N_HT
		remove_proc_entry("path_rssi", dir_dev);

		remove_proc_entry("rssi_disp", dir_dev);

#ifdef CONFIG_BT_COEXIST
		remove_proc_entry("btcoex_dbg", dir_dev);
#endif //CONFIG_BT_COEXIST

#if defined(DBG_CONFIG_ERROR_DETECT)
	remove_proc_entry("sreset", dir_dev);
#endif /* DBG_CONFIG_ERROR_DETECT */

		remove_proc_entry(dev->name, rtw_proc);
		dir_dev = NULL;

	}
	else
	{
		return;
	}

	rtw_proc_cnt--;

	if(rtw_proc_cnt == 0)
	{
		if(rtw_proc){
			remove_proc_entry("ver_info", rtw_proc);

			remove_proc_entry(rtw_proc_name, init_net.proc_net);
			rtw_proc = NULL;
		}
	}
}
#endif
#endif //ifndef create_proc_entry

static uint loadparam(struct rtw_adapter *padapter,  struct net_device *pnetdev)
{

	uint status = _SUCCESS;
	struct registry_priv  *registry_par = &padapter->registrypriv;

_func_enter_;

	GlobalDebugLevel = rtw_debug;
	registry_par->chip_version = (u8)rtw_chip_version;
	registry_par->rfintfs = (u8)rtw_rfintfs;
	registry_par->lbkmode = (u8)rtw_lbkmode;
	//registry_par->hci = (u8)hci;
	registry_par->network_mode  = (u8)rtw_network_mode;

	memcpy(registry_par->ssid.Ssid, "ANY", 3);
	registry_par->ssid.SsidLength = 3;

	registry_par->channel = (u8)rtw_channel;
	registry_par->wireless_mode = (u8)rtw_wireless_mode;
	registry_par->vrtl_carrier_sense = (u8)rtw_vrtl_carrier_sense ;
	registry_par->vcs_type = (u8)rtw_vcs_type;
	registry_par->rts_thresh=(u16)rtw_rts_thresh;
	registry_par->frag_thresh=(u16)rtw_frag_thresh;
	registry_par->preamble = (u8)rtw_preamble;
	registry_par->scan_mode = (u8)rtw_scan_mode;
	registry_par->adhoc_tx_pwr = (u8)rtw_adhoc_tx_pwr;
	registry_par->soft_ap=  (u8)rtw_soft_ap;
	registry_par->smart_ps =  (u8)rtw_smart_ps;
	registry_par->power_mgnt = (u8)rtw_power_mgnt;
	registry_par->ips_mode = (u8)rtw_ips_mode;
	registry_par->radio_enable = (u8)rtw_radio_enable;
	registry_par->long_retry_lmt = (u8)rtw_long_retry_lmt;
	registry_par->short_retry_lmt = (u8)rtw_short_retry_lmt;
	registry_par->busy_thresh = (u16)rtw_busy_thresh;
	//registry_par->qos_enable = (u8)rtw_qos_enable;
	registry_par->ack_policy = (u8)rtw_ack_policy;
	registry_par->software_encrypt = (u8)rtw_software_encrypt;
	registry_par->software_decrypt = (u8)rtw_software_decrypt;

	registry_par->acm_method = (u8)rtw_acm_method;

	 //UAPSD
	registry_par->wmm_enable = (u8)rtw_wmm_enable;
	registry_par->uapsd_enable = (u8)rtw_uapsd_enable;
	registry_par->uapsd_max_sp = (u8)rtw_uapsd_max_sp;
	registry_par->uapsd_acbk_en = (u8)rtw_uapsd_acbk_en;
	registry_par->uapsd_acbe_en = (u8)rtw_uapsd_acbe_en;
	registry_par->uapsd_acvi_en = (u8)rtw_uapsd_acvi_en;
	registry_par->uapsd_acvo_en = (u8)rtw_uapsd_acvo_en;

#ifdef CONFIG_80211N_HT
	registry_par->ht_enable = (u8)rtw_ht_enable;
	registry_par->cbw40_enable = (u8)rtw_cbw40_enable;
	registry_par->ampdu_enable = (u8)rtw_ampdu_enable;
	registry_par->rx_stbc = (u8)rtw_rx_stbc;
	registry_par->ampdu_amsdu = (u8)rtw_ampdu_amsdu;
#endif
#ifdef CONFIG_TX_EARLY_MODE
	registry_par->early_mode = (u8)rtw_early_mode;
#endif
	registry_par->lowrate_two_xmit = (u8)rtw_lowrate_two_xmit;
	registry_par->rf_config = (u8)rtw_rf_config;
	registry_par->low_power = (u8)rtw_low_power;


	registry_par->wifi_spec = (u8)rtw_wifi_spec;

	registry_par->channel_plan = (u8)rtw_channel_plan;

#ifdef CONFIG_BT_COEXIST
	registry_par->btcoex = (u8)rtw_btcoex_enable;
	registry_par->bt_iso = (u8)rtw_bt_iso;
	registry_par->bt_sco = (u8)rtw_bt_sco;
	registry_par->bt_ampdu = (u8)rtw_bt_ampdu;
#endif

	registry_par->bAcceptAddbaReq = (u8)rtw_AcceptAddbaReq;

	registry_par->antdiv_cfg = (u8)rtw_antdiv_cfg;
	registry_par->antdiv_type = (u8)rtw_antdiv_type;

#ifdef CONFIG_AUTOSUSPEND
	registry_par->usbss_enable = (u8)rtw_enusbss;//0:disable,1:enable
#endif
#ifdef SUPPORT_HW_RFOFF_DETECTED
	registry_par->hwpdn_mode = (u8)rtw_hwpdn_mode;//0:disable,1:enable,2:by EFUSE config
	registry_par->hwpwrp_detect = (u8)rtw_hwpwrp_detect;//0:disable,1:enable
#endif

	registry_par->hw_wps_pbc = (u8)rtw_hw_wps_pbc;

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
	snprintf(registry_par->adaptor_info_caching_file_path, PATH_LENGTH_MAX, "%s", rtw_adaptor_info_caching_file_path);
	registry_par->adaptor_info_caching_file_path[PATH_LENGTH_MAX-1]=0;
#endif

	registry_par->max_roaming_times = (u8)rtw_max_roaming_times;

#ifdef CONFIG_IOL
	registry_par->fw_iol = rtw_fw_iol;
#endif

#ifdef CONFIG_80211D
	registry_par->enable80211d = (u8)rtw_80211d;
#endif

	snprintf(registry_par->ifname, 16, "%s", ifname);
	snprintf(registry_par->if2name, 16, "%s", if2name);

	registry_par->notch_filter = (u8)rtw_notch_filter;

	registry_par->regulatory_tid = (u8)rtw_regulatory_id;

_func_exit_;

	return status;
}

static int rtw_net_set_mac_address(struct net_device *pnetdev, void *p)
{
	struct rtw_adapter *padapter = netdev_priv(pnetdev);
	struct sockaddr *addr = p;

	if(padapter->bup == false)
	{
		//DBG_8723A("r8711_net_set_mac_address(), MAC=%x:%x:%x:%x:%x:%x\n", addr->sa_data[0], addr->sa_data[1], addr->sa_data[2], addr->sa_data[3],
		//addr->sa_data[4], addr->sa_data[5]);
		memcpy(padapter->eeprompriv.mac_addr, addr->sa_data, ETH_ALEN);
		//memcpy(pnetdev->dev_addr, addr->sa_data, ETH_ALEN);
		//padapter->bset_hwaddr = true;
	}

	return 0;
}

static struct net_device_stats *rtw_net_get_stats(struct net_device *pnetdev)
{
	struct rtw_adapter *padapter = netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	struct recv_priv *precvpriv = &(padapter->recvpriv);

	padapter->stats.tx_packets = pxmitpriv->tx_pkts;//pxmitpriv->tx_pkts++;
	padapter->stats.rx_packets = precvpriv->rx_pkts;//precvpriv->rx_pkts++;
	padapter->stats.tx_dropped = pxmitpriv->tx_drop;
	padapter->stats.rx_dropped = precvpriv->rx_drop;
	padapter->stats.tx_bytes = pxmitpriv->tx_bytes;
	padapter->stats.rx_bytes = precvpriv->rx_bytes;

	return &padapter->stats;
}

/*
 * AC to queue mapping
 *
 * AC_VO -> queue 0
 * AC_VI -> queue 1
 * AC_BE -> queue 2
 * AC_BK -> queue 3
 */
static const u16 rtw_1d_to_queue[8] = { 2, 3, 3, 2, 1, 1, 0, 0 };

/* Given a data frame determine the 802.1p/1d tag to use. */
static unsigned int rtw_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp;

	/* skb->priority values from 256->263 are magic values to
	 * directly indicate a specific 802.1d priority.  This is used
	 * to allow 802.1d priority to be passed directly in from VLAN
	 * tags, etc.
	 */
	if (skb->priority >= 256 && skb->priority <= 263)
		return skb->priority - 256;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		dscp = ip_hdr(skb)->tos & 0xfc;
		break;
	default:
		return 0;
	}

	return dscp >> 5;
}

static u16 rtw_select_queue(struct net_device *dev, struct sk_buff *skb)
{
	struct rtw_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	skb->priority = rtw_classify8021d(skb);

	if(pmlmepriv->acm_mask != 0)
	{
		skb->priority = qos_acm(pmlmepriv->acm_mask, skb->priority);
	}

	return rtw_1d_to_queue[skb->priority];
}

u16 rtw_recv_select_queue(struct sk_buff *skb)
{
	struct iphdr *piphdr;
	unsigned int dscp;
	u16	eth_type;
	u32 priority;
	u8 *pdata = skb->data;

	memcpy(&eth_type, pdata+(ETH_ALEN<<1), 2);

	switch (eth_type) {
		case htons(ETH_P_IP):

			piphdr = (struct iphdr *)(pdata+ETH_HLEN);

			dscp = piphdr->tos & 0xfc;

			priority = dscp >> 5;

			break;
		default:
			priority = 0;
	}

	return rtw_1d_to_queue[priority];

}

static const struct net_device_ops rtw_netdev_ops = {
	.ndo_open = netdev_open,
	.ndo_stop = netdev_close,
	.ndo_start_xmit = rtw_xmit_entry,
	.ndo_select_queue	= rtw_select_queue,
	.ndo_set_mac_address = rtw_net_set_mac_address,
	.ndo_get_stats = rtw_net_get_stats,
	.ndo_do_ioctl = rtw_ioctl,
};

int rtw_init_netdev_name(struct net_device *pnetdev, const char *ifname)
{
	struct rtw_adapter *padapter = netdev_priv(pnetdev);

	if (dev_alloc_name(pnetdev, ifname) < 0)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("dev_alloc_name, fail! \n"));
	}

	netif_carrier_off(pnetdev);
	return 0;
}

static const struct device_type wlan_type = {
	.name = "wlan",
};

struct net_device *rtw_init_netdev(struct rtw_adapter *old_padapter)
{
	struct rtw_adapter *padapter;
	struct net_device *pnetdev;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+init_net_dev\n"));

	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_adapter), 4);
	if (!pnetdev)
		return NULL;

	pnetdev->dev.type = &wlan_type;
	padapter = netdev_priv(pnetdev);
	padapter->pnetdev = pnetdev;

	//pnetdev->init = NULL;

	DBG_8723A("register rtw_netdev_ops to netdev_ops\n");
	pnetdev->netdev_ops = &rtw_netdev_ops;

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	pnetdev->features |= NETIF_F_IP_CSUM;
#endif
	//pnetdev->tx_timeout = NULL;
	pnetdev->watchdog_timeo = HZ*3; /* 3 second timeout */

#ifdef WIRELESS_SPY
	//priv->wireless_data.spy_data = &priv->spy_data;
	//pnetdev->wireless_data = &priv->wireless_data;
#endif

	//step 2.
	loadparam(padapter, pnetdev);
	return pnetdev;
}

u32 rtw_start_drv_threads(struct rtw_adapter *padapter)
{
	u32 _status = _SUCCESS;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_start_drv_threads\n"));
#ifdef CONFIG_XMIT_THREAD_MODE
	padapter->xmitThread = kthread_run(rtw_xmit_thread, padapter, "RTW_XMIT_THREAD");
	if(IS_ERR(padapter->xmitThread))
		_status = _FAIL;
#endif

#ifdef CONFIG_RECV_THREAD_MODE
	padapter->recvThread = kthread_run(rtw_recv_thread, padapter, "RTW_RECV_THREAD");
	if(IS_ERR(padapter->recvThread))
		_status = _FAIL;
#endif


	padapter->cmdThread = kthread_run(rtw_cmd_thread, padapter, "RTW_CMD_THREAD");
        if(IS_ERR(padapter->cmdThread))
		_status = _FAIL;
	else
		down(&padapter->cmdpriv.terminate_cmdthread_sema); //wait for cmd_thread to run

#ifdef CONFIG_EVENT_THREAD_MODE
	padapter->evtThread = kthread_run(event_thread, padapter, "RTW_EVENT_THREAD");
	if(IS_ERR(padapter->evtThread))
		_status = _FAIL;
#endif

	rtw_hal_start_thread(padapter);
	return _status;
}

void rtw_stop_drv_threads (struct rtw_adapter *padapter)
{
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_stop_drv_threads\n"));

	//Below is to termindate rtw_cmd_thread & event_thread...
	up(&padapter->cmdpriv.cmd_queue_sema);
	//up(&padapter->cmdpriv.cmd_done_sema);
	if(padapter->cmdThread){
		down(&padapter->cmdpriv.terminate_cmdthread_sema);
	}

#ifdef CONFIG_EVENT_THREAD_MODE
        up(&padapter->evtpriv.evt_notify);
	if(padapter->evtThread){
		down_interruptible(&padapter->evtpriv.terminate_evtthread_sema);
	}
#endif

#ifdef CONFIG_XMIT_THREAD_MODE
	// Below is to termindate tx_thread...
	{
		up(&padapter->xmitpriv.xmit_sema);
		down(&padapter->xmitpriv.terminate_xmitthread_sema);
	}
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("\n drv_halt: rtw_xmit_thread can be terminated ! \n"));
#endif

#ifdef CONFIG_RECV_THREAD_MODE
	// Below is to termindate rx_thread...
	up(&padapter->recvpriv.recv_sema);
	down(&padapter->recvpriv.terminate_recvthread_sema);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("\n drv_halt:recv_thread can be terminated! \n"));
#endif

	rtw_hal_stop_thread(padapter);
}

u8 rtw_init_default_value(struct rtw_adapter *padapter)
{
	u8 ret  = _SUCCESS;
	struct registry_priv* pregistrypriv = &padapter->registrypriv;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;

	//xmit_priv
	pxmitpriv->vcs_setting = pregistrypriv->vrtl_carrier_sense;
	pxmitpriv->vcs = pregistrypriv->vcs_type;
	pxmitpriv->vcs_type = pregistrypriv->vcs_type;
	//pxmitpriv->rts_thresh = pregistrypriv->rts_thresh;
	pxmitpriv->frag_len = pregistrypriv->frag_thresh;



	//recv_priv


	//mlme_priv
	pmlmepriv->scan_interval = SCAN_INTERVAL;// 30*2 sec = 60sec
	pmlmepriv->scan_mode = SCAN_ACTIVE;

	//qos_priv
	//pmlmepriv->qospriv.qos_option = pregistrypriv->wmm_enable;

	//ht_priv
#ifdef CONFIG_80211N_HT
	pmlmepriv->htpriv.ampdu_enable = false;//set to disabled
#endif

	//security_priv
	//rtw_get_encrypt_decrypt_from_registrypriv(padapter);
	psecuritypriv->binstallGrpkey = _FAIL;
	psecuritypriv->sw_encrypt=pregistrypriv->software_encrypt;
	psecuritypriv->sw_decrypt=pregistrypriv->software_decrypt;

	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; //open system
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;

	psecuritypriv->dot11PrivacyKeyIndex = 0;

	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpKeyid = 1;

	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;
	psecuritypriv->ndisencryptstatus = Ndis802_11WEPDisabled;


	//pwrctrl_priv


	//registry_priv
	rtw_init_registrypriv_dev_network(padapter);
	rtw_update_registrypriv_dev_network(padapter);


	//hal_priv
	rtw_hal_def_value_init(padapter);

	//misc.
	padapter->bReadPortCancel = false;
	padapter->bWritePortCancel = false;
	padapter->bRxRSSIDisplay = 0;
	padapter->bNotifyChannelChange = 0;
#ifdef CONFIG_P2P
	padapter->bShowGetP2PState = 1;
#endif

	return ret;
}

u8 rtw_reset_drv_sw(struct rtw_adapter *padapter)
{
	u8	ret8=_SUCCESS;
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	//hal_priv
	rtw_hal_def_value_init(padapter);
	padapter->bReadPortCancel = false;
	padapter->bWritePortCancel = false;
	padapter->bRxRSSIDisplay = 0;
	pmlmepriv->scan_interval = SCAN_INTERVAL;// 30*2 sec = 60sec

	padapter->xmitpriv.tx_pkts = 0;
	padapter->recvpriv.rx_pkts = 0;

	pmlmepriv->LinkDetectInfo.bBusyTraffic = false;

	_clr_fwstate_(pmlmepriv, _FW_UNDER_SURVEY |_FW_UNDER_LINKING);

#ifdef DBG_CONFIG_ERROR_DETECT
	rtw_hal_sreset_reset_value(padapter);
#endif
	pwrctrlpriv->pwr_state_check_cnts = 0;

	//mlmeextpriv
	padapter->mlmeextpriv.sitesurvey_res.state= SCAN_DISABLE;

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	rtw_set_signal_stat_timer(&padapter->recvpriv);
#endif

	return ret8;
}


u8 rtw_init_drv_sw(struct rtw_adapter *padapter)
{

	u8	ret8=_SUCCESS;

_func_enter_;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_init_drv_sw\n"));

	if ((rtw_init_cmd_priv(&padapter->cmdpriv)) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init cmd_priv\n"));
		ret8=_FAIL;
		goto exit;
	}

	padapter->cmdpriv.padapter=padapter;

	if ((rtw_init_evt_priv(&padapter->evtpriv)) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init evt_priv\n"));
		ret8=_FAIL;
		goto exit;
	}


	if (rtw_init_mlme_priv(padapter) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init mlme_priv\n"));
		ret8=_FAIL;
		goto exit;
	}

#ifdef CONFIG_P2P
	rtw_init_wifidirect_timers(padapter);
	init_wifidirect_info(padapter, P2P_ROLE_DISABLE);
	reset_global_wifidirect_info(padapter);
	rtw_init_cfg80211_wifidirect_info(padapter);
#ifdef CONFIG_WFD
	if(rtw_init_wifi_display_info(padapter) == _FAIL)
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init init_wifi_display_info\n"));
#endif
#endif /* CONFIG_P2P */

	if(init_mlme_ext_priv(padapter) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init mlme_ext_priv\n"));
		ret8=_FAIL;
		goto exit;
	}

	if(_rtw_init_xmit_priv(&padapter->xmitpriv, padapter) == _FAIL)
	{
		DBG_8723A("Can't _rtw_init_xmit_priv\n");
		ret8=_FAIL;
		goto exit;
	}

	if(_rtw_init_recv_priv(&padapter->recvpriv, padapter) == _FAIL)
	{
		DBG_8723A("Can't _rtw_init_recv_priv\n");
		ret8=_FAIL;
		goto exit;
	}

	if(_rtw_init_sta_priv(&padapter->stapriv) == _FAIL) {
		DBG_8723A("Can't _rtw_init_sta_priv\n");
		ret8=_FAIL;
		goto exit;
	}

	padapter->stapriv.padapter = padapter;
	padapter->setband = GHZ24_50;
	rtw_init_bcmc_stainfo(padapter);

	rtw_init_pwrctrl_priv(padapter);

	//memset((u8 *)&padapter->qospriv, 0, sizeof (struct qos_priv));//move to mlme_priv

	ret8 = rtw_init_default_value(padapter);

	rtw_hal_dm_init(padapter);
	rtw_hal_sw_led_init(padapter);

#ifdef DBG_CONFIG_ERROR_DETECT
	rtw_hal_sreset_init(padapter);
#endif

#ifdef CONFIG_BR_EXT
	spin_lock_init(&padapter->br_ext_lock);
#endif	// CONFIG_BR_EXT

exit:

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-rtw_init_drv_sw\n"));

	_func_exit_;

	return ret8;

}

#ifdef CONFIG_WOWLAN
void rtw_cancel_dynamic_chk_timer(struct rtw_adapter *padapter)
{
	del_timer_sync(&padapter->mlmepriv.dynamic_chk_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel dynamic_chk_timer! \n"));
}
#endif

void rtw_cancel_all_timer(struct rtw_adapter *padapter)
{
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_cancel_all_timer\n"));

	del_timer_sync(&padapter->mlmepriv.assoc_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel association timer complete! \n"));

	del_timer_sync(&padapter->mlmepriv.scan_to_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel scan_to_timer! \n"));

	del_timer_sync(&padapter->mlmepriv.dynamic_chk_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel dynamic_chk_timer! \n"));

	// cancel sw led timer
	rtw_hal_sw_led_deinit(padapter);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel DeInitSwLeds! \n"));

	del_timer_sync(&padapter->pwrctrlpriv.pwr_state_check_timer);

#ifdef CONFIG_P2P
	del_timer_sync(&padapter->cfg80211_wdinfo.remain_on_ch_timer);
#endif //CONFIG_P2P

#ifdef CONFIG_SET_SCAN_DENY_TIMER
	del_timer_sync(&padapter->mlmepriv.set_scan_deny_timer);
	rtw_clear_scan_deny(padapter);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel set_scan_deny_timer! \n"));
#endif

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	del_timer_sync(&padapter->recvpriv.signal_stat_timer);
#endif
#if defined(CONFIG_CHECK_BT_HANG) && defined(CONFIG_BT_COEXIST)
	if (padapter->HalFunc.hal_cancel_checkbthang_workqueue)
		padapter->HalFunc.hal_cancel_checkbthang_workqueue(padapter);
#endif
	//cancel dm timer
	rtw_hal_dm_deinit(padapter);

}

u8 rtw_free_drv_sw(struct rtw_adapter *padapter)
{
	struct net_device *pnetdev = (struct net_device*)padapter->pnetdev;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("==>rtw_free_drv_sw"));

	//we can call rtw_p2p_enable here, but:
	// 1. rtw_p2p_enable may have IO operation
	// 2. rtw_p2p_enable is bundled with wext interface
	#ifdef CONFIG_P2P
	{
		struct wifidirect_info *pwdinfo = &padapter->wdinfo;
		if(!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
		{
			del_timer_sync(&pwdinfo->find_phase_timer);
			del_timer_sync(&pwdinfo->restore_p2p_state_timer);
			del_timer_sync(&pwdinfo->pre_tx_scan_timer);
			rtw_p2p_set_state(pwdinfo, P2P_STATE_NONE);
		}
	}
	#endif

	free_mlme_ext_priv(&padapter->mlmeextpriv);

	rtw_free_cmd_priv(&padapter->cmdpriv);

	rtw_free_evt_priv(&padapter->evtpriv);

	rtw_free_mlme_priv(&padapter->mlmepriv);
#if defined(CONFIG_CHECK_BT_HANG) && defined(CONFIG_BT_COEXIST)
	if (padapter->HalFunc.hal_free_checkbthang_workqueue)
		padapter->HalFunc.hal_free_checkbthang_workqueue(padapter);
#endif
	//free_io_queue(padapter);

	_rtw_free_xmit_priv(&padapter->xmitpriv);

	_rtw_free_sta_priv(&padapter->stapriv); //will free bcmc_stainfo here

	_rtw_free_recv_priv(&padapter->recvpriv);

	rtw_free_pwrctrl_priv(padapter);

	//rtw_mfree((void *)padapter, sizeof (padapter));

#ifdef CONFIG_DRVEXT_MODULE
	free_drvext(&padapter->drvextpriv);
#endif

	rtw_hal_free_data(padapter);

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("<==rtw_free_drv_sw\n"));

	//free the old_pnetdev
	if(padapter->rereg_nd_name_priv.old_pnetdev) {
		free_netdev(padapter->rereg_nd_name_priv.old_pnetdev);
		padapter->rereg_nd_name_priv.old_pnetdev = NULL;
	}

	// clear pbuddy_adapter to avoid access wrong pointer.
	if(padapter->pbuddy_adapter != NULL) {
		padapter->pbuddy_adapter->pbuddy_adapter = NULL;
	}

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-rtw_free_drv_sw\n"));

	return _SUCCESS;

}

#ifdef CONFIG_BR_EXT
void netdev_br_init(struct net_device *netdev)
{
	struct rtw_adapter *adapter = netdev_priv(netdev);

	rcu_read_lock();

	//if(check_fwstate(pmlmepriv, WIFI_STATION_STATE|WIFI_ADHOC_STATE) == true)
	{
		//struct net_bridge	*br = netdev->br_port->br;//->dev->dev_addr;
		if (rcu_dereference(adapter->pnetdev->rx_handler_data))
		{
			struct net_device *br_netdev;
			struct net *devnet = NULL;

			devnet = dev_net(netdev);

			br_netdev = dev_get_by_name(devnet, CONFIG_BR_EXT_BRNAME);

			if (br_netdev) {
				memcpy(adapter->br_mac, br_netdev->dev_addr, ETH_ALEN);
				dev_put(br_netdev);
			} else
				DBG_8723A("%s()-%d: dev_get_by_name(%s) failed!", __FUNCTION__, __LINE__, CONFIG_BR_EXT_BRNAME);
		}

		adapter->ethBrExtInfo.addPPPoETag = 1;
	}

	rcu_read_unlock();
}
#endif //CONFIG_BR_EXT

static int _rtw_drv_register_netdev(struct rtw_adapter *padapter, char *name)
{
	int ret = _SUCCESS;
	struct net_device *pnetdev = padapter->pnetdev;

	/* alloc netdev name */
	rtw_init_netdev_name(pnetdev, name);

	memcpy(pnetdev->dev_addr, padapter->eeprompriv.mac_addr, ETH_ALEN);

	/* Tell the network stack we exist */
	if (register_netdev(pnetdev) != 0) {
		DBG_8723A(FUNC_NDEV_FMT "Failed!\n", FUNC_NDEV_ARG(pnetdev));
		ret = _FAIL;
		goto error_register_netdev;
	}

	DBG_8723A("%s, MAC Address (if%d) = " MAC_FMT "\n", __FUNCTION__, (padapter->iface_id+1), MAC_ARG(pnetdev->dev_addr));

	return ret;

error_register_netdev:

	if(padapter->iface_id > IFACE_ID0)
	{
		rtw_free_drv_sw(padapter);

		free_netdev(pnetdev);
	}

	return ret;
}

int rtw_drv_register_netdev(struct rtw_adapter *if1)
{
	int i, status = _SUCCESS;
	struct dvobj_priv *dvobj = if1->dvobj;

	if(dvobj->iface_nums < IFACE_ID_MAX)
	{
		for(i=0; i<dvobj->iface_nums; i++)
		{
			struct rtw_adapter *padapter = dvobj->padapters[i];

			if(padapter)
			{
				char *name;

				if(padapter->iface_id == IFACE_ID0)
					name = if1->registrypriv.ifname;
				else if(padapter->iface_id == IFACE_ID1)
					name = if1->registrypriv.if2name;
				else
					name = "wlan%d";

				if((status = _rtw_drv_register_netdev(padapter, name)) != _SUCCESS) {
					break;
				}
			}
		}
	}

	return status;
}

int _netdev_open(struct net_device *pnetdev)
{
	uint status;
	struct rtw_adapter *padapter = netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+871x_drv - dev_open\n"));
	DBG_8723A("+871x_drv - drv_open, bup=%d\n", padapter->bup);

	if(pwrctrlpriv->ps_flag == true){
		padapter->net_closed = false;
		goto netdev_open_normal_process;
	}

	if(padapter->bup == false)
	{
		padapter->bDriverStopped = false;
		padapter->bSurpriseRemoved = false;
		padapter->bCardDisableWOHSM = false;

		status = rtw_hal_init(padapter);
		if (status ==_FAIL)
		{
			RT_TRACE(_module_os_intfs_c_,_drv_err_,("rtl871x_hal_init(): Can't init h/w!\n"));
			goto netdev_open_error;
		}

		DBG_8723A("MAC Address = "MAC_FMT"\n", MAC_ARG(pnetdev->dev_addr));

#ifdef CONFIG_RF_GAIN_OFFSET
		rtw_bb_rf_gain_offset(padapter);
#endif //CONFIG_RF_GAIN_OFFSET

		status=rtw_start_drv_threads(padapter);
		if(status ==_FAIL)
		{
			DBG_8723A("Initialize driver software resource Failed!\n");
			goto netdev_open_error;
		}

		if (init_hw_mlme_ext(padapter) == _FAIL)
		{
			DBG_8723A("can't init mlme_ext_priv\n");
			goto netdev_open_error;
		}

#ifdef CONFIG_DRVEXT_MODULE
		init_drvext(padapter);
#endif

		if(padapter->intf_start)
		{
			padapter->intf_start(padapter);
		}

		rtw_proc_init_one(pnetdev);

		rtw_cfg80211_init_wiphy(padapter);

		rtw_led_control(padapter, LED_CTL_NO_LINK);

		padapter->bup = true;
	}
	padapter->net_closed = false;

	_set_timer(&padapter->mlmepriv.dynamic_chk_timer, 2000);

	padapter->pwrctrlpriv.bips_processing = false;
	rtw_set_pwr_state_check_timer(&padapter->pwrctrlpriv);

	//netif_carrier_on(pnetdev);//call this func when rtw_joinbss_event_callback return success
	if(!rtw_netif_queue_stopped(pnetdev))
		rtw_netif_start_queue(pnetdev);
	else
		rtw_netif_wake_queue(pnetdev);

#ifdef CONFIG_BR_EXT
	netdev_br_init(pnetdev);
#endif	// CONFIG_BR_EXT

netdev_open_normal_process:

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-871x_drv - dev_open\n"));
	DBG_8723A("-871x_drv - drv_open, bup=%d\n", padapter->bup);

	return 0;

netdev_open_error:

	padapter->bup = false;

	netif_carrier_off(pnetdev);
	rtw_netif_stop_queue(pnetdev);

	RT_TRACE(_module_os_intfs_c_,_drv_err_,("-871x_drv - dev_open, fail!\n"));
	DBG_8723A("-871x_drv - drv_open fail, bup=%d\n", padapter->bup);

	return (-1);

}

int netdev_open(struct net_device *pnetdev)
{
	int ret;
	struct rtw_adapter *padapter = netdev_priv(pnetdev);

	mutex_lock(&(adapter_to_dvobj(padapter)->hw_init_mutex));
	ret = _netdev_open(pnetdev);
	mutex_unlock(&(adapter_to_dvobj(padapter)->hw_init_mutex));

	return ret;
}

#ifdef CONFIG_IPS
static int  ips_netdrv_open(struct rtw_adapter *padapter)
{
	int status = _SUCCESS;
	padapter->net_closed = false;
	DBG_8723A("===> %s.........\n",__FUNCTION__);


	padapter->bDriverStopped = false;
	padapter->bSurpriseRemoved = false;
	padapter->bCardDisableWOHSM = false;
	//padapter->bup = true;

	status = rtw_hal_init(padapter);
	if (status ==_FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("ips_netdrv_open(): Can't init h/w!\n"));
		goto netdev_open_error;
	}

	if(padapter->intf_start)
	{
		padapter->intf_start(padapter);
	}

	rtw_set_pwr_state_check_timer(&padapter->pwrctrlpriv);
	_set_timer(&padapter->mlmepriv.dynamic_chk_timer,5000);

	 return _SUCCESS;

netdev_open_error:
	//padapter->bup = false;
	DBG_8723A("-ips_netdrv_open - drv_open failure, bup=%d\n", padapter->bup);

	return _FAIL;
}


int rtw_ips_pwr_up(struct rtw_adapter *padapter)
{
	int result;
	u32 start_time = rtw_get_current_time();
	DBG_8723A("===>  rtw_ips_pwr_up..............\n");
	rtw_reset_drv_sw(padapter);

	result = ips_netdrv_open(padapter);

	rtw_led_control(padapter, LED_CTL_NO_LINK);

	DBG_8723A("<===  rtw_ips_pwr_up.............. in %dms\n", rtw_get_passing_time_ms(start_time));
	return result;

}

void rtw_ips_pwr_down(struct rtw_adapter *padapter)
{
	u32 start_time = rtw_get_current_time();
	DBG_8723A("===> rtw_ips_pwr_down...................\n");

	padapter->bCardDisableWOHSM = true;
	padapter->net_closed = true;

	rtw_led_control(padapter, LED_CTL_POWER_OFF);

	rtw_ips_dev_unload(padapter);
	padapter->bCardDisableWOHSM = false;
	DBG_8723A("<=== rtw_ips_pwr_down..................... in %dms\n", rtw_get_passing_time_ms(start_time));
}
#endif
void rtw_ips_dev_unload(struct rtw_adapter *padapter)
{
	struct net_device *pnetdev= (struct net_device*)padapter->pnetdev;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	DBG_8723A("====> %s...\n",__FUNCTION__);

	rtw_hal_set_hwreg(padapter, HW_VAR_FIFO_CLEARN_UP, NULL);

	if(padapter->intf_stop)
	{
		padapter->intf_stop(padapter);
	}

	//s5.
	if(padapter->bSurpriseRemoved == false)
	{
		rtw_hal_deinit(padapter);
	}

}

#ifdef CONFIG_RF_GAIN_OFFSET
void rtw_bb_rf_gain_offset(struct rtw_adapter *padapter)
{
	u8      value = padapter->eeprompriv.EEPROMRFGainOffset;
	u8      tmp = 0x3e;
	u32     res;

	DBG_8723A("+%s value: 0x%02x+\n", __func__, value);

	if (!(value & 0x01)) {
		DBG_8723A("Offset RF Gain.\n");
		res = rtw_hal_read_rfreg(padapter, RF_PATH_A, REG_RF_BB_GAIN_OFFSET, 0xffffffff);
		value &= tmp;
		res = value << 14;
		rtw_hal_write_rfreg(padapter, RF_PATH_A, REG_RF_BB_GAIN_OFFSET, RF_GAIN_OFFSET_MASK, res);
	} else {
		DBG_8723A("Using the default RF gain.\n");
	}
}
#endif //CONFIG_RF_GAIN_OFFSET

int pm_netdev_open(struct net_device *pnetdev,u8 bnormal)
{
	int status;


	if (true == bnormal)
		status = netdev_open(pnetdev);
#ifdef CONFIG_IPS
	else
		status =  (_SUCCESS == ips_netdrv_open(netdev_priv(pnetdev)))?(0):(-1);
#endif

	return status;
}

static int netdev_close(struct net_device *pnetdev)
{
	struct rtw_adapter *padapter = netdev_priv(pnetdev);

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+871x_drv - drv_close\n"));

	if(padapter->pwrctrlpriv.bInternalAutoSuspend == true)
	{
		//rtw_pwr_wakeup(padapter);
		if(padapter->pwrctrlpriv.rf_pwrstate == rf_off)
			padapter->pwrctrlpriv.ps_flag = true;
	}
	padapter->net_closed = true;

	if(padapter->pwrctrlpriv.rf_pwrstate == rf_on){
		DBG_8723A("(2)871x_drv - drv_close, bup=%d, hw_init_completed=%d\n", padapter->bup, padapter->hw_init_completed);

		//s1.
		if(pnetdev)
		{
			if (!rtw_netif_queue_stopped(pnetdev))
				rtw_netif_stop_queue(pnetdev);
		}

		//s2.
		LeaveAllPowerSaveMode(padapter);
		rtw_disassoc_cmd(padapter, 500, false);
		//s2-2.  indicate disconnect to os
		rtw_indicate_disconnect(padapter);
		//s2-3.
		rtw_free_assoc_resources(padapter, 1);
		//s2-4.
		rtw_free_network_queue(padapter,true);
		// Close LED
		rtw_led_control(padapter, LED_CTL_POWER_OFF);
	}

#ifdef CONFIG_BR_EXT
	nat25_db_cleanup(padapter);
#endif	// CONFIG_BR_EXT

#ifdef CONFIG_P2P
	if(wdev_to_priv(padapter->rtw_wdev)->p2p_enabled == true)
		wdev_to_priv(padapter->rtw_wdev)->p2p_enabled = false;
	rtw_p2p_enable(padapter, P2P_ROLE_DISABLE);
#endif //CONFIG_P2P

	rtw_scan_abort(padapter);
	wdev_to_priv(padapter->rtw_wdev)->bandroid_scan = false;
	padapter->rtw_wdev->iftype = NL80211_IFTYPE_MONITOR; //set this at the end

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-871x_drv - drv_close\n"));
	DBG_8723A("-871x_drv - drv_close, bup=%d\n", padapter->bup);

	return 0;
}

void rtw_ndev_destructor(struct net_device *ndev)
{
	DBG_8723A(FUNC_NDEV_FMT"\n", FUNC_NDEV_ARG(ndev));

	if (ndev->ieee80211_ptr)
		kfree(ndev->ieee80211_ptr);
	free_netdev(ndev);
}

#CHIP_RTL8366SR=y
SWITCH_MODULE=y
CUSTOMER_DUSA=y #other is wordwide 

##############################################
##naming rule: 
##             file name                       signature
##USA+SR  dir685_v1.00NA_933e.bin         wrgns01_dlwbr_dir685
##USA+RB  dir685_v1.00NA_RB933e.bin       wrgns01_dlwbr_dir685RB
##WW +SR  dir685_v1.00_933e.bin           wrgns01_dlwbr_dir685WW
##WW +RB  dir685_v1.00_RB933e.bin         wrgns01_dlwbr_dir685RBWW
##
##################################################


#Image signature	re-alloc the signature //joel
DUSA_DLINK_VWRSION = 1.01NA
EU_DLINK_VWRSION = 1.00

ifdef CHIP_RTL8366SR

ifdef CUSTOMER_DUSA
ELBOX_FIRMWARE_VERSION :=$(DUSA_DLINK_VWRSION)
EXTRA_SIG := 
else
ELBOX_FIRMWARE_VERSION :=$(EU_DLINK_VWRSION)
EXTRA_SIG := WW
endif

ELBOX_SIGNATURE := $(shell echo $(ELBOX_BOARD_NAME)_$(ELBOX_BRAND_NAME)_$(ELBOX_MODEL_NAME)$(EXTRA_SIG))
RELIMAGE:=$(shell echo $(ELBOX_MODEL_NAME)_v$(ELBOX_FIRMWARE_VERSION)_$(BUILDNO))

else

ifdef CUSTOMER_DUSA
ELBOX_FIRMWARE_VERSION :=$(DUSA_DLINK_VWRSION)
EXTRA_SIG := RB
else
ELBOX_FIRMWARE_VERSION :=$(EU_DLINK_VWRSION)
EXTRA_SIG := RBWW
endif

ELBOX_SIGNATURE := $(shell echo $(ELBOX_BOARD_NAME)_$(ELBOX_BRAND_NAME)_$(ELBOX_MODEL_NAME)$(EXTRA_SIG))

endif

#this is for the DUSA remove 11b wireless,need different default config...
ifeq ($(strip $(CUSTOMER_DUSA)),y)
EL_GENDEF		:= $(shell echo $(TOPDIR)/templates/$(ELBOX_TEMPLATE_NAME)/$(ELBOX_BRAND_NAME)/$(ELBOX_MODEL_NAME)/customer_dusa/gendef.sh)

endif

ifndef AVOID_DOUBLE_INCLUDE
AVOID_DOUBLE_INCLUDE :=y
vchange: 
	make mykernel
	make -C progs.board/DFB/nas_ui/nas_app clean
	rm -rf progs.board/*.o
	make all_without_kernel
	@echo -e "\033[32mbuild from vchange success...\033[0m"

.PHONY: vchange

#joel always re-gen default link
#-include dir685_init.mk
#ifneq ($(strip $(HAVE_REGEN)),y)
#$(shell echo HAVE_REGEN=y > dir685_init.mk)
$(shell rm -rf gendef.sh)
$(shell ln -s $(EL_GENDEF)	gendef.sh)
#endif
endif

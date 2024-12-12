 ###################################################################
 # Copyright 2022 GL-iNet. https://www.gl-inet.com/
 # 
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 # http://www.apache.org/licenses/LICENSE-2.0
 # 
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 ####################################################################
include $(TOPDIR)/rules.mk
include version.mk

PKG_NAME:=gl-ble-sdk
PKG_VERSION:=$(strip $(call findrev))
PKG_RELEASE:=2

TARGET_CFLAGS += -ggdb3


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libglble
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE driver library
	DEPENDS:= +libuci +libpthread +lrzsz +@BUSYBOX_CUSTOM +@BUSYBOX_CONFIG_STTY
endef

define Package/gl-bleScanner
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE Scanner
	DEPENDS:= +libjson-c +libglble
endef

define Package/gl-bleSynchronize
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE Synchronize
	DEPENDS:= +libjson-c +libglble
endef

define Package/gl-bleAdvLegacy
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE Legacy Advertising
	DEPENDS:= +libjson-c +libglble
endef

define Package/gl-bleAdvExtended
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE Extended Advertising
	DEPENDS:= +libjson-c +libglble
endef

define Package/gl-bleAdvPeriodic
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE Periodic Advertising
	DEPENDS:= +libjson-c +libglble
endef

define Package/gl-bleClient
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE Client
	DEPENDS:= +libjson-c +libglble
endef

define Package/gl-bleService
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE Service
	DEPENDS:= +libjson-c +libglble
endef

define Package/gl-bletool
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE Debug Cmdline Tool
	DEPENDS:= +libjson-c +libreadline +libncurses +libglble
endef

define Package/gl-bleIbeacon
	SECTION:=base
	CATEGORY:=gl-inet-ble
	TITLE:=GL inet BLE Ibeacon
	DEPENDS:= +libjson-c +libglble +libubox +libubus +libblobmsg-json 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/lib/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/lib/libglbleapi.* $(1)/usr/lib/
endef

define Package/libglble/install
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib/gl
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/gl
	$(LN) /usr/lib/gl/libglbleapi.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) files/etc/uci-defaults/30-ble-model-config $(1)/etc/uci-defaults/30-ble-model-config
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) files/etc/gl_gattdb_cfg.json $(1)/etc/gl_gattdb_cfg.json

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) files/usr/bin/* $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/ble
	$(CP) files/etc/ble/* $(1)/etc/ble
endef

define Package/gl-bleScanner/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bleScanner $(1)/usr/sbin/
endef

define Package/gl-bleSynchronize/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bleSynchronize $(1)/usr/sbin/
endef

define Package/gl-bleAdvLegacy/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bleAdvLegacy $(1)/usr/sbin/
endef

define Package/gl-bleAdvExtended/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bleAdvExtended $(1)/usr/sbin/
endef

define Package/gl-bleAdvPeriodic/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bleAdvPeriodic $(1)/usr/sbin/
endef

define Package/gl-bleClient/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bleClient $(1)/usr/sbin/
endef

define Package/gl-bleService/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bleService $(1)/usr/sbin/
endef

define Package/gl-bletool/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bletool $(1)/usr/sbin/
endef

define Package/gl-bleIbeacon/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bleIbeacon $(1)/usr/sbin/
endef


$(eval $(call BuildPackage,libglble))
$(eval $(call BuildPackage,gl-bleScanner))
$(eval $(call BuildPackage,gl-bleSynchronize))
$(eval $(call BuildPackage,gl-bleAdvLegacy))
$(eval $(call BuildPackage,gl-bleAdvExtended))
$(eval $(call BuildPackage,gl-bleAdvPeriodic))
$(eval $(call BuildPackage,gl-bleClient))
$(eval $(call BuildPackage,gl-bleService))
$(eval $(call BuildPackage,gl-bletool))
$(eval $(call BuildPackage,gl-bleIbeacon))
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ktxPubInc := include
ktxLibSrc := lib

LOCAL_MODULE    := ktx
FILE_LIST := $(wildcard $(LOCAL_PATH)/$(ktxLibSrc)/*.c*)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_CFLAGS := -DANDROID
ifneq ($(OPTIMIZE),true)
  LOCAL_CFLAGS += -g
endif

LOCAL_C_INCLUDES	:= $(LOCAL_PATH)/$(ktxPubInc) \
						$(LOCAL_PATH)/$(ktxLibSrc) \
						$(ROOT_PATH)

include $(BUILD_STATIC_LIBRARY)

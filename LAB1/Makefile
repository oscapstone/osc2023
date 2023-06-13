# 設定 ARM 編譯器名稱
:ARMGNU ?= aarch64-linux-gnu

# 設定編譯與組譯選項
COPS = -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only
ASMOPS = -Iinclude 

# 設定輸出目錄與源代碼目錄
BUILD_DIR = build
SRC_DIR = src

# 建立 all 規則，dependency為 kernel8.img
all : kernel8.img

# 建立 clean 規則，用於刪除所有編譯產生的img檔案
clean :
	rm -rf $(BUILD_DIR) *.img 

# 建立編譯 C 語言的規則
$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c -g $< -o $@

# 建立編譯組合語言的規則
$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c -g $< -o $@

# 搜尋所有的 C 與組合語言源代碼檔案
C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)

# 建立對應的目標檔案列表
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

# 建立目標檔案的依賴關係列表
DEP_FILES = $(OBJ_FILES:%.o=%.d)

# 使用 -include 指令加入目標檔案的依賴關係
-include $(DEP_FILES)

# 建立產生 kernel8.img 目標的規則
kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -g -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

# 建立 debug 規則，用於啟動 qemu 進行除錯
debug: 
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -S -s -display none

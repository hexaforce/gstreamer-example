# Makefile

# デフォルトのターゲットを指定
.DEFAULT_GOAL := all

# ビルドディレクトリ
BUILD_DIR := build

# ビルドおよびインストール
.PHONY: all
all:
	@echo "Building project..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@cd $(BUILD_DIR) && make -j$(shell nproc)

# クリーンアップ
.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR)

# ヘルプメッセージ
.PHONY: help
help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all      : Build the project (default target)"
	@echo "  clean    : Clean up built files"
	@echo "  help     : Show this help message"

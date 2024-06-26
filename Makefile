# Makefile

# デフォルトのターゲットを指定
.DEFAULT_GOAL := all

# ビルドディレクトリ
BUILD_DIR := build

# ビルドおよびインストール
.PHONY: all
all:
	@echo "Building project..."
	@cd RTPStreaming
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@cd $(BUILD_DIR) && make -j$(shell nproc)

# クリーンアップ
.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR)

# RPMパッケージのビルド
.PHONY: rpm
rpm:
	@echo "Building RPM package..."
	@rm -rf BUILD BUILDROOT RPMS SRPMS  # RPMビルド前にクリーンアップ
	@rpmbuild -ba rpm.spec

# Debianパッケージのビルド
.PHONY: deb
deb:
	@echo "Building Debian package..."
	@rm -rf $(BUILD_DIR)
	@debuild -us -uc

# ヘルプメッセージ
.PHONY: help
help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all      : Build the project (default target)"
	@echo "  rpm      : Build RPM package"
	@echo "  deb      : Build Debian package"
	@echo "  clean    : Clean up built files"
	@echo "  help     : Show this help message"

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

# RPMパッケージのビルド
.PHONY: rpm
rpm:
	@echo "Building RPM package..."
	@rm -rf BUILD BUILDROOT RPMS SRPMS  # RPMビルド前にクリーンアップ
	@rpmbuild -ba rpm.spec

# Debianパッケージのビルド
.PHONY: deb
deb:
	@echo "Building Debian package.

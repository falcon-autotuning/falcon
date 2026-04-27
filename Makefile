# Falcon-lib Root Makefile
# Manages build configurations for all submodules

.PHONY: all deps help clean install-vcpkg-deps build-all test-all install-deps install-libs

VCPKG_ROOT ?= $(CURDIR)/.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
UNAME_S := $(shell uname -s)

# GitHub release download base URL
GITHUB_RELEASE_URL = https://github.com/$(REPO)/releases/download/$(RELEASE_TAG)

PREFIX ?= /opt/falcon
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include

# Detect OS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
    CMAKE_GENERATOR := Ninja
    VCPKG_TRIPLET ?= x64-linux-dynamic
    NPROC := $(shell nproc 2>/dev/null || echo 4)
    SUDO ?= sudo
		export CC=clang
		export CXX=clang++
endif
ifeq ($(OS),Windows_NT)
    PLATFORM := windows
    CMAKE_GENERATOR := "Visual Studio 17 2022"
    VCPKG_TRIPLET ?= x64-windows
    NPROC := 4
SUDO := 
endif

ENV_FILE := .nuget-credentials
ifeq ($(wildcard $(ENV_FILE)),)
  $(info [Makefile] $(ENV_FILE) not found, skipping environment sourcing)
else
  include $(ENV_FILE)
  export $(shell sed 's/=.*//' $(ENV_FILE) | xargs)
  $(info [Makefile] Loaded environment from $(ENV_FILE))
endif
# ── Paths ─────────────────────────────────────────────────────────────────────
VCPKG_ROOT ?= $(CURDIR)/vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
VCPKG_INSTALLED_DIR ?= $(CURDIR)/vcpkg_installed
FEED_URL ?= 
NUGET_API_KEY ?=
FEED_NAME ?= 
USERNAME ?=
VCPKG_BINARY_SOURCES ?= ""
ifeq ($(strip $(FEED_URL)),)
  CMAKE_VCPKG_BINARY_SOURCES :=
else
	VCPKG_BINARY_SOURCES := "clear;nuget,$(FEED_URL),readwrite"
  CMAKE_VCPKG_BINARY_SOURCES := -DVCPKG_BINARY_SOURCES=$(VCPKG_BINARY_SOURCES)
endif
LINKER_FLAGS ?=
ifeq ($(PLATFORM),linux)
	LINKER_FLAGS := -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld"
endif

all: build-all

.PHONY: vcpkg-bootstrap
vcpkg-bootstrap:
	@if [ ! -d "$(VCPKG_ROOT)" ]; then \
		echo "Cloning vcpkg..."; \
		git clone https://github.com/microsoft/vcpkg.git $(VCPKG_ROOT); \
	fi
	@if [ ! -f "$(VCPKG_ROOT)/vcpkg" ]; then \
		echo "Bootstrapping vcpkg..."; \
		cd $(VCPKG_ROOT) && ./bootstrap-vcpkg.sh; \
	fi

setup-nuget-auth:
	@if [ -z "$$NUGET_API_KEY" ]; then \
		echo "No NUGET_API_KEY found, skipping NuGet setup (local-only build, no binary cache)."; \
		exit 0; \
	fi
	@echo "Setting up NuGet authentication for vcpkg binary caching..."
	@if ! command -v mono >/dev/null 2>&1; then \
		echo "Error: mono is not installed. Please install mono (e.g., 'sudo pacman -S mono' on Arch, 'sudo apt install mono-complete' on Ubuntu)."; \
		exit 1; \
	fi
	@NUGET_EXE=$$(vcpkg fetch nuget | tail -n1); \
	mono "$$NUGET_EXE" sources remove -Name "$(FEED_NAME)" || true; \
	mono "$$NUGET_EXE" sources add -Name "$(FEED_NAME)" -Source "$(FEED_URL)" -Username "$(USERNAME)" -Password "$(NUGET_API_KEY)"

.PHONY: vcpkg-install-deps
vcpkg-install-deps: setup-nuget-auth 
	@echo "Installing vcpkg dependencies" 
	VCPKG_FEATURE_FLAGS=binarycaching MAKELEVEL=0 \
		$(VCPKG_ROOT)/vcpkg install \
		--overlay-ports=ports \
		--binarysource=$(VCPKG_BINARY_SOURCES) \
		--triplet="$(VCPKG_TRIPLET)"

install: install-libs
	@echo "Installing falcon metapackage wrapper..."
	$(SUDO) mkdir -p $(PREFIX)/bin $(PREFIX)/lib $(PREFIX)/include
	$(SUDO) find $(CURDIR)/vcpkg_installed/$(VCPKG_TRIPLET)/tools -type f -exec cp -v {} $(PREFIX)/bin/ \; 2>/dev/null || true
	$(SUDO) cp -P $(CURDIR)/vcpkg_installed/$(VCPKG_TRIPLET)/lib/*.so* $(PREFIX)/lib/ 2>/dev/null || true
	$(SUDO) cp -r $(CURDIR)/vcpkg_installed/$(VCPKG_TRIPLET)/include/* $(PREFIX)/include/ 2>/dev/null || true
	@echo "✓ Metapackage binaries installed to $(PREFIX)"

install-libs:
	@echo "Installing standard libraries to $(PREFIX)/packages..."
	$(SUDO) mkdir -p $(PREFIX)/packages
	$(SUDO) cp -r $(CURDIR)/libs/* $(PREFIX)/packages/
	@echo "✓ Standard libraries installed."

clean:
	@echo "Cleaning all components..."
	rm -rf $(VCPKG_ROOT)
	rm -rf ./vcpkg_installed/
	@echo "✓ Clean complete"

help:
	@echo "Falcon Library Root Makefile"
	@echo "============================"
	@echo ""
	@echo "Setup targets:"
	@echo "  make deps               - Install or update vcpkg"
	@echo "  make install-vcpkg-deps - Install all dependencies"
	@echo "  make install-core       - Install falcon_core"
	@echo ""
	@echo "Build targets:"
	@echo "  make install           - Install all components"
	@echo "  make clean             - Clean all builds"
	@echo ""
	@echo "Component-specific:"
	@echo "  make -C database <target>   - Run target in database/"
	@echo "  make -C autotuner <target>  - Run target in autotuner/"
	@echo ""
	@echo "Current configuration:"
	@echo "  VCPKG_ROOT: $(VCPKG_ROOT)"
	@echo "  VCPKG_TRIPLET: $(VCPKG_TRIPLET)"

# ==========================================
# Docker & Database Configuration Targets
# ==========================================

RELEASE_VERSION ?= v1.1.0
PACKAGE_DIR ?= $(CURDIR)/packaging/release

DOCKER_IMAGE ?= falcon:latest
DOCKER_REGISTRY ?= ghcr.io
DOCKER_REPO ?= falcon-autotuning/falcon
DOCKER_TAG ?= latest
DB_CONTAINER_NAME ?= falcon-postgres
DB_PORT ?= 5432
CONFIG_VOLUME ?= falcon-config
DB_DATA_VOLUME ?= falcon-postgres-data

.PHONY: docker-build docker-push docker-pull docker-db-start docker-db-stop docker-db-purge docker-install-wrappers docker-uninstall-wrappers docker-teardown

docker-build:
	@echo "Building FAlCon Docker Image..."
	docker build -t $(DOCKER_IMAGE) -f packaging/docker/Dockerfile .
	@echo "✓ Docker image $(DOCKER_IMAGE) built successfully."

package-release:
	@echo "Packaging release artifacts for version $(RELEASE_VERSION)..."
	mkdir -p $(PACKAGE_DIR)
	
	# Linux/Mac package
	rm -rf $(PACKAGE_DIR)/linux
	mkdir -p $(PACKAGE_DIR)/linux/falcon/bin
	cp packaging/wrappers/linux_mac/*.sh $(PACKAGE_DIR)/linux/falcon/bin/
	# Remove .sh extensions
	for f in $(PACKAGE_DIR)/linux/falcon/bin/*.sh; do mv "$$f" "$${f%.sh}"; done
	chmod +x $(PACKAGE_DIR)/linux/falcon/bin/*
	tar -czf $(PACKAGE_DIR)/falcon-$(RELEASE_VERSION)-Linux.tar.gz -C $(PACKAGE_DIR)/linux falcon
	rm -rf $(PACKAGE_DIR)/linux
	
	# Windows package
	rm -rf $(PACKAGE_DIR)/windows
	mkdir -p $(PACKAGE_DIR)/windows/falcon/bin
	cp packaging/wrappers/windows/*.bat $(PACKAGE_DIR)/windows/falcon/bin/
	cd $(PACKAGE_DIR)/windows && zip -r $(PACKAGE_DIR)/falcon-$(RELEASE_VERSION)-win64.zip falcon
	rm -rf $(PACKAGE_DIR)/windows
	
	@echo "✓ Release packages created in $(PACKAGE_DIR):"
	@ls -lh $(PACKAGE_DIR)

docker-push:
	@echo "Tagging and pushing $(DOCKER_IMAGE) to $(DOCKER_REGISTRY)..."
	docker tag $(DOCKER_IMAGE) $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(DOCKER_TAG)
	docker push $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(DOCKER_TAG)

docker-pull:
	@echo "Pulling $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(DOCKER_TAG)..."
	docker pull $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(DOCKER_TAG)
	docker tag $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(DOCKER_TAG) $(DOCKER_IMAGE)

docker-db-start:
	@echo "Creating secure volume..."
	@docker volume create $(CONFIG_VOLUME) > /dev/null
	@echo "Launching secure configuration prompt..."
	@# 1. Spin up an interactive bash container. 
	@# It captures input securely (hiding the password) and writes to the volume.
	@docker run -it --rm -v $(CONFIG_VOLUME):/config bash -c "\
		echo '=== FAlCon Database Setup ===' && \
		read -p 'Database Username [falcon_user]: ' usr && usr=\$${usr:-falcon_user} && \
		read -s -p 'Database Password: ' pass && echo && \
		read -p 'Database Name [falcon_db]: ' dbname && dbname=\$${dbname:-falcon_db} && \
		echo \"\$$usr\" > /config/db_user.txt && \
		echo \"\$$pass\" > /config/db_pass.txt && \
		echo \"\$$dbname\" > /config/db_name.txt && \
		echo \"export FALCON_DB_URL=postgresql://\$$usr:\$$pass@host.docker.internal:$(DB_PORT)/\$$dbname\" > /config/db.env && \
		echo '✓ Credentials securely stored in Docker volume.'"
	@echo "Starting PostgreSQL container..."
	@# 2. Start Postgres using the _FILE variables, pointing to the volume.
	@docker run -d --name $(DB_CONTAINER_NAME) \
		-v $(CONFIG_VOLUME):/config:ro \
		-v $(DB_DATA_VOLUME):/var/lib/postgresql/data \
		-e POSTGRES_USER_FILE=/config/db_user.txt \
		-e POSTGRES_PASSWORD_FILE=/config/db_pass.txt \
		-e POSTGRES_DB_FILE=/config/db_name.txt \
		-p $(DB_PORT):5432 \
		postgres:15
	@echo "✓ Database started securely with persistent volume $(DB_DATA_VOLUME)."

docker-db-stop:
	@echo "Stopping database container (data persists in volumes)..."
	-docker stop $(DB_CONTAINER_NAME)
	-docker rm $(DB_CONTAINER_NAME)
	@echo "✓ Database container stopped."

docker-db-purge:
	@echo "DANGER: Destroying all database data and configurations..."
	-docker stop $(DB_CONTAINER_NAME)
	-docker rm $(DB_CONTAINER_NAME)
	-docker volume rm $(CONFIG_VOLUME)
	-docker volume rm $(DB_DATA_VOLUME)
	@echo "✓ Persistence volumes destroyed."

docker-install-wrappers:
	@if [ "$(UNAME_S)" = "Linux" ] || [ "$(UNAME_S)" = "Darwin" ]; then \
		echo "Installing wrapper scripts to /usr/local/bin..."; \
		$(SUDO) cp packaging/wrappers/linux_mac/*.sh /usr/local/bin/; \
		$(SUDO) rename 's/\.sh$$//' /usr/local/bin/falcon-*.sh 2>/dev/null || \
			for f in /usr/local/bin/falcon-*.sh; do $(SUDO) mv "$$f" "$${f%.sh}"; done; \
		$(SUDO) chmod +x /usr/local/bin/falcon-*; \
		echo "✓ Wrappers installed. You can now run 'falcon-run', etc."; \
	else \
		echo "For Windows, please manually add 'packaging/wrappers/windows' to your PATH."; \
	fi

docker-uninstall-wrappers:
	@if [ "$(UNAME_S)" = "Linux" ] || [ "$(UNAME_S)" = "Darwin" ]; then \
		echo "Removing wrapper scripts from /usr/local/bin..."; \
		$(SUDO) rm -f /usr/local/bin/falcon-run /usr/local/bin/falcon-test /usr/local/bin/falcon-pm /usr/local/bin/falcon-db-cli; \
		echo "✓ Wrappers uninstalled."; \
	else \
		echo "For Windows, please manually remove the folder from your PATH."; \
	fi

docker-teardown: docker-db-stop docker-uninstall-wrappers
	@echo "Removing FAlCon Docker image..."
	-docker rmi $(DOCKER_IMAGE)
	@echo "✓ Teardown complete."

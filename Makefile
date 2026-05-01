.PHONY: help configure build test install clean vcpkg-bootstrap

# Detect the preset from CMAKE_PRESET environment variable or default to linux-clang-release
PRESET ?= linux-clang-release
CMAKE_BUILD_DIR := build/$(PRESET)

help:
	@echo "Falcon Build System"
	@echo "========================"
	@echo ""
	@echo "Available presets:"
	@cmake --list-presets=all
	@echo ""
	@echo "Usage:"
	@echo "  make configure PRESET=<preset>  - Configure build (default: $(PRESET))"
	@echo "  make build PRESET=<preset>      - Build (default: $(PRESET))"
	@echo "  make test PRESET=<preset>       - Run tests (default: $(PRESET))"
	@echo "  make install PRESET=<preset>    - Install to /opt/falcon"
	@echo "  make clean                      - Clean all build artifacts"
	@echo ""
	@echo "Examples:"
	@echo "  make build                                      # Build with clang (default)"
	@echo "  make build PRESET=linux-gcc-release             # Build with gcc"
	@echo "  make install PRESET=linux-gcc-release           # Install gcc build"
	@echo ""
	@echo "Or use cmake directly:"
	@echo "  cmake --preset linux-clang-release"
	@echo "  cmake --build --preset linux-clang-release"
	@echo "  ctest --preset linux-clang-release"

vcpkg-bootstrap:
	@echo "Bootstrapping vcpkg..."
	MAKELEVEL=0 cmake -P cmake/bootstrap/bootstrap-vcpkg.cmake

configure: vcpkg-bootstrap
	@echo "Configuring $(PRESET)..."
	cmake --preset $(PRESET)

build: configure
	@echo "Building $(PRESET)..."
	cmake --build --preset $(PRESET)

install: build
	@echo "Installing $(PRESET) to /opt/falcon..."
	cmake --install $(CMAKE_BUILD_DIR) --prefix /opt/falcon

clean:
	@echo "Cleaning all build artifacts..."
	rm -rf build vcpkg_installed
	@echo "✓ Clean complete"

# ==========================================
# Docker & Database Configuration Targets
# ==========================================

RELEASE_VERSION ?= v1.1.2
PACKAGE_DIR ?= $(CURDIR)/packaging/release

DOCKER_REGISTRY ?= ghcr.io
DOCKER_REPO ?= falcon-autotuning/falcon
DOCKER_TAG ?= latest
DOCKER_IMAGE ?= falcon:$(DOCKER_TAG)
DB_CONTAINER_NAME ?= falcon-postgres
DB_PORT ?= 5432
CONFIG_VOLUME ?= falcon-config
DB_DATA_VOLUME ?= falcon-postgres-data

.PHONY: docker-build docker-push docker-pull docker-db-start docker-db-stop docker-db-purge docker-teardown

docker-build:
	@echo "Building Falcon Docker Image..."
	docker build -t $(DOCKER_IMAGE) -t $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(RELEASE_VERSION) -f packaging/docker/Dockerfile .
	@echo "✓ Docker image built with tags:"
	@echo "  - $(DOCKER_IMAGE)"
	@echo "  - $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(RELEASE_VERSION)"

docker-push:
	@echo "Pushing $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(RELEASE_VERSION) to registry..."
	docker push $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(DOCKER_TAG)
	docker push $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(RELEASE_VERSION)
	@echo "✓ Pushed both $(DOCKER_TAG) and $(RELEASE_VERSION) tags"

package-release: docker-build
	@echo "Packaging release artifacts for version $(RELEASE_VERSION)..."
	mkdir -p $(PACKAGE_DIR)
	
	# Linux/Mac package
	rm -rf $(PACKAGE_DIR)/linux
	mkdir -p $(PACKAGE_DIR)/linux/falcon
	# Extract toolchain from versioned Docker image
	docker run --rm $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(RELEASE_VERSION) tar -C /opt/falcon -cf - . | tar -C $(PACKAGE_DIR)/linux/falcon -xf -
	# Copy wrappers over binaries (overwriting them with host wrappers)
	cp packaging/wrappers/linux_mac/*.sh $(PACKAGE_DIR)/linux/falcon/bin/
	for f in $(PACKAGE_DIR)/linux/falcon/bin/*.sh; do [ -f "$$f" ] && mv "$$f" "$${f%.sh}" || true; done
	chmod +x $(PACKAGE_DIR)/linux/falcon/bin/*
	tar -czf $(PACKAGE_DIR)/falcon-$(RELEASE_VERSION)-Linux.tar.gz -C $(PACKAGE_DIR)/linux falcon
	rm -rf $(PACKAGE_DIR)/linux
	
	# Windows package
	rm -rf $(PACKAGE_DIR)/windows
	mkdir -p $(PACKAGE_DIR)/windows/falcon/bin
	cp packaging/wrappers/windows/*.bat $(PACKAGE_DIR)/windows/falcon/bin/
	cd $(PACKAGE_DIR)/windows && zip -r $(PACKAGE_DIR)/falcon-$(RELEASE_VERSION)-win64.zip falcon
	rm -rf $(PACKAGE_DIR)/windows
	
	# Export Docker Image (versioned)
	@echo "Exporting Falcon Docker Image $(RELEASE_VERSION) (this may take a while)..."
	docker save $(DOCKER_REGISTRY)/$(DOCKER_REPO):$(RELEASE_VERSION) | pigz -9 -p 4 > $(PACKAGE_DIR)/falcon-image-$(RELEASE_VERSION).tar.gz
	
	# Also create a symlink for latest
	ln -sf $(PACKAGE_DIR)/falcon-image-$(RELEASE_VERSION).tar.gz $(PACKAGE_DIR)/falcon-image.tar.gz
	
	@echo "✓ Release packages created in $(PACKAGE_DIR):"
	@ls -lh $(PACKAGE_DIR)

publish-release: package-release
	@echo "Publishing release artifacts to GitHub for version $(RELEASE_VERSION)..."
	gh release create $(RELEASE_VERSION) --title "Falcon $(RELEASE_VERSION)" --notes "Release $(RELEASE_VERSION)" || true
	gh release upload $(RELEASE_VERSION) \
		$(PACKAGE_DIR)/falcon-$(RELEASE_VERSION)-Linux.tar.gz \
		$(PACKAGE_DIR)/falcon-$(RELEASE_VERSION)-win64.zip \
		$(PACKAGE_DIR)/falcon-image-$(RELEASE_VERSION).tar.gz \
		packaging/install.sh \
		--clobber
	@echo "✓ Release published to GitHub"


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

docker-teardown: docker-db-stop docker-uninstall-wrappers
	@echo "Removing FAlCon Docker image..."
	-docker rmi $(DOCKER_IMAGE)
	@echo "✓ Teardown complete."

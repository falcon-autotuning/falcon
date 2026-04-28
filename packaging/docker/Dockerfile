FROM ubuntu:24.04

# Install necessary build dependencies
RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  bison \
  flex \
  clang \
  pkg-config \
  git \
  curl \
  zip \
  unzip \
  tar \
  linux-libc-dev \
  python3-dev \
  python3 \
  python3-venv \
  autoconf \
  autoconf-archive \
  automake \
  libtool \
  ninja-build \
  lld \
  coreutils \
  mono-complete && \
  rm -rf /var/lib/apt/lists/*

# Install uv - correct installation for Ubuntu
RUN curl -LsSf https://astral.sh/uv/install.sh | sh
ENV PATH="/root/.local/bin:${PATH}"

# Set up your workspace
COPY . /src
WORKDIR /src

# Verify uv is available
RUN which uv && uv --version

# runs the make install-deps command that builds all of the dependencies
ENV SUDO=""
RUN make check-vcpkg

# runs the make install command that builds the current package and installs it to /opt/falcon/
RUN make install && \
  mv /opt/falcon /opt/falcon-seed && \
  mkdir -p /opt/falcon

# Set the environment variables
ENV PATH="/opt/falcon/bin:${PATH}" \
  LD_LIBRARY_PATH="/opt/falcon/lib:${LD_LIBRARY_PATH:-}" \
  PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

RUN ls -lR /opt/falcon
# Run the DSL smoke test to ensure the environment is functional
RUN /bin/bash /src/packaging/docker/run-smoke-test.sh

# Setup the entrypoint
COPY packaging/docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

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
RUN make build-release 

# Build and install explicitly to /opt/falcon
RUN cmake --install build --prefix /opt/falcon

# Set the environment variables
ENV PATH="/opt/falcon/bin:$PATH"
ENV LD_LIBRARY_PATH="/opt/falcon/lib:$LD_LIBRARY_PATH"
ENV PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:$PKG_CONFIG_PATH"

# Verify again after PATH is set
# RUN which falcon-run

# Run the DSL smoke test
# RUN /bin/bash /src/packaging/docker/run-smoke-test.sh

# Setup the entrypoint
COPY packaging/docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

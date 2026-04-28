# Build stage
FROM ubuntu:24.04 AS builder

# Install uv 
RUN curl -LsSf https://astral.sh/uv/install.sh | sh
RUN mkdir /venv && uv venv --python 3.12 /venv && uv pip install --python /venv/bin/python 'qarray>=1.6.0' 'numpy>=1.24.0' 'pyyaml>=6.0'



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

# Set up workspace
COPY . /src
WORKDIR /src

# Build and install
ENV SUDO=""
RUN make build-release && \
  cmake --install build --prefix /opt/falcon && \
  make clean

# Runtime stage
FROM ubuntu:24.04

# Install only runtime dependencies
RUN apt-get update && apt-get install -y \
  libssl3 \
  libcurl4 \
  && rm -rf /var/lib/apt/lists/*

# Copy installed binaries and libraries from builder
COPY --from=builder /opt/falcon /opt/falcon

RUN apt-get update && apt-get install -y \
  clang \
  pkg-config \
  git \
  curl \
  zip \
  unzip \
  tar \
  python3-dev \
  python3 \
  python3-venv \
  libtool \
  lld \
  coreutils \
  libssl3 \
  libcurl4 \
  libc6 \
  && rm -rf /var/lib/apt/lists/*

# Set the environment variables
ENV PATH="/root/.local/bin:/opt/falcon/bin:$PATH"
ENV LD_LIBRARY_PATH="/opt/falcon/lib:$LD_LIBRARY_PATH"
ENV PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:$PKG_CONFIG_PATH"

# Copy smoke test script and run it to verify the runtime environment
COPY packaging/docker/run-smoke-test.sh /tmp/run-smoke-test.sh
RUN chmod +x /tmp/run-smoke-test.sh && \
  /bin/bash /tmp/run-smoke-test.sh && \
  rm /tmp/run-smoke-test.sh

# Clean up unnecessary files
RUN rm -rf \
  /var/log/* \
  /var/cache/* \
  /var/tmp/* \
  /tmp/* \
  /usr/share/doc/* \
  /usr/share/man/* \
  /usr/share/info/*

# Setup the entrypoint
COPY packaging/docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

# Build stage
FROM ubuntu:24.04 AS builder
# Set shell to bash and enable pipefail
SHELL ["/bin/bash", "-o", "pipefail", "-c"]
# Install necessary build dependencies
RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  bison \
  flex \
  pkg-config \
  git \
  gcc \
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
# Install uv 
RUN curl -LsSf https://astral.sh/uv/install.sh | sh
ENV PATH="/root/.local/bin:$PATH"
# qarray appears to only depend on pandas, matplotlib, tqdm, dash,and plotly for gui and examples, which we don't need
RUN mkdir -p /opt/falcon/venv && \
  uv venv --python 3.12 /opt/falcon/venv && \
  uv pip install --python /opt/falcon/venv/bin/python \
  'qarray>=1.6.0' \
  'numpy>=1.24.0' \
  'pyyaml>=6.0'
ENV VENV_DIR=/opt/falcon/venv
ENV PYTHON_EXECUTABLE=${VENV_DIR}/bin/python
# Set up workspace
COPY . /src
WORKDIR /src

# Build and install
ENV SUDO=""
RUN make install PRESET=linux-gcc-release

# Runtime stage
FROM ubuntu:24.04 AS release
# Copy installed binaries and libraries from builder
COPY --from=builder /opt/falcon /opt/falcon
COPY --from=builder /opt/falcon /opt/falcon-seed
RUN apt-get update && apt-get install -y \
  git \
  tar \
  gcc \
  libssl3 \
  libcurl4 \
  libc6 \
  && rm -rf /var/lib/apt/lists/*
# Set the environment variables
ENV PATH="/root/.local/bin:/opt/falcon/bin:$PATH"
ENV LD_LIBRARY_PATH="/opt/falcon/lib:$LD_LIBRARY_PATH"
# Copy smoke test script and run it to verify the runtime environment
COPY packaging/docker/run-smoke-test.sh /tmp/run-smoke-test.sh
RUN /bin/bash /tmp/run-smoke-test.sh && \
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

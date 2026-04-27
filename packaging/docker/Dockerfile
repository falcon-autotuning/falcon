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

# Install vcpkg
ENV VCPKG_ROOT=/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT && \
  $VCPKG_ROOT/bootstrap-vcpkg.sh

# Build arguments for NuGet authentication
ARG NUGET_FEED_URL
ARG NUGET_API_KEY

# runs the make install-deps command that builds all of the dependencies
ENV SUDO=""
RUN make install-vcpkg-deps 

# runs the make install command that builds the current package and installs it to /opt/falcon/
RUN make install && \
  mv /opt/falcon /opt/falcon-seed && \
  mkdir -p /opt/falcon

# Set the environment variables
ENV PATH="/opt/falcon/bin:/opt/falcon/lib:${PATH}"
ENV LD_LIBRARY_PATH="/opt/falcon/lib"
ENV PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig"

# Run the DSL smoke test to ensure the environment is functional
RUN /bin/bash /src/packaging/docker/run-smoke-test.sh

# Setup the entrypoint
COPY packaging/docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["entrypoint.sh"]

# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.134.0/containers/cpp/.devcontainer/base.Dockerfile
ARG VARIANT="ubuntu"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# [Optional] Uncomment this section to install additional packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
     && apt-get -y install --no-install-recommends clang-format clang tidy

RUN wget https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz \
    tar zxfv boost_1_75_0.tar.gz \
    cd boost_1_75_0 \
    ./bootstrap.sh --prefix=/usr/ \
    ./b2 --with=all -j 16 install 
img1a=ghcr.io/tst-race/race-in-the-box/race-in-the-box:add-ci
img1b=ghcr.io/tst-race/race-core/race-sdk:main-x64host-linux-aarch64

# Unnecessary if race-in-the-box and race-sdk images are public
if [ $# -gt 0 ]; then
    docker login ghcr.io -u $1 -p $2
fi
docker pull $img1a && \
    docker pull $img1b && \
    ./configure && \
    make src/artifactory DOCKERFILE_THREADS=2 && \
    rm -rf ./kit && \
    cp -r ./src/artifactory/kestrel-*-artifactory/race/public.staging.1.*/ta1/carma/latest/ ./kit


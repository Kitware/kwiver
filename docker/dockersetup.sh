#
# Setup script for KWIVER docker 
# Script must run in /src/docker/ directory for pathing purposes
# Script sets up a mounted shared volume in the folder /src/docker/shared 
#

docker build -t kwiver:1.2.0 .
docker run -td kwiver:1.2.0 -v /shared:/KWIVER/shared /bin/bash




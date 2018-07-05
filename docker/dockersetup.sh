# Kitware 2018:
#
# Setup and run script for KWIVER docker 1.2.0
# Script must run in /src/docker/ directory for pathing purposes
#
# Script optionally sets up a mounted shared volume between host/docker


# Initial setup
docker build -t kwiver:1.2.0 .

# Start without shared volume (default)
docker run -it kwiver:1.2.0 /bin/bash

#	OR

# Start with shared volume
# **This will create a shared folder on both host/docker in /SharedKIWVER/KWIVER1.2.0
#docker run -it -v /SharedKWIVER/KWIVER1.2.0:/SharedKWIVER/KWIVER1.2.0 kwiver:1.2.0 /bin/bash




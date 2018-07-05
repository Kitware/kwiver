# Kitware 2018:
#
<<<<<<< HEAD
# Setup and run script for KWIVER docker 1.2.0
# Script must run in /src/docker/ directory for pathing purposes
#
# Script optionally sets up a mounted shared volume between host/docker
=======
# Setup and run script for KWIVER docker 1.2.0  
# Script must run in /src/docker/ directory for pathing purposes
# 
# Script optionally sets up a mounted shared volume between host/docker 
# 	*Permission may have to be granted to docker to create a folder in /Shared
>>>>>>> cd6b14ff177bbde9ec4dd7b80f11a42ff1ed6e19


# Initial setup
docker build -t kwiver:1.2.0 .

# Start without shared volume (default)
docker run -it kwiver:1.2.0 /bin/bash

#	OR

<<<<<<< HEAD
# Start with shared volume
# **This will create a shared folder on both host/docker in /SharedKIWVER/KWIVER1.2.0
#docker run -it -v /SharedKWIVER/KWIVER1.2.0:/SharedKWIVER/KWIVER1.2.0 kwiver:1.2.0 /bin/bash
=======
# Start with shared volume 
# **This will create a shared folder on both host/docker in /Shared/KWIVER1.2.0
#docker run -it -v /Shared/KWIVER1.2.0:/Shared/KWIVER1.2.0 kwiver:1.2.0 /bin/bash
>>>>>>> cd6b14ff177bbde9ec4dd7b80f11a42ff1ed6e19




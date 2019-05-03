# if MIE4NITF is enabled, we need to have LibXml2 as well.

if (KWIVER_ENABLE_GDAL)
  # Optionally find and configure LibXml2 dependency
  find_package( LibXml2 REQUIRED )
  include_directories(SYSTEM ${LIBXML2_INCLUDE_DIR})
endif (KWIVER_ENABLE_GDAL)

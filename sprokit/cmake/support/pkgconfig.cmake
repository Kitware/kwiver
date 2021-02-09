function (sprokit_configure_pkgconfig module)
  if (UNIX)
    set(pkgconfig_file "${sprokit_binary_dir}/${CMAKE_INSTALL_LIBDIR}/pkgconfig/${module}.pc")

    sprokit_configure_file(sprokit-${module}.pc
      "${CMAKE_CURRENT_SOURCE_DIR}/${module}.pc.in"
      "${pkgconfig_file}"
      KWIVER_VERSION
      CMAKE_INSTALL_PREFIX
      CMAKE_INSTALL_LIBDIR
      ${ARGN})

    sprokit_install(
      FILES       "${pkgconfig_file}"
      DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
      COMPONENT   development)
  endif ()
endfunction ()

# external dependency for kpf i/o

if(DEFINED fletch_ENABLED_YAMLCPP)
  set(_default ${fletch_ENABLED_YAMLCPP})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_KPF
  "Enable kpf i/o for vital types"
  ${_default}
)
unset(_default)
# Mark this as advanced
mark_as_advanced( KWIVER_ENABLE_KPF )
if (KWIVER_ENABLE_KPF )
  add_definitions( -DKWIVER_ENABLE_KPF )
endif()

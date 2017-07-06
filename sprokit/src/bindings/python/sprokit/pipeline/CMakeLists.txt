project(sprokit_python_sprokit_pipeline)

# Because of the way pybind11 handles reused types
# Each submodule has to have its own directory
sprokit_create_python_init(sprokit/pipeline
)
_sprokit_add_python_library(config
  sprokit/pipeline/config
  config.cxx)
target_link_libraries(python-sprokit.pipeline.config-config
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/config
  config
)

_sprokit_add_python_library(datum
  sprokit/pipeline/datum
  datum.cxx)
target_link_libraries(python-sprokit.pipeline.datum-datum
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/datum
  datum
)

_sprokit_add_python_library(edge
  sprokit/pipeline/edge
  edge.cxx)
target_link_libraries(python-sprokit.pipeline.edge-edge
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/edge
  edge
)

_sprokit_add_python_library(modules
  sprokit/pipeline/modules
  modules.cxx)
target_link_libraries(python-sprokit.pipeline.modules-modules
  PRIVATE             sprokit_pipeline
                      sprokit_python_util
                      ${Boost_SYSTEM_LIBRARY}
                      vital_vpm
  )

sprokit_create_python_init(sprokit/pipeline/modules
  modules
)

_sprokit_add_python_library(pipeline
  sprokit/pipeline/pipeline
  pipeline.cxx)
target_link_libraries(python-sprokit.pipeline.pipeline-pipeline
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/pipeline
  pipeline
)

_sprokit_add_python_library(process
  sprokit/pipeline/process
  process.cxx)
target_link_libraries(python-sprokit.pipeline.process-process
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/process
  process
)

_sprokit_add_python_library(process_cluster
  sprokit/pipeline/process_cluster
  process_cluster.cxx)
target_link_libraries(python-sprokit.pipeline.process_cluster-process_cluster
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/process_cluster
  process_cluster
)

_sprokit_add_python_library(process_factory
  sprokit/pipeline/process_factory
  process_factory.cxx)
target_link_libraries(python-sprokit.pipeline.process_factory-process_factory
  PRIVATE             sprokit_pipeline
                      sprokit_python_util
                      vital_vpm
    )

sprokit_create_python_init(sprokit/pipeline/process_factory
  process_factory
)

_sprokit_add_python_library(scheduler
  sprokit/pipeline/scheduler
  scheduler.cxx)
target_link_libraries(python-sprokit.pipeline.scheduler-scheduler
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/scheduler
  scheduler
)

_sprokit_add_python_library(scheduler_factory
  sprokit/pipeline/scheduler_factory
  scheduler_factory.cxx)
target_link_libraries(python-sprokit.pipeline.scheduler_factory-scheduler_factory
  PRIVATE             sprokit_pipeline
                      sprokit_python_util
                      vital_vpm
  )

sprokit_create_python_init(sprokit/pipeline/scheduler_factory
  scheduler_factory
)

_sprokit_add_python_library(stamp
  sprokit/pipeline/stamp
  stamp.cxx)
target_link_libraries(python-sprokit.pipeline.stamp-stamp
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/stamp
  stamp
)

_sprokit_add_python_library(utils
  sprokit/pipeline/utils
  utils.cxx)
target_link_libraries(python-sprokit.pipeline.utils-utils
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/utils
  utils
)

_sprokit_add_python_library(version
  sprokit/pipeline/version
  version.cxx)
target_link_libraries(python-sprokit.pipeline.version-version
  PRIVATE             sprokit_pipeline
                      sprokit_python_util)

sprokit_create_python_init(sprokit/pipeline/version
  version
)

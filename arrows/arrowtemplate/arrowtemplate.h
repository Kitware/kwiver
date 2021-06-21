/**
 * This header declares and registers a 'hello_kitware' function with the kwiver build system that prints "Hello Kitware!".
 * The macros that are used to register the function are generated during the build. In general it will be named "kwiver_algo_<NAME OF ARROW>_export.h"
 * 
 */
#pragma once

#include <arrows/arrowtemplate/kwiver_algo_arrowtemplate_export.h>
#include <stdio.h>

namespace kwiver {
    namespace arrows{
        namespace ArrowTemplate {
            // This macro is used to register the hello_kitware function with kwiver
            KWIVER_ALGO_ARROWTEMPLATE_EXPORT
            void hello_kitware();
        }
    }
}
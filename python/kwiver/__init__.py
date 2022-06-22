import logging
import os

_LOGGING_ENVIRON_VAR = "KWIVER_PYTHON_DEFAULT_LOG_LEVEL"


def _logging_onetime_init() -> None:
    """
    One-time initialize kwiver-module-scope logging level.

    This will default to the WARNING level unless the environment variable
    "KWIVER_PYTHON_DEFAULT_LOG_LEVEL" is set to a valid case-insensitive value
    (see the map below).

    This does NOT create a logging formatter. This is left to the discretion of
    the application.
    """
    if not hasattr(_logging_onetime_init, "called"):
        # Pull logging from environment variable if set
        llevel = logging.WARN
        if _LOGGING_ENVIRON_VAR in os.environ:
            llevel_str = os.environ[_LOGGING_ENVIRON_VAR].lower()
            # error warn info debug trace
            m = {"error": logging.ERROR,
                 "warn": logging.WARN,
                 "info": logging.INFO,
                 "debug": logging.DEBUG,
                 "trace": 1}
            if llevel_str in m:
                llevel = m[llevel_str]
            else:
                logging.getLogger("kwiver").warning(
                    f"KWIVER python logging level value set but did not match "
                    f"a valid value. Was given: \"{llevel_str}\". "
                    f"Must be one of: {list(m.keys())}. Defaulting to warning "
                    f"level.")
        logging.getLogger(__name__).setLevel(llevel)
        # Mark this one-time logic as invoked to mater calls are idempotent.
        _logging_onetime_init.called = True
    else:
        logging.getLogger(__name__).debug(
            "Logging one-time setup already called, doing nothing."
        )


_logging_onetime_init()

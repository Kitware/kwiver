#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

pytest --cov=vital --cov-report=term "$@" ${SCRIPT_DIR}/vital/tests/

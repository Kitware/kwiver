#!/bin/bash
#
# Run the uncrustify tool against repository C++ files.
# Assumes a unix environment.
#
# The --check option may be provided to instead run uncrustify in check-mode,
# which doesn't modify files but instead exits with a non-zero code if any
# files *would* be changed. Files that would be changed are output to STDERR:
#     $ bash unix_uncrustify.sh 1>/dev/null
#
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(realpath "$SCRIPT_DIR"/..)"

cd "$ROOT_DIR" || exit

# If we should run uncrustify in to CHECK files instead of modifying them.
CHECK_MODE=0

while [[ $# -gt 0 ]]
do
  key="$1"
  shift  # past key
  case ${key} in
    --check)
      CHECK_MODE=1
    ;;
    *)  # Anything else (wildcard)
      echo "Received unknown parameter: \"$key\""
      exit 1
    ;;
  esac
done


# Directories relative to the root directory to search for C++ files for
# uncrustification.
CPP_SOURCE_DIRS=(
  "CMake"
  "python"
  "tests"
  "vital"
)

# NOTE: This subtractive grep usage to filter out certain places we chose not to
# enforce style requirements on.
mapfile -d "" CPP_FILES \
  < <(find "${CPP_SOURCE_DIRS[@]}" \
      -type f -regextype egrep -iregex ".*\.(h|cxx|txx)" -print0 \
      | grep -zv "^vital/kwiversys" )

## Debug file selection
#for F in $(echo "${CPP_FILES[@]}" | sort)
#do
#  echo "- $F"
#done

# This call may be adapted to only "check" for files that **would** be modified
# by replacing the ``--no-backup --replace`` options with ``--check``.
if [[ ${CHECK_MODE} ]]
then
  MODE_ARGS="--check"
else
  MODE_ARGS="--no-backup --replace"
fi
# shellcheck disable=SC2086
uncrustify -c "$ROOT_DIR"/.uncrustify.cfg -lCPP ${MODE_ARGS} "${CPP_FILES[@]}"

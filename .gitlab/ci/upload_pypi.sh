#!/bin/bash

# Upload Python wheels to GitLab PyPI

set -e

dnf install -y --setopt=install_weak_deps=False twine curl jq

gitlab_api_url="https://gitlab.kitware.com/api/v4/projects/$CI_PROJECT_ID"

# Only remove old dev packages if we're not building from a tag
if [ -z "$CI_COMMIT_TAG" ]; then
  echo "Removing old development packages..."
  packages=$(curl --silent --header "PRIVATE-TOKEN: $CI_JOB_TOKEN" \
    "$gitlab_api_url/packages?package_type=pypi&order_by=created_at&sort=desc&per_page=100")

  dev_packages=$(echo "$packages" | jq -r '.[] | select(.version | test("\\.dev[0-9]+")) | "\(.version)|\(.id)"')

  if [ -n "$dev_packages" ]; then
    echo "$dev_packages" | while IFS='|' read -r version pkg_id; do
      echo "Deleting Version: $version, ID: $pkg_id"
      curl --silent --request DELETE --header "PRIVATE-TOKEN: $CI_JOB_TOKEN" \
        "$gitlab_api_url/packages/$pkg_id"
      echo "  Successfully deleted package ID: $pkg_id"
    done
  else
    echo "No dev packages found"
  fi
fi

ls dist
twine upload -u gitlab-ci-token -p "$CI_JOB_TOKEN" --repository-url "$gitlab_api_url/packages/pypi" dist/*

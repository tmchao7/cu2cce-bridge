#!/bin/bash
# Compatibility entrypoint. The canonical shared environment helper is now
# tools/virtual_toolkit_env.sh; source it directly for new integrations.
# This file contains no separate implementation and is intentionally kept as
# a regular script (not a symlink) for existing callers.
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)/virtual_toolkit_env.sh"

#!/bin/fish
if test -n "$IDF_PATH"
  return 0
end

export IDF_PATH=$HOME/esp/esp-idf
. $HOME/esp/esp-idf/export.fish

#https://github.com/espressif/idf-component-manager/issues/40
#https://github.com/espressif/idf-component-manager#environment-variables
export IDF_COMPONENT_OVERWRITE_MANAGED_COMPONENTS=1
#!/bin/bash
if [ -v IDF_PATH ]; then
  return 0
fi

export IDF_PATH=$HOME/esp/esp-idf
. $HOME/esp/esp-idf/export.sh

#https://github.com/espressif/idf-component-manager/issues/40
#https://github.com/espressif/idf-component-manager#environment-variables
export IDF_COMPONENT_OVERWRITE_MANAGED_COMPONENTS=1
############### Add include ###################
list(APPEND ADD_INCLUDE "include")
###############################################

############ Add source files #################
append_srcs_dir(ADD_SRCS "src")
###############################################

###### Add required/dependent components ######
# Hello World 只使用 MaixCDK 的基础组件，后续按需增加 vision、peripheral 等组件。
list(APPEND ADD_REQUIREMENTS basic)
###############################################

register_component()
#include "src/commands/add_node.cpp"
#include "src/commands/extract.cpp"
#include "src/commands/set_property.cpp"
#include "src/commands/set_tag.cpp"
#include "src/commands/delete_tag.cpp"
#include "src/commands/copy_node.cpp"
#include "src/commands/delete_node.cpp"

#include "src/filters/base_node_filters.cpp"
#include "src/filters/contain.cpp"
#include "src/filters/distance.cpp"
#include "src/filters/distance_on_axis.cpp"
#include "src/filters/intersect.cpp"
#include "src/filters/node.cpp"
#include "src/filters/tag_select.cpp"
#include "src/filters/volume.cpp"

#include "src/cliproxy.cpp"
#include "src/command.cpp"
#include "src/command_table.cpp"
#include "src/common.cpp"
#include "src/drawer.cpp"
#include "src/filter.cpp"
#include "src/filter_input.cpp"
#include "src/filter_table.cpp"
#include "src/logger.cpp"
#include "src/mat.cpp"
#include "src/relation.cpp"
#include "src/scene.cpp"
#include "src/serialize.cpp"
#include "src/sgnode.cpp"
#include "src/sgnode_algs.cpp"
#include "src/soar_interface.cpp"
#include "src/svs.cpp"
#include "src/timer.cpp"

global env
set env(LD_LIBRARY_PATH) "/Applications/Soar/lib"
set env(DYLID_LIBRARY_PATH) "/Applications/Soar/lib"
set env(TCLLIBPATH) "/Applications/Soar/lib"
set auto_path "$auto_path /Applications/Soar/lib"

set argv ""
cd /Applications/Soar/soar-library
source /Applications/Soar/tclenvironments/eaters/init-eaters.tcl
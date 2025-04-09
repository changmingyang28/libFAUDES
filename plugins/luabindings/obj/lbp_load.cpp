/* Auto-generated file to load all lua modules  */

/* 
The build system appends the definition of the
load function by adding  a function call for for each plugin.
*/

#include "lbp_load.h"

// over all load
int luaopen_faudes_allplugins(lua_State* L) 
{
luaopen_faudes(L); 
 luaopen_synthesis(L); 
 luaopen_observer(L); 
 luaopen_multitasking(L); 
 luaopen_diagnosis(L); 
 luaopen_hiosys(L); 
 luaopen_iosystem(L); 
 luaopen_coordinationcontrol(L); 

return 0;
}

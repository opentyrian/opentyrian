#include "opentyr.h"
#include "nortvars.h"

/* File constants for Saving ShapeFile */
const JE_byte NV_shapeactive   = 0x01;
const JE_byte NV_shapeinactive = 0x00;

JE_boolean ScanForJoystick;
JE_word z, y;
JE_boolean InputDetected;
JE_word LastMouseX, LastMouseY;

/*Mouse Data*/  /*Mouse_Installed is in VGA256d*/
JE_byte MouseCursor;
JE_boolean Mouse_ThreeButton;
JE_word MouseX, MouseY, MouseButton;

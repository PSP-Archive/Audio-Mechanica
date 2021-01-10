#ifndef RAM_INCLUDED 
#define RAM_INCLUDED 

/* RAM simple check functions header */ 


// *** INCLUDES *** 

#include <psptypes.h> 
#include <malloc.h> 


// *** DEFINES *** 

#define RAM_BLOCK      (1024 * 1024) 


// *** FUNCTIONS DECLARATIONS *** 

u32 ramAvailableLineareMax (void); 
u32 ramAvailable (void); 

#endif

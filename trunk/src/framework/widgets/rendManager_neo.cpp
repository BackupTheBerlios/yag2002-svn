
#include <ctd_frbase.h>

#include "rendManager_neo.h"
#include "Main\mainGG.h"


/**************************************************
Constructor - Construct
**************************************************/
rendManager_Neo::rendManager_Neo()
{
}

bool rendManager_Neo::Init(int value)
{

    return true;
}


/**************************************************
Constructor - Init
**************************************************/
bool rendManager_Neo::ShutDown(int value)
{
    return true;
}

int  rendManager_Neo::Pulse()
{
   
    GG_Pulse();

    return 0;

}


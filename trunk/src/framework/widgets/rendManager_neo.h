/**************************************************
D3D Manager - Initializes and managers D3D Objects
**************************************************/
#ifndef _RENDMANAGER_NEO_H_
#define _RENDMANAGER_NEO_H_

class rendManager_Neo
{
public:

    /***************************************
    Constructor(s) and Destructor
    ***************************************/
    rendManager_Neo();
    ~rendManager_Neo(){};

    /***************************************
    Pulse functions
    ***************************************/
    bool Init       (int a_Value);
    bool ShutDown   (int a_Value);
    int  Pulse();

};


#endif // _RENDMANAGER_NEO_H_

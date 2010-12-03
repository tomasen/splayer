#include "svplib.h"
#include "SVPToolBox.h"

bool SVP_CanUseCoreAvcCUDA(bool useCUDA)
{
    CSVPToolBox svpTool;
    if(useCUDA){
        useCUDA = svpTool.CanUseCUDAforCoreAVC();
    }

    return useCUDA;

}

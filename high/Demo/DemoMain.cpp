#include "examples.h"
#include "console.h"

PNLW_USING

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        printf("\nUsage: .exe [Crop|Asia|Waste|Gaussian|Pigs|DBN|Fraud] \n");
        return 1;
    }

	SetMaximizedWindow();
    
	if (strcmp(argv[1],"Crop") == 0)
        CropDemo();
    else
        if (strcmp(argv[1],"Asia") == 0)
            AsiaDemo();
        else
            if (strcmp(argv[1],"Waste") == 0)
                WasteDemo();
            else
                if (strcmp(argv[1],"Gaussian") == 0)
                    ScalarGaussianBNetDemo();
                else
                    if (strcmp(argv[1],"Pigs") == 0)
                        PigsDemo();
                    else
                        if (strcmp(argv[1],"DBN") == 0)
                            DBNDemo();
                        else
                            if (strcmp(argv[1],"Fraud") == 0)
                                FraudDemo();
                            else
                            {
                                printf("\nUnknown model \n"); 
                                return 1;
                            }
    return 0;
}

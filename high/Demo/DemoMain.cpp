#include "examples.h"

PNLW_USING

int main(int argc, char* argv[])
{

//    Structural();
//    FraudDemo();  

    if (argc <= 1)
    {
        printf("\nUsage: .exe [Asia|Waste|Gaussian|Pigs|DBN] \n");
        return 1;
    }

    if (strcmp(argv[1],"Crop") == 0)
        CropDemo();
    else
/*        if (strcmp(argv[1],"RPS") == 0)
            RPSDemo();
        else
*/
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
//                        KjaerulfsBNetDemo();
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

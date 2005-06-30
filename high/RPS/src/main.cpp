#include <windows.h>
#include <time.h>
#include "cv.h"
#include "highgui.h"
#include "cvcam.h"
#include "RPSGame.h"
#include <string>
#include <iostream>

const int DiffThreshold = 40;
const double mhiDuration = 1500.0;
const double ProcentOfPoints = 0.25;
int BufferSize = 10;
const int MaxBufferSize = 25;
const int AuthenticFrameThreshold = 100;

bool TimeToStopCapture = false;


enum Modes
{
    Undefined, Intro, MoveSelection, MoveShowing, Settings
};

enum Buttons 
{
    NoButton, Start, Exit, RockButton, PaperButton, ScissorsButton, SettingsButton, 
	InfoButton, SpeedButton, ResolutionButton
};

enum Resolutions
{
    r640, r800, r1024
};

void UpdateMHIImages(IplImage *pMHI, IplImage *pGrayMHI, IplImage *pImage, IplImage *pLastImage)
{
    IplImage *pGrayImage = cvCreateImage(cvSize(pImage->width, pImage->height), IPL_DEPTH_8U, 1 );
    IplImage *pGrayImageLast = cvCreateImage(cvSize(pImage->width, pImage->height), IPL_DEPTH_8U, 1 );
    IplImage *pTempDifference = cvCreateImage( cvSize(pImage->width, pImage->height), IPL_DEPTH_8U, 1);
    cvCvtColor( pImage, pGrayImage, CV_BGR2GRAY );
    cvCvtColor( pLastImage, pGrayImageLast, CV_BGR2GRAY );

    cvAbsDiff( pGrayImage, pGrayImageLast, pTempDifference );

    cvFlip( pTempDifference, pTempDifference , 0 );
    //cvFlip( pGrayImageLast, pGrayImageLast , -1 );

    cvThreshold( pTempDifference, pTempDifference, DiffThreshold, 255, CV_THRESH_BINARY );

    double timestamp = static_cast<double>(clock());

    cvUpdateMotionHistory( pTempDifference, pMHI, timestamp, mhiDuration );

    for (int i = 0; i < pGrayMHI->imageSize; i++)
    {
	if (pMHI->imageData[i*4] || pMHI->imageData[i*4+1] || pMHI->imageData[i*4+2] || pMHI->imageData[i*4+3])
	{
	    pGrayMHI->imageData[i] = static_cast<char>(255);
	}
	else
	{
	    pGrayMHI->imageData[i] = 0;
	};
    };

    if (pGrayImage != 0)
    {
	cvReleaseImage(&pGrayImage);
    };

    if (pGrayImageLast != 0)
    {
	cvReleaseImage(&pGrayImageLast);
    };
    
    if (pTempDifference != 0)
    {
	cvReleaseImage(&pTempDifference);
    };
};

Buttons GetButton(IplImage *pGrayMHI, Modes Mode, int SubImageWidth, int SubImageHeight, int HStepLength, int VStepLength, IplImage *pImage)
{
    CvRect RectROI;
    bool bStart;
    bool bExit;
    bool bSettings;

    bool bRock;
    bool bPaper;
    bool bScissors;

    bool bInfo;
    bool bSpeed;
    bool bResolution;

    if (Mode == Intro)
    {
	//Check sequentially start and exit button
	//start
	RectROI = cvRect(HStepLength, /*pGrayMHI->height - SubImageHeight - VStepLength*/SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bStart = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	//settings
	RectROI = cvRect(HStepLength*2 + SubImageWidth, /*pGrayMHI->height - SubImageHeight - VStepLength*/ SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bSettings = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	//exit
	RectROI = cvRect(HStepLength*3 + SubImageWidth*2, /*pGrayMHI->height - SubImageHeight - VStepLength*/ SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bExit = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	if (bStart && !bExit && !bSettings)
	{
	    return Start;
	};

	if (bExit && !bStart &&!bSettings)
	{
	    return Exit;
	};

	if (bSettings && !bExit && !bStart)
	{
	    return SettingsButton;
	};
    };

    if (Mode == MoveSelection)
    {
	//Check r, p, s, exit
	//Rock
	RectROI = cvRect(HStepLength, /*pGrayMHI->height - SubImageHeight - VStepLength*/SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bRock = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	//Paper
	RectROI = cvRect(HStepLength*2 + SubImageWidth, /*pGrayMHI->height - SubImageHeight - VStepLength*/SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bPaper = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	//Scissors
	RectROI = cvRect(HStepLength*3 + SubImageWidth*2, /*pGrayMHI->height - SubImageHeight - VStepLength*/SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bScissors = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	//Exit
	RectROI = cvRect(HStepLength*4 + SubImageWidth*3, /*pGrayMHI->height - SubImageHeight - VStepLength*/SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bExit = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	if (bRock && !bPaper && !bScissors && !bExit)
	{
	    return RockButton;
	};

	if (!bRock && bPaper && !bScissors && !bExit)
	{
	    return PaperButton;
	};

	if (!bRock && !bPaper && bScissors && !bExit)
	{
	    return ScissorsButton;
	};

	if (!bRock && !bPaper && !bScissors && bExit)
	{
	    return Exit;
	};
    };
    if (Mode == Settings)
    {
	//Info
	RectROI = cvRect(HStepLength, /*pGrayMHI->height - SubImageHeight - VStepLength*/SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bInfo = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	//Resolution
	RectROI = cvRect(HStepLength*2 + SubImageWidth, /*pGrayMHI->height - SubImageHeight - VStepLength*/SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bResolution = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	//Response
	RectROI = cvRect(HStepLength*3 + SubImageWidth*2, /*pGrayMHI->height - SubImageHeight - VStepLength*/SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bSpeed = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	//Exit
	RectROI = cvRect(HStepLength*4 + SubImageWidth*3, /*pGrayMHI->height - SubImageHeight - VStepLength*/SubImageHeight + VStepLength, SubImageWidth, SubImageHeight);
	cvSetImageROI(pGrayMHI, RectROI);
	bExit = cvCountNonZero(pGrayMHI) >= ProcentOfPoints*SubImageWidth*SubImageHeight;
	cvResetImageROI(pGrayMHI);

	if (bInfo && !bResolution && !bSpeed && !bExit)
	{
	    return InfoButton;
	};

	if (!bInfo && bResolution && !bSpeed && !bExit)
	{
	    return ResolutionButton;
	};

	if (!bInfo && !bResolution && bSpeed && !bExit)
	{
	    return SpeedButton;
	};

	if (!bInfo && !bResolution && !bSpeed && bExit)
	{
	    return Exit;
	};
    };

    return NoButton;
};

void Process(CvCapture* pCapture)
{
    int ComputerWon = 0;
    int HumanWon = 0;

    Modes Mode = Intro;

    IplImage *pRock = cvLoadImage( "..\\rps\\pix\\rock.bmp", 1 );
    IplImage *pPaper = cvLoadImage( "..\\rps\\pix\\paper.bmp", 1);
    IplImage *pScissors = cvLoadImage( "..\\rps\\pix\\scissors.bmp", 1);
    IplImage *pExit = cvLoadImage( "..\\rps\\pix\\exit.bmp", 1);
    IplImage *pStart = cvLoadImage( "..\\rps\\pix\\start.bmp", 1 );
    IplImage *pSettings = cvLoadImage( "..\\rps\\pix\\settings.bmp", 1 );
    IplImage *pInfo = cvLoadImage( "..\\rps\\pix\\info.bmp", 1);
    IplImage *pResolution = cvLoadImage( "..\\rps\\pix\\resolution.bmp", 1 );
    IplImage *pSpeed = cvLoadImage( "..\\rps\\pix\\speed.bmp", 1 );

    cvFlip( pRock, NULL, 0);
    cvFlip( pPaper, NULL, 0);
    cvFlip( pScissors, NULL, 0);
    cvFlip( pExit, NULL, 0);
    cvFlip( pStart, NULL, 0);
    cvFlip( pSettings, NULL, 0);
    cvFlip( pInfo, NULL, 0);
    cvFlip( pResolution, NULL, 0);
    cvFlip( pSpeed, NULL, 0);

    //Sizes of the full image
    int Width = 0;
    int Height = 0;

    //Sizes of button for the case of mode = MoveSelection
    int WidthButtonSelection;
    int HeightButtonSelection;
    int StepLengthSelection;
    int StepLengthIntro;

    //MHI images: pBlackWhiteMHI is black and white MHI for internal purposes
    IplImage * pBlackWhiteMHI = 0;
    //MHI image
    IplImage * pMHI = 0;

    IplImage *pLastImage = 0;

    Modes *BufferModes = new Modes[MaxBufferSize];
    Buttons *BufferButtons = new Buttons[MaxBufferSize];
    int LastIndex = -1;

    for (int index = 0; index < MaxBufferSize; index++)
    {
	BufferModes[index] = Undefined;
	BufferButtons[index] = NoButton;
    };

    int FrameCounter = 0;
    int FrameToSave = 0;

    RPSMove ComputerMove;
    RPSMove HumanMove;

    //Current distribution
    float fPaper;
    float fRock;
    float fScissors;

    SetRandomDistribution();

    bool NeedToShowDistribution = false;
    Resolutions Resolution = r640;
    bool RecreateImages = false;

    //Set the current distribution
    GetCurrentDistribution(fRock, fPaper, fScissors);

    CvFont font;
    cvInitFont( &font, CV_FONT_HERSHEY_SIMPLEX , 1.0f, 1.0f, 0.0f, 3);

    while (!TimeToStopCapture)
    {
        IplImage* pImage2;
        IplImage* pImage;

        if( !cvGrabFrame( pCapture ))
	{
	    TimeToStopCapture = true;
	    continue;
	};

        pImage2 = cvRetrieveFrame( pCapture );

	switch (Resolution)
	{
	case r640:
	    pImage = cvCreateImage( cvSize( 640,480 ), pImage2->depth, pImage2->nChannels );
	    break;
	case r800:
	    pImage = cvCreateImage( cvSize( 800,600 ), pImage2->depth, pImage2->nChannels );
	    break;
	case r1024:
	    pImage = cvCreateImage( cvSize( 1024,768 ), pImage2->depth, pImage2->nChannels );
	    break;
	default:
	    pImage = cvCreateImage( cvSize( 640,480 ), pImage2->depth, pImage2->nChannels );
	    break;
	};
        pImage->origin = 1;

        cvResize(pImage2, pImage);
        //cvResize(pImage, pImage);
        
	cvFlip(pImage, 0, 1);

	if (!pMHI)
	{
	    Width = pImage->width;
	    Height = pImage->height;

	    WidthButtonSelection = pRock->width;
	    HeightButtonSelection = pRock->height;
	    StepLengthSelection = (Width - 4*WidthButtonSelection) / 5;
	    StepLengthIntro = (Width - 3*WidthButtonSelection) / 4;
	    
	    pMHI = cvCreateImage( cvSize( Width,Height ), IPL_DEPTH_32F, 1 );	
	    cvZero( pMHI );

	    pBlackWhiteMHI = cvCreateImage( cvSize( Width,Height ), IPL_DEPTH_8U, 1 );
	    cvZero( pBlackWhiteMHI );

	    pLastImage = cvCreateImage( cvSize( Width,Height ), IPL_DEPTH_8U, 3 );
	    cvZero( pLastImage );
	};

	UpdateMHIImages(pMHI, pBlackWhiteMHI, pImage, pLastImage);

	cvCopy(pImage, pLastImage);

	CvRect RectROI;

	if (FrameCounter >= AuthenticFrameThreshold)
	{
	    char buffer[256];

	    switch (Mode)
	    {
	    case Intro:
		//Show start and exit on the top of the screen 

		//Start
		RectROI = cvRect(StepLengthIntro, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pStart, pImage );
		cvResetImageROI(pImage);

		//Settings
		RectROI = cvRect(StepLengthIntro*2 + WidthButtonSelection, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pSettings, pImage );
		cvResetImageROI(pImage);

		//Exit 
		RectROI = cvRect(StepLengthIntro*3 + WidthButtonSelection*2, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pExit, pImage );
		cvResetImageROI(pImage);

		break;

	    case Settings:
		//Info
		RectROI = cvRect(StepLengthSelection, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pInfo, pImage );
		cvResetImageROI(pImage);

		//Resolution
		RectROI = cvRect(StepLengthSelection*2 + WidthButtonSelection, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pResolution, pImage );
		cvResetImageROI(pImage);

		//Speed
		RectROI = cvRect(StepLengthSelection*3 + WidthButtonSelection*2, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pSpeed, pImage );
		cvResetImageROI(pImage);

		//Exit 
		RectROI = cvRect(StepLengthSelection*4 + WidthButtonSelection*3, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pExit, pImage );
		cvResetImageROI(pImage);

		//Show text information about selected options
		if (true)
		{
		    sprintf(buffer, "Show distribution: %s", (NeedToShowDistribution)?("yes"):("no"));
		    std::string ShowDistribution = buffer;
		    switch (Resolution)
		    {
		    case r640:
			sprintf(buffer, "Resolution: 640*480" );
			break;
		    case r800:
			sprintf(buffer, "Resolution: 800*600" );
			break;
		    case r1024:
			sprintf(buffer, "Resolution: 1024*768" );
			break;
		    default:
			sprintf(buffer, "Resolution: 640*480" );
			break;
		    };
		    std::string Resolution = buffer;
		    sprintf(buffer, "Motion duration (response): %i", BufferSize);
		    std::string MotionDuration = buffer;

		    cvPutText( pImage, ShowDistribution.c_str(), cvPoint(StepLengthSelection, Height - StepLengthSelection/2 - HeightButtonSelection*3+60), &font, CV_RGB(255,255,255) );
		    cvPutText( pImage, Resolution.c_str(), cvPoint(StepLengthSelection, Height - StepLengthSelection/2 - HeightButtonSelection*3+30), &font, CV_RGB(255,255,255) );
		    cvPutText( pImage, MotionDuration.c_str(), cvPoint(StepLengthSelection, Height - StepLengthSelection/2 - HeightButtonSelection*3), &font, CV_RGB(255,255,255) );
		};

		break;


	    case MoveSelection:
	    case MoveShowing:

		//Show r, p, s on the top of the screen 
		//Rock
		RectROI = cvRect(StepLengthSelection, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pRock, pImage );
		cvResetImageROI(pImage);

		//Paper
		RectROI = cvRect(StepLengthSelection*2 + WidthButtonSelection, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pPaper, pImage );
		cvResetImageROI(pImage);

		//Scissors
		RectROI = cvRect(StepLengthSelection*3 + WidthButtonSelection*2, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pScissors, pImage );
		cvResetImageROI(pImage);

		//Exit 
		RectROI = cvRect(StepLengthSelection*4 + WidthButtonSelection*3, Height - StepLengthSelection/2 - HeightButtonSelection, WidthButtonSelection, HeightButtonSelection);
		cvSetImageROI(pImage, RectROI);
		cvCopy( pExit, pImage );
		cvResetImageROI(pImage);

		if ((Mode == MoveSelection)&&(NeedToShowDistribution))
		{
		    sprintf(buffer, "Rock: %.3f", fRock);
		    std::string RockStr = buffer;
		    sprintf(buffer, "Paper: %.3f", fPaper);
		    std::string PaperStr = buffer;
		    sprintf(buffer, "Scissors: %.3f", fScissors);
		    std::string ScissorsStr = buffer;

		    //Show probability distribution
		    cvPutText( pImage, RockStr.c_str(), cvPoint(StepLengthSelection, Height - StepLengthSelection/2 - HeightButtonSelection*3), &font, CV_RGB(255,255,255) );
		    cvPutText( pImage, PaperStr.c_str(), cvPoint(StepLengthSelection, Height - StepLengthSelection/2 - HeightButtonSelection*3+30), &font, CV_RGB(255,255,255) );
		    cvPutText( pImage, ScissorsStr.c_str(), cvPoint(StepLengthSelection, Height - StepLengthSelection/2 - HeightButtonSelection*3+60), &font, CV_RGB(255,255,255) );
		};

		if (Mode == MoveShowing)
		{
		    //Show selected moves on the screen 
		    IplImage *pComputerMove;
		    IplImage *pHumanMove;
		    
		    switch (ComputerMove)
		    {
		    case Rock:
			pComputerMove = pRock;
			break;

		    case Paper:
			pComputerMove = pPaper;
		        break;

		    case Scissors:
		    	pComputerMove = pScissors;
			break;
		    };

		    switch (HumanMove)
		    {
		    case Rock:
			pHumanMove = pRock;
			break;

		    case Paper:
			pHumanMove = pPaper;
		        break;

		    case Scissors:
		    	pHumanMove = pScissors;
			break;
		    };

		    //Human's move
		    RectROI = cvRect(StepLengthSelection*2 + WidthButtonSelection, Height - StepLengthSelection/2 - HeightButtonSelection*3, WidthButtonSelection, HeightButtonSelection);
		    cvSetImageROI(pImage, RectROI);
		    cvCopy( pHumanMove, pImage );
		    cvResetImageROI(pImage);

		    //Computer's move
		    RectROI = cvRect(StepLengthSelection*3 + WidthButtonSelection*2, Height - StepLengthSelection/2 - HeightButtonSelection*3, WidthButtonSelection, HeightButtonSelection);
		    cvSetImageROI(pImage, RectROI);
		    cvCopy( pComputerMove, pImage );
		    cvResetImageROI(pImage);

		    if (FrameCounter - FrameToSave > BufferSize*2)
		    {
			Mode = MoveSelection;

			//Set the current distribution
			GetCurrentDistribution(fRock, fPaper, fScissors);
		    };
		};

		
		//Show the score
		char buffer[256];
		std::string HumanStr = "Human: ";
		itoa(HumanWon, buffer, 10);
		HumanStr += buffer;
		std::string ComputerStr = "Computer: ";
		itoa(ComputerWon, buffer, 10);
		ComputerStr += buffer;

		cvPutText( pImage, HumanStr.c_str(), cvPoint(StepLengthSelection, 30), &font, CV_RGB(255,255,255) );
		cvPutText( pImage, ComputerStr.c_str(), cvPoint(StepLengthSelection*3 + WidthButtonSelection*2, 30), &font, CV_RGB(255,255,255) );

		break;

	    };
	};

	Buttons CurrentButton = GetButton(pBlackWhiteMHI, Mode, WidthButtonSelection, HeightButtonSelection, 
	    (Mode == Intro)?(StepLengthIntro):(StepLengthSelection), StepLengthSelection/2, pImage);

	bool InfoAuthentic = true;

	if (FrameCounter < AuthenticFrameThreshold) 
	{
	    InfoAuthentic = false;
	};

	for (int index = 0; (index < BufferSize)&&(InfoAuthentic); index ++)
	{
	    if (BufferModes[index] != Mode)
	    {
		InfoAuthentic = false;
	    };

	    if (BufferButtons[index] != CurrentButton)
	    {
		InfoAuthentic = false;
	    };
	};

	if (Mode == Intro)
	{
	    if ((CurrentButton == Start) && InfoAuthentic)
	    {
		Mode = MoveSelection;
	    };

	    if ((CurrentButton == SettingsButton) && InfoAuthentic)
	    {
		Mode = Settings;
	    };

	    if ((CurrentButton == Exit) && InfoAuthentic)
	    {
		TimeToStopCapture = true;
	    };
	}
	else
	{
	    if (Mode == MoveSelection)
	    {
		if ((CurrentButton == Exit) && InfoAuthentic)
		{
		    Mode = Intro;
		    ComputerWon = 0; 
		    HumanWon = 0;
		    SetRandomDistribution();
		    GetCurrentDistribution(fRock, fPaper, fScissors);
		    cvZero(pMHI);
		};

		if ((CurrentButton == RockButton) && InfoAuthentic)
		{
		    HumanMove = Rock;
		};
		
		if ((CurrentButton == PaperButton) && InfoAuthentic)
		{
		    HumanMove = Paper;
		};
		
		if ((CurrentButton == ScissorsButton) && InfoAuthentic)
		{
		    HumanMove = Scissors;
		};

		if (((CurrentButton == RockButton)||(CurrentButton == PaperButton)
		    ||(CurrentButton == ScissorsButton)) && InfoAuthentic)
		{    
		    Mode = MoveShowing;
		    ComputerMove = GetNextMove(HumanMove);
		    FrameToSave = FrameCounter;

		    //Who won?
		    if ((ComputerMove == HumanMove+1) || ((ComputerMove == Rock)&&(HumanMove == Scissors)))
		    {
			ComputerWon++;
		    };
		    if ((HumanMove == ComputerMove+1) || ((HumanMove == Rock)&&(ComputerMove == Scissors)))
		    {
			HumanWon++;
		    };
		};
	    }
	    else
		if (Mode == Settings)
		{
		    if ((CurrentButton == Exit) && InfoAuthentic)
		    {
			Mode = Intro;
			ComputerWon = 0; 
			HumanWon = 0;
			SetRandomDistribution();
			GetCurrentDistribution(fRock, fPaper, fScissors);
			cvZero(pMHI);

			RecreateImages = true;
		    };

		    if ((CurrentButton == InfoButton) && InfoAuthentic)
		    {
			NeedToShowDistribution = !NeedToShowDistribution;

			RecreateImages = true;
		    };

		    if ((CurrentButton == SpeedButton) && InfoAuthentic)
		    {
			BufferSize = (BufferSize-1+5)%MaxBufferSize + 1;
			cvZero(pMHI);

			RecreateImages = true;
		    };

		    if ((CurrentButton == ResolutionButton) && InfoAuthentic)
		    {
			switch (Resolution)
			{
			case r640:
			    Resolution = r800;
			    break;
			case r800:
			    Resolution = r1024;
			    break;
			case r1024:
			    Resolution = r640;
			    break;
			};

			RecreateImages = true;
		    };		    
		};
	};

	cvShowImage("RPS Game", pImage);
	//cvShowImage("RPS Game 2", pBlackWhiteMHI);

	LastIndex = (LastIndex + 1)%BufferSize;
	BufferModes[LastIndex] = Mode;
	BufferButtons[LastIndex] = CurrentButton;

	FrameCounter++;

        if (pImage)
        {
	    cvReleaseImage( &pImage );
        };

	if (RecreateImages)
	{
	    if (pMHI)
	    {
		cvReleaseImage( &pMHI );
		pMHI = 0;
	    };
	    
	    if (pBlackWhiteMHI)
	    {
		cvReleaseImage( &pBlackWhiteMHI );
		pBlackWhiteMHI = 0;
	    };

	    if (pLastImage)
	    {
		cvReleaseImage( &pLastImage);
		pLastImage = 0;
	    };

	    RecreateImages = false;
	};

	cvWaitKey(1);
    };

    if (pRock)
    {
	cvReleaseImage( &pRock );
    };

    if (pPaper)
    {
	cvReleaseImage( &pPaper );
    };

    if (pScissors)
    {
	cvReleaseImage( &pScissors );
    };

    if (pExit)
    {
	cvReleaseImage( &pExit );
    };

    if (pStart)
    {
	cvReleaseImage( &pStart );
    };

    if (pMHI)
    {
	cvReleaseImage( &pMHI );
    };

    if (pBlackWhiteMHI)
    {
	cvReleaseImage( &pBlackWhiteMHI );
    };

    if (pLastImage)
    {
	cvReleaseImage( &pLastImage );
    };

    if (BufferModes)
    {
	delete[] BufferModes;
    };

    if (BufferButtons)
    {
	delete[] BufferButtons;
    };

    if (pInfo)
    {
	cvReleaseImage( &pInfo);
    };

    if (pResolution)
    {
	cvReleaseImage(&pResolution);
    };

    if (pSpeed)
    {
	cvReleaseImage(&pSpeed);
    };
};

int main(int argc, char *argv[])
{
    std::cout << "List of connected cameras:\n";
    int ncams = cvcamGetCamerasCount();

    int camera;
    for (camera = 0; camera < ncams; camera++)
    {
	CameraDescription CDesrp;

	cvcamGetProperty(camera, CVCAM_DESCRIPTION, &CDesrp);

	std::cout << camera << ". " <<CDesrp.DeviceDescription << "\n";
    };

    std::cout << "\nDefault camera is 0. \nIf you want to use another camera please run \nthe application with parameter \"camera index\".\n";

    std::cout << "\nTo exit the application please select exit icon\nin the visual menu.\n";

    std::cout << "\nThe application works correct when a folder \n..\\RPS\\pix exists.\n";

    int NumberOfCamera = 0;
    if (argc == 2)
    {
	NumberOfCamera = atoi(argv[1]);
    };

    cvNamedWindow( "RPS Game", 1 );

    //cvNamedWindow( "RPS Game 2", 1 );
    CvCapture* pCapture = cvCaptureFromCAM(NumberOfCamera);

    cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH, 640 );
    cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT, 480);


    Process(pCapture);

    if (pCapture)
    {
	cvReleaseCapture(&pCapture);
    };

    return 0;
};
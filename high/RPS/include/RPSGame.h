enum RPSMove
{
    Rock,
    Paper,
    Scissors
};

void SetRandomDistribution();

RPSMove GetNextMove(RPSMove HumansMove);

void GetCurrentDistribution(float &fRock, float &fPaper, float &fScissors);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                         %%
%%                INTEL CORPORATION PROPRIETARY INFORMATION                %%
%%   This software is supplied under the terms of a license agreement or   %%
%%  nondisclosure agreement with Intel Corporation and may not be copied   %%
%%   or disclosed except in accordance with the terms of that agreement.   %%
%%       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        %%
%%                                                                         %%
%%  File:      learningArHMM.m                                             %%
%%                                                                         %%
%%  Purpose:   Example of learning dynamic graphical model                 %% 
%%                                                                         %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%get dynamic model
dbn = ArHMMCreation;

%generate evidences
nSeries = 30;
maxnumSlices = 10;
nts = floor(rand(1,nSeries)*maxnumSlices) +1;
samples = GenerateSamples(dbn, nts);

%create learning procedure
learnEng = CEmLearningEngineDBNCreate(dbn);

%enter evidences
SetData(learnEng, samples);

%start learning procedure
Learn(learnEng);

%results of learning
cpd=GetFactor(dbn, 1);

matMean0 = GetMatrix(cpd, 'matMean', -1, [0]);
matCov0 = GetMatrix(cpd, 'matCovariance', -1, [0]);

matMean1 = GetMatrix(cpd, 'matMean', -1, [1]);
matCov1 = GetMatrix(cpd, 'matCovariance', -1, [1]);

cpd=GetFactor(dbn, 3);

matMean0 = GetMatrix(cpd, 'matMean', -1, [0]);
matCov0 = GetMatrix(cpd, 'matCovariance', -1, [0]);
matWeights0 = GetMatrix(cpd, 'matWeights', 0, [0]);

matMean1 = GetMatrix(cpd, 'matMean', -1, [1])
matCov1 = GetMatrix(cpd, 'matCovariance', -1, [1]);
matWeights1 = GetMatrix(cpd, 'matWeights', 0, [1]);

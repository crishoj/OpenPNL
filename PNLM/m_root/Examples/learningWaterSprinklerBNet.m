%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                         %%
%%                INTEL CORPORATION PROPRIETARY INFORMATION                %%
%%   This software is supplied under the terms of a license agreement or   %%
%%  nondisclosure agreement with Intel Corporation and may not be copied   %%
%%   or disclosed except in accordance with the terms of that agreement.   %%
%%       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        %%
%%                                                                         %%
%%  File:      learningWaterSprinklerLearning.m                            %%
%%                                                                         %%
%%  Purpose:   Example of learning BNEt                                    %% 
%%                                                                         %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

bnet = WaterSprinklerBNetCreation
nnodes = GetNumberOfNodes(bnet);

%generate random samles
nSamples = 100;
samples = GenerateSamples(bnet, nSamples);
for i = 1:nSamples
    evidence = samples{i};
    %make arbitrary node unobserved
    node = round(rand*(nnodes-1));
    MakeNodeHidden(evidence, node);
end

%create learning engine
eng = CEMLearningEngineCreate(bnet);

%set observations
SetData(eng, samples);

%start learning procedure
Learn(eng);

disp('example of learnng is completed');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                         %%
%%                INTEL CORPORATION PROPRIETARY INFORMATION                %%
%%   This software is supplied under the terms of a license agreement or   %%
%%  nondisclosure agreement with Intel Corporation and may not be copied   %%
%%   or disclosed except in accordance with the terms of that agreement.   %%
%%       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        %%
%%                                                                         %%
%%  File:      WaterSprinklerLearning.m                                    %%
%%                                                                         %%
%%  Purpose:   Creating water sprinkler BNet and run learning on it        %% 
%%                                                                         %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
pnlBNet = pnlExCreateScalarGaussianBNet;
bnet = pnlBNet2bntBNet(pnlBNet);

nnodes = double(GetNumberOfNodes(pnlBNet));

numEv = 5000;
pnlSamples = GenerateSamples(pnlBNet, numEv);

array = 1:double(GetNumberOfNodes(pnlBNet));


for i = 1:numEv
    pnlHiddenNds = GetRandSubset(array);
    ToggleNodeState(pnlSamples{i}, pnlHiddenNds-1);
end

bntSamples = pnlEv2bntEv(pnlSamples);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

pnlJTreeInf = CJTreeInfEngineCreate(pnlBNet);
bntJTreeInf = jtree_inf_engine(bnet);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

maxiter = 5;
epsEM = 0.001;
pnlLearn = CEmLearningEngineCreate(pnlBNet);
SetMaxIterEM(pnlLearn, maxiter);
SetTerminationToleranceEM(pnlLearn, epsEM)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

SetData(pnlLearn, pnlSamples);
Learn(pnlLearn);
[bntBNetNew, bntLL] = learn_params_em( bntJTreeInf, bntSamples, maxiter, epsEM );

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
for i = 1:5
s = struct(bntBNetNew.CPD{i});
s.mean
mat1 = GetMatrix(GetFactor(pnlBNet, i-1), 'matMean')
end


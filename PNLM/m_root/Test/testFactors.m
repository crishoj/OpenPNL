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

%Example of creation simlest condition gaussian network
nnodes = 2;
dag = zeros(2, 2);
dag(1, 2) = 1;

graph = CGraphCreateFromAdjMat(dag);

nodeTypes = cell(1, 2);
nodeTypes{1} = CNodeType(1, 2);
nodeTypes{2} = CNodeType(0, 2);

nodeAss = [0 1];
MD = CModelDomainCreate(nodeTypes, nodeAss);
matrix=[0.5 0.5];
domain = [0];
f1 = CTabularCPDCreate( MD, domain, matrix);
domain = [0 1];
f2 = CGaussianCPDCreate(domain, MD);

mean1 = [0 0];
cov1 = [1 0; 0 1];

mean2 = [-0.1 0.1];
cov2 = [2 0; 0 1];

weights = cell(0);% continuous parents are absent 

dparentsComb = [0];
AllocDistribution(f2, mean1, cov1, 1, weights, dparentsComb);

dparentsComb = [1]
AllocDistribution(f2, mean2, cov2, 1, weights, dparentsComb);

bnet = CBNetCreateByModelDomain(graph, MD);
AttachFactor(bnet, f1);
AttachFactor(bnet, f2);
%check model validity
[res, mess] = IsValid(bnet);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Example of Gausian potential creation
domain = [1];
form = 1;
pot = CGaussianPotentialCreate(MD, domain, form, mean1, cov1 );
%check validity of reated potential
res = IsValid(pot);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%Example of creation gaussian CPD for gausian node which gaussian parents
MD = CModelDomainCreateIfAllTheSame(3, CNodeType(0, 2));
domain = [0 1 2];
CPD = CGaussianCPDCreate(domain, MD);

mean = [0 0];
cov = [1 0; 0 1];
weights = cell(1, 2);
weights{1} = [1 2; 3 4];
weights{2} = [-1 -2; -3 -4];

AllocDistribution(CPD, mean, cov, 1, weights);
%check validity of reated CPD
res = IsValid(CPD);




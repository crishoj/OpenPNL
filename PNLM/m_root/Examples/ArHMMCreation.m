%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                         %%
%%                INTEL CORPORATION PROPRIETARY INFORMATION                %%
%%   This software is supplied under the terms of a license agreement or   %%
%%  nondisclosure agreement with Intel Corporation and may not be copied   %%
%%   or disclosed except in accordance with the terms of that agreement.   %%
%%       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        %%
%%                                                                         %%
%%  File:      ArHMMCreation.m                                             %%
%%                                                                         %%
%%  Purpose:   Creation Auto Regressive Hidden Markov Model                %% 
%%                                                                         %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function dbn = ArHMMCreation

% Make an HMM with autoregressive Gaussian observations (switching AR model)
%   X1 -> X2
%   |     | 
%   v     v
%   Y1 -> Y2 


nnodesPerSlice = 2;
dag = zeros(2*nnodesPerSlice, 2*nnodesPerSlice);
dag(1, 2) = 1;
dag(1, 3) = 1;
dag(2, 4) = 1;
dag(3, 4) = 1;

graph = CGraphCreateFromAdjMat(dag);

nodeTypes = cell(1, 2);
nodeTypes{1} = CNodeType(1, 2);
nodeTypes{2} = CNodeType(0, 1);

nodeAss = [0 1 0 1];
MD = CModelDomainCreate(nodeTypes, nodeAss);
bnet = CBNetCreateWithRandomMatrices(graph, MD);
dbn = CDBNCreate(bnet);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                         %%
%%                INTEL CORPORATION PROPRIETARY INFORMATION                %%
%%   This software is supplied under the terms of a license agreement or   %%
%%  nondisclosure agreement with Intel Corporation and may not be copied   %%
%%   or disclosed except in accordance with the terms of that agreement.   %%
%%       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        %%
%%                                                                         %%
%%  File:      infArHMM.m                                                  %%
%%                                                                         %%
%%  Purpose:   Example of inference (smmothing procedure)                  %% 
%%                                                                         %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

dbn = ArHMMCreation;

%define number of slices 
nSlices = 4;
evidences = cell(1, nSlices);

%let node 1 is observed on the every slice
sliceObsNodes = [1];
MD = GetModelDomain(dbn);

%create random evidences
for i = 1 : nSlices
    evidences{i} = CEvidenceCreateByModelDomain(MD, sliceObsNodes, pnlRand(0,1));
end

%create 1.5 Slice Juncton tree inference
infEng = C1_5SliceJTreeInfEngine(dbn);

%defince procedure type (smoothing)
DefineProcedure(infEng,'ptSmoothing', nSlices);

%enter evidences into engine
EnterEvidence(infEng, evidences);

%start smoothing procedure
Smoothing(infEng);

%define query slice and query nodes
querySlice = floor(rand*nSlices);
if querySlice == 0
    query = [0];
else
    query = [0 2];
end

%JPD on query nodes
MarginalNodes(infEng, query, querySlice);
resPot = GetQueryJPD(infEng);

disp(GetDistributionType(resPot));
mat = GetMatrix(resPot, 'matTable');
disp(mat);
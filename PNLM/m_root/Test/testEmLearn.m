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

function res = testEmLearn(varargin)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  Test leanring engine 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
eps = 0.1;
args = varargin;
if length(args) == 0
    rand('seed', now)
else
    rand('seed', varargin{1});
end

res = 0;
seed = rand('seed');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

nnds =  round(rand*(10))+2;
nedges = round(rand*nnds/3)+1;
ncont = round(rand*(nnds-1));
ntd = round(rand)+2;
ntc = round(rand*2)+1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

pnlBNet = pnlExCreateRandomBNet(nnds, nedges, ncont, ntd, ntc);
pnlBNet = pnlExCreateWaterSprinklerBNet;
bnet = pnlBNet2bntBNet(pnlBNet);

nnodes = double(GetNumberOfNodes(pnlBNet));
bntDnodes = bnet.dnodes;
pnlDnodes = bntDnodes - 1;
numDnodes = numel(bntDnodes);
numCnodes = nnodes - numDnodes;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

numEv = round(rand*(50*nnodes));
pnlSamples = GenerateSamples(pnlBNet, numEv);

if numCnodes > 0 & numDnodes > 0
    array = bnet.dnodes -1;
else
    if numDnodes > 0
        array = bnet.dnodes - 1;
    else
        array = bnet.cnodes - 1;
    end
end 

for i = 1:numEv
    pnlHiddenNds = GetRandSubset(array);
    pnlHiddenNds = [];%!!!!!!!!!!!!! need to be removed
    ToggleNodeState(pnlSamples{i}, pnlHiddenNds);
end

bntSamples = pnlEv2bntEv(pnlSamples);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

pnlJTreeInf = CJTreeInfEngineCreate(pnlBNet);
bntJTreeInf = jtree_inf_engine(bnet);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

maxiter = 5;
epsEM = 0.01;
pnlLearn = CEmLearningEngineCreate(pnlBNet);
SetMaxIterEM(pnlLearn, maxiter);
SetTerminationToleranceEM(pnlLearn, epsEM)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

SetData(pnlLearn, pnlSamples);
Learn(pnlLearn);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

pnlLL = GetCriterionValue(pnlLearn);
[bntBNetNew, bntLL] = learn_params_em( bntJTreeInf, bntSamples, maxiter, epsEM );

res = cmpModels(pnlBNet, bntBNetNew, eps);
disp( 'pnl loglik' );
disp(pnlLL);
disp('bnt logik');
disp(bntLL(end));




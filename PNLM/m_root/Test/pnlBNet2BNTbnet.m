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

function bnet = pnlBNet2bntBNet(pnlBNet)

nnodes = double(GetNumberOfNodes(pnlBNet));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dnodes = [];
j = 1;
for i = 0 : nnodes-1
    varType = GetNodeType(pnlBNet, i);
    if IsDiscrete(varType) == 1
        dnodes(j) = i+1; 
        j = j + 1;
    end
    ns(i+1) = double(GetNodeSize(varType));
   
end
if numel(dnodes) > 0
    cnodes = mysetdiff(1 : nnodes, dnodes);
else
    cnodes = 1:nnodes;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

pnlGraph = GetGraph(pnlBNet);
dag = zeros(nnodes, nnodes);
for i = 0 : nnodes-1
    chld = double(GetChildren(pnlGraph, i)) + 1;
    dag(i+1, chld) = 1;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

bnet = mk_bnet(dag, ns, 'discrete', dnodes );

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

for i=dnodes(:)'
    pnlCPD = GetFactor(pnlBNet, i - 1);
    pnlMatrix = GetMatrix(pnlCPD, 'matTable');
    bntMatrix = pnlMatrix;
    bnet.CPD{i} = tabular_CPD(bnet, i, 'CPT', bntMatrix(:));
    clear bntMatrix;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

for i=cnodes(:)'
    
    combMean = [];
    combCov = [];
    combWeights = [];
    pnlCPD = GetFactor(pnlBNet, i - 1);
    parents = double(GetParents(pnlGraph, i-1)) + 1;
    contPar = mysetdiff(parents, dnodes);
    discrPar = mysetdiff(parents, cnodes);
    discrParNt = ns(discrPar);
    ncomb= prod(discrParNt(:));
    nel = numel(discrPar);
    
    for dp = 0 : ncomb - 1
        comb = pnlDiscrComb(dp, discrParNt);
        weights = [];
        mean = GetMatrix(pnlCPD, 'matMean', -1, comb);
        cov  = GetMatrix(pnlCPD, 'matCovariance', -1, comb);
        for k = 0 : numel(contPar)-1
            tmp = GetMatrix(pnlCPD, 'matWeights', k, comb);
            %weights = [weights tmp'];
            weights = [weights tmp(:)'];
        end
        %combMean = [combMean mean];
        %combCov = [combCov cov];
        %combWeights = [combWeights weights];
        combMean = [combMean mean(:)];
        combCov = [combCov cov(:)];
        combWeights = [combWeights weights];
    end
   bnet.CPD{i} = gaussian_CPD(bnet, i, 'mean', combMean, 'cov', combCov, 'weights', combWeights );
end

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

function dbn = pnlDBN2bntDBN(pnlDBN)

nnodes = double(GetNumberOfNodes(pnlDBN));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dnodes = [];
j = 1;
for i = 0 : nnodes-1
    varType = GetNodeType(pnlDBN, i);
    if IsDiscrete(varType) == 1
        dnodes(j) = i+1; 
        j = j + 1;
    end
    ns(i+1) = GetNodeSize(varType);
   
end
if numel(dnodes) > 0
    cnodes = mysetdiff(1 : nnodes, dnodes);
else
    cnodes = 1:nnodes;
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

pnlGraph = GetGraph(pnlDBN);
dag = CreateAdjacencyMatrix(pnlGraph);
intra1 = zeros(nnodes);

intra1 = dag(1:nnodes, 1:nnodes);
intra  = dag((1:nnodes)+nnodes,(1:nnodes)+nnodes);
inter  = dag(1:nnodes,(1:nnodes)+nnodes);

if numel(cnodes) > 0
    if numel(dnodes) > 0
        onodes = cnodes;
    else
        onodes = getrandsubset(cnodes);
    end
else
    onodes = getrandsubset(dnodes);
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if intra1 ~= intra 
    dbn = mk_dbn(intra, inter, ns,'discrete', dnodes, 'intra1', intra1, 'observed', onodes );
else
    dbn = mk_dbn(intra, inter, ns,'discrete', dnodes, 'observed', onodes );
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dnds = [dnodes(:) ; dnodes(:) + nnodes];
for i = 1:numel(dnds)
    pnlCPD = GetFactor(pnlDBN, dnds(i) - 1);
    pnlMatrix = GetMatrix(pnlCPD, 'matTable');
    bntMatrix = pnlMatrix;
    dbn.CPD{dnds(i)} = tabular_CPD(dbn, dnds(i), bntMatrix);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
cnds = [cnodes(:); cnodes(:) + nnodes];
ns = [ns ns];
for i=1:numel(cnds)
    combMean = [];
    combCov = [];
    combWeights = [];
    pnlCPD = GetFactor(pnlDBN, cnds(i) - 1);
    parents = double(GetParents(pnlGraph, cnds(i)-1)) + 1;
    contPar = mysetdiff(parents, dnds);
    discrPar = mysetdiff(parents, cnds);  
    nel = numel(discrPar);
    ncomb = 1;
    discrParNt = ns(discrPar);
    ncomb= prod(discrParNt(:));
    
    
    for dp = 0 : ncomb - 1
        comb = pnlDiscrComb(dp, discrParNt);
        weights = [];
        mean = GetMatrix(pnlCPD, 'matMean', -1, comb);
        cov  = GetMatrix(pnlCPD, 'matCovariance', -1, comb);
        for k = 0 : numel(contPar)-1
            tmp = GetMatrix(pnlCPD, 'matWeights', k, comb);
            weights = [weights tmp(:)'];
        end
        combMean = [combMean mean(:)'];
        combCov = [combCov cov(:)'];
        combWeights = [combWeights weights];
    end
   dbn.CPD{cnds(i)} = gaussian_CPD(dbn, cnds(i), 'mean', combMean, 'cov', combCov, 'weights', combWeights );
end

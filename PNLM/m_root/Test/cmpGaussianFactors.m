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
function out = cmpGaussianFactors(pnlFactor, bntFactor, eps)

out = 1;
pnlDomain = double(GetDomain(pnlFactor));
x = struct(bntFactor);
sz = numel(pnlDomain);
pnlParents = pnlDomain(1:sz-1);
discrParNs = [];
contPar=[];
discrPar=[];
MD = GetModelDomain(pnlFactor);
j = 1;
for i = 1:numel(pnlParents)
    type = GetVariableType(MD, pnlParents(i) );
    if IsDiscrete(type) == 1
        discrParNs(j) = GetNodeSize(type);
        discrPar(j) = pnlParents(i);
        j = j + 1;
    end
end

if numel(discrParNs) > 0
    ncomb = prod(discrParNs);
    contPar = mysetdiff(pnlParents+1, discrPar+1)-1;
else
    ncomb = 1;
    if sz > 1
        contPar = pnlParents(1 : end-1 );
    end
end

if GetFactorType(pnlFactor) == 1
    combMean = [];
    combCov = [];
    combWeights = [];
    for dp = 0 : ncomb - 1
        comb = pnlDiscrComb(dp, discrParNs);
        weights = [];
        mean = GetMatrix(pnlFactor, 'matMean', -1, comb);
        
            cov  = GetMatrix(pnlFactor, 'matCovariance', -1, comb);
       
        for k = 0 : numel(contPar)-1
            tmp = GetMatrix(pnlFactor, 'matWeights', k, comb);
            weights = [weights tmp];
        end
        combMean = [combMean mean];
        combCov = [combCov cov];
        combWeights = [combWeights weights];
    end
    
    pnlMean = combMean(:);
    pnlCov = combCov(:);
    pnlWeights = combWeights(:);
    
    bntMean = x.mean(:);
    bntCov = x.cov(:);
    bntWeights = x.weights(:);
    
    if max(abs(pnlMean - bntMean)) > eps
        out = 0;
        (pnlMean)
        (bntMean)
        return
    end
    if max(abs(pnlCov - bntCov)) > eps
        out = 0;
        pnlCov
        bntCov
        return
    end
    if numel(pnlWeights) > 0
        if max(abs(pnlWeights - bntWeights)) > eps
            out = 0;
            (pnlWeights)
            (bntWeights)
            return
        end
    end
    out = 1;
else
    
    if IsDistributionSpecific(pnlFactor)
        if bntFactor.T == 1
            out = 1;
            
        else
            out = 0;
        end
        return
    end
    
    dt = GetDistributionType(pnlFactor);
      
    switch dt
    case 'dtScalar'  
        if bntFactor.T == 1
            out = 1;
            
        else
            out = 0;
        end
        return
        
    case 'dtGaussian'
        pnlMean = GetMatrix(pnlFactor, 'matMean');
        pnlCov  = GetMatrix(pnlFactor, 'matCovariance');
        
        bntMean = x.mu;
        bntCov  = x.Sigma;
        
        if max(abs(pnlMean(:) - bntMean(:))) > eps
            out = 0;
            (pnlMean)
            (bntMean)
            return
        end
        if max(abs(pnlCov(:) - bntCov(:))) > eps
            out = 0;
            (pnlCov)
            (bntCov)
            return
        end
        out = 1;
    otherwise
        ('bad distribution');
        out = 0;
        return
    end
end
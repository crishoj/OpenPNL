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

function out = cmpTabularFactors(pnlFactor, bntFactor, eps)


pnlMat = GetMatrix(pnlFactor, 'matTable');
x = struct(bntFactor);
if GetFactorType(pnlFactor) == 1
    bntMat = x.CPT;
else
    bntMat = x.T;
    bntMat = normalise(bntMat);
end
if max(abs(bntMat(:) - pnlMat(:))) > eps
    out = 0;
    disp('pnl tabular Factor');
    disp(pnlMat);
    disp('bnt tabular Factor');
    disp(bntMat);
    return
end
out = 1;
return
    

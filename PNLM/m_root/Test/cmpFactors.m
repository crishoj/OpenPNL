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
function out = cmpFactors(pnlFactor, bntFactor, eps)

out = 0;
% bntDomain = bntFactor.domain;
% pnlDomain = GetDomain(pnlFactor);
% 
% if( pnlDomain ~= bntDomain - 1)
%     disp('bad domain');
%     return    
% end
dt = GetDistributionType(pnlFactor);
switch(dt)
case 'dtTabular'
    out = cmpTabularFactors(pnlFactor, bntFactor, eps);
case {'dtGaussian', 'dtCondCaussian'}
    out = cmpGaussianFactors(pnlFactor, bntFactor, eps);
case 'dtScalar'
    if bntFactor.T == 1
        out = 1;
    end
    return
otherwise
    out = 0;
    disp('not implemented');
    return
end


 

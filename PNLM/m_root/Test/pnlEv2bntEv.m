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

function bntEv = pnlEv2bntEv(pnlEv)
dnodes = [];
nEv = numel(pnlEv);
ev=pnlEv{1};
MD = GetModelDomain(ev);
nnodes= double(GetNumberVariables(MD));
allNds = 1:nnodes;
nEl = ones(1, nnodes);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

j = 1;
for i = allNds   
    varType = GetVariableType(MD, i-1);
    if IsDiscrete(varType) == 1
        dnodes(j) = i; 
        j = j + 1;
        
    else
        nEl(i) = double(GetNodeSize(varType));
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

bntEv = cell(nnodes, nEv);

% for i = 1 : nEv 
%     ev = pnlEv{i};
%     bntEv = cell(1, nnodes);
%     [obsNds, pnlEvData] = GetObsNodesWithValues(ev);
%     if numel(obsNds) > 0
%         obsNds = double(obsNds) + 1;
%         offset = 1;
%         for j = obsNds
%             bntEv{j}=pnlEvData( offset : offset + nEl(j) - 1 );
%             offset = offset + nEl(j);
%         end
%         
%         for k = dnodes
%             bntEv{k} = bntEv{k} + 1;
%         end
%     end
%     bntEvVec{i} = bntEv;
% end

% Need to remove after GetObsNodesWithValues will work normal
for i = 1:nEv
    ev = pnlEv{i};
    rawData = GetRawData(ev);
    offset = 1;
    for j = allNds
        if IsNodeObserved(ev, j - 1 ) == 1
            bntEv{j, i} = rawData( offset : offset + nEl(j) - 1 );
        end
        offset = offset + nEl(j);
    end
    
    for k = dnodes
        bntEv{k, i} = bntEv{k, i} + 1;
    end  
end    

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
function res = testInf(varargin)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  Test inference engine 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

args = varargin;
if length(args) == 0
    rand('seed', now);
else
    rand('seed', varargin{1});
    pnlSeed(varargin{1});
end
res = 0;
seed = rand('seed');
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

nnds =  round(rand*(10))+2;
nedges = round(rand*nnds/3)+1;
ncont = round(rand*(nnds));
ntd = round(rand)+2;
ntc = round(rand*2)+1;

if round(rand*2) == 2
    if round(rand) == 1
        ncont = nnds;
    else
        ncont = 0;
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
pnlBNet = pnlExCreateRandomBNet(nnds, nedges, ncont, ntd, ntc);
ncomp = NumberOfConnectivityComponents(GetGraph(pnlBNet));
if ncomp > 1
    disp('bad Model');
end

bntBNet = pnlBNet2bntBNet(pnlBNet);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if numel(bntBNet.dnodes) == 0 | numel(bntBNet.cnodes) == 0
    infType = round(rand*3);
else
    infType = round(rand);
end
try
    switch infType
    case 0
        pnlInf = CNaiveInfEngineCreate(pnlBNet);
        bntInf = global_joint_inf_engine(bntBNet);
        disp('Naive inference engine');
    case 1
        pnlInf = CJTreeInfEngineCreate(pnlBNet);
        bntInf = jtree_inf_engine(bntBNet);
        disp('Junction tree inference engine');
        
    otherwise
        pnlInf = CPearlInfEngine(pnlBNet);
        bntInf = pearl_inf_engine(bntBNet,'protocol', 'parallel');
        disp('Pearl inference engine');
    end
catch
    disp('number conn comp');
    disp( NumberOfConnectivityComponents(GetGraph(pnlBNet)))
    disp('bad model')
    res = 0;
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

nnodes = double(GetNumberOfNodes(pnlBNet));
bntDnodes = bntBNet.dnodes;
pnlDnodes = bntDnodes - 1;

samp = GenerateSamples(pnlBNet, 1);
pnlEv = samp{1};

if numel(pnlDnodes) > 0
pnlHiddenNds = GetRandSubset(pnlDnodes);
else
    pnlHiddenNds = GetRandSubset(bntBNet.cnodes -1 );
end
ToggleNodeState(pnlEv, pnlHiddenNds);
bntEv = pnlEv2bntEv(samp);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if numel(pnlHiddenNds) > 0
pnlQuery = pnlHiddenNds(1);
else
    pnlQuery = round(rand*(nnodes - 1));
end
bntQuery =  pnlQuery + 1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

try
    
    EnterEvidence(pnlInf, pnlEv);
    [bntInf, ll] = enter_evidence(bntInf, bntEv);
    
    bntRes = marginal_nodes(bntInf, bntQuery);
    
    MarginalNodes(pnlInf, pnlQuery);
    pnlRes = GetQueryJPD(pnlInf);
    
    if cmpFactors(pnlRes, bntRes, 0.01) == 1
        disp('test OK')
        res = 1;
    else
        disp('test FAIL')
        res = 0;
    end
    
catch
     res = 0;
     disp('exception')
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if res == 0
    disp('seed')
    disp(seed)
    disp('pnl model description')
    disp('number of nodes')
    disp(nnodes)
    disp('discrete nodes')
    disp(pnlDnodes)
    disp('graph')
    disp(bntBNet.dag)
    disp('hidden nodes')
    disp(pnlHiddenNds)
    disp('query nodes')
    disp(pnlQuery)
    disp('inference type')
    disp(infType)
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                         %%
%%                INTEL CORPORATION PROPRIETARY INFORMATION                %%
%%   This software is supplied under the terms of a license agreement or   %%
%%  nondisclosure agreement with Intel Corporation and may not be copied   %%
%%   or disclosed except in accordance with the terms of that agreement.   %%
%%       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        %%
%%                                                                         %%
%%  File:      infWaterSprinklerBNet.m                                     %%
%%                                                                         %%
%%  Purpose:   Example of infernce procedure for BNet                      %% 
%%                                                                         %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

pBNet = WaterSprinklerBNetCreation;

%junction tree inference engine creation
jtree = CJtreeInfEngineCreate(pBNet);

%creation of the evidence
%let node 1 took on value 0 and node 2 took on value 1
obsNds = [ 1, 3 ];
obsNdsVls = [ 1, 1 ];

e = CEvidenceCreate( pBNet, obsNds, obsNdsVls );

% add the evidence to the engine
EnterEvidence( jtree, e );

%Finally, we can compute p=P(node_2 |node_1 = 0, node_3 = 1) as follows. 
query = [ 2 ];

MarginalNodes( jtree, query );
margPot = GetQueryJPD( jtree );
matrix = GetMatrix( margPot, 'matTable' );
barh(matrix);

disp('example of inference on the Water Sprinkler BNet is completed');

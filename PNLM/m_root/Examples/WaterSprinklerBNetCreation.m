%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                         %%
%%                INTEL CORPORATION PROPRIETARY INFORMATION                %%
%%   This software is supplied under the terms of a license agreement or   %%
%%  nondisclosure agreement with Intel Corporation and may not be copied   %%
%%   or disclosed except in accordance with the terms of that agreement.   %%
%%       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        %%
%%                                                                         %%
%%  File:      WaterSprinklerBNetCreation.m                                %%
%%                                                                         %%
%%  Purpose:   Creation water sprinkler BNet                               %% 
%%                                                                         %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


function bnet = WaterSprinklerBNetCreation()

numOfNds = 4;

%create NodeType objects and specify node types for
%all nodes of the model;
nodeTypes{1} = CNodeType(1, 2);% node type - discrete and binary
nodeAssociation(1:numOfNds) = 0;
%means that all nodes are of the same node type,
%which is 0th one in the array of node types for the model

%tables of probability distribution
matrix1 = [ 0.5, 0.5 ];
matrix2 = [ 0.5, 0.5; 0.9, 0.1 ];
matrix3 = [ 0.8, 0.2; 0.2, 0.8 ];
matrix4(:,:,1) = [ 1.0, 0.1; 0.1, 0.01 ];
matrix4(:,:,2) = [ 0.0, 0.9; 0.9, 0.99 ];

matrices = {matrix1, matrix2, matrix3, matrix4};

if 1
    % neighbors can be of either one of three following types:
    % a parent, a child or just a neighbor - for undirected graphs.
    % if a neighbor of a node is it's parent, then neighbor type is ntParent
    % if it's a child, then ntChild and if it's a neighbor, then ntNeighbor
    nbrs = {
        [ 1, 2 ],
        [ 0, 3 ],
        [ 0, 3 ],
        [ 1, 2 ]
    };
    nbrsTypes = {
        { 'ntChild',  'ntChild'  },
        { 'ntParent', 'ntChild'  },
        { 'ntParent', 'ntChild'  },
        { 'ntParent', 'ntParent' }
    };
    
    %this is a creation of directed graph for the BNet model
    graph = CGraph( nbrs, nbrsTypes );
    
    
    %creation BNet
    bnet = CBNet( numOfNds, nodeTypes, nodeAssociation, graph );
    
    for i=1:numOfNds
        AllocFactorByDomainNumber(bnet, i-1);
        factor = GetFactor(bnet, i-1);
        AttachMatrix(factor, matrices{i}, 'matTable');
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
else
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %create model domain
    MD = CModelDomainCreate( nodeTypes, nodeAssociation );
    
    %create adjacency matrix
    A = zeros(numOfNds,numOfNds);
    A(1,2) = 1;
    A(1,3) = 1;
    A(2,4) = 1;
    A(3,4) = 1;
    
    %create graph by adjacency matrix
    graph = CGraphCreateFromAdjMat(A);
   
    %create BNet by cgraph and model domain
    bnet = CBNetCreateByModelDomain( graph, MD );
       
    for i=1:numOfNds
        domain = [GetParents(graph, i-1); i-1];
        cpd = CTabularCPDCreate(MD,domain, matrices{i}); 
        AttachFactor(bnet, cpd);
    end
    
end
%% This file were automatically generated by SWIG's MatLab module
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                                                                         %%
%%                INTEL CORPORATION PROPRIETARY INFORMATION                %%
%%   This software is supplied under the terms of a license agreement or   %%
%%  nondisclosure agreement with Intel Corporation and may not be copied   %%
%%   or disclosed except in accordance with the terms of that agreement.   %%
%%       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        %%
%%                                                                         %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% [] = MaximumLikelihood(varargin)
%%
%% C++ prototype: void MaximumLikelihood(pnl::CSoftMaxCPD *self,float **Observation,int NumberOfObservations,float Accuracy,float step)
%%

function [] = MaximumLikelihood(varargin)

feval('pnl_full', 'CSoftMaxCPD_MaximumLikelihood_wrap', varargin{:});

return
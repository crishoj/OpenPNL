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
%% [OUTPUTOut1] = GetParents(varargin)
%%
%% C++ prototype: void GetParents(pnl::CGraph const *self,int nodeNum,pnl::intVector *OUTPUT)
%%

function [OUTPUTOut1] = GetParents(varargin)

[OUTPUTOut1] = feval('pnl_full', 'CGraph_GetParents_wrap', varargin{:});

return

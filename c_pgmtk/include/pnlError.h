// pgmError.h interface for class CEvidence

#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum 
{   pgmOK = 0
,   pgmFail = 0xffffffff
,   pnlMemoryFail = 0x100
,   pgmNotEnoughMemory = 0x101
,   pgmDamagedMemory = 0x102
,   pgmBadPointer = 0x103
,   pgmNumericFail = 0x200
,   pgmOverflow = 0x201
,   pgmUnderflow = 0x202
,   pgmAlgorithmic = 0x300
,   pgmNotConverged = 0x301
,   pgmInvalidOperation = 0x302
,   pgmBadArg = 0x400
,   pgmNULLPointer = 0x401
,   pgmOutOfRange = 0x402
,   pgmInconsistentType = 0x403
,   pgmInconsistentSize = 0x404
,   pgmBadConst = 0x405
,   pgmInternalError = 0x500
,   pgmNotImplemented = 0x501
} pgmErrorType;

#endif // __ERROR_H__

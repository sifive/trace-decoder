/*
 * Copyright 2019 SiFive, Inc.
 *
 * dqr.hpp
 */

/*
   This file is part of dqr, the SiFive Inc. Risc-V Nexus 2001 trace decoder.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DQR_HPP_
#define DQR_HPP_

// if config.h is not present, uncomment the lines below

//#define PACKAGE 1
//#define PACKAGE_VERSION 1

// PUBLIC definitions

#include "config.h"
#include "bfd.h"
#include "dis-asm.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <cassert>

#define DQR_MAXCORES	8

extern const char * const DQR_VERSION;

class TraceDqr {
public:
  typedef uint32_t RV_INST;

  typedef uint64_t ADDRESS;
  typedef uint64_t TIMESTAMP;
  typedef int RCode;

  enum {
	TRACE_HAVE_INSTINFO = 0x01,
	TRACE_HAVE_SRCINFO  = 0x02,
	TRACE_HAVE_MSGINFO  = 0x04,
  };

  typedef enum {
  	MSEO_NORMAL  = 0x00,
  	MSEO_VAR_END = 0x01,
  	MSEO_END     = 0x03,
  } MSEO;

  typedef enum {
  	DQERR_OK   = 0,		// no error
  	DQERR_OPEN = 1,		// can't open file
  	DQERR_EOF  = 2,		// at file eof
  	DQERR_EOM  = 3,		// at file eom
  	DQERR_BM   = 4,		// bad message (mallformed)
  	DQERR_ERR  = 5,		// general error
  	DQERR_DONE = 6,		// done with trace message
  } DQErr;

  typedef enum {
  	TCODE_DEBUG_STATUS       = 0,
  	TCODE_DEVICE_ID          = 1,
  	TCODE_OWNERSHIP_TRACE    = 2,
  	TCODE_DIRECT_BRANCH      = 3,
  	TCODE_INDIRECT_BRANCH    = 4,
  	TCODE_DATA_WRITE         = 5,
  	TCODE_DATA_READ          = 6,
  	TCODE_DATA_ACQUISITION   = 7,
  	TCODE_ERROR              = 8,
  	TCODE_SYNC               = 9,
  	TCODE_CORRECTION         = 10,
  	TCODE_DIRECT_BRANCH_WS   = 11,
  	TCODE_INDIRECT_BRANCH_WS = 12,
  	TCODE_DATA_WRITE_WS      = 13,
  	TCODE_DATA_READ_WS       = 14,
  	TCODE_WATCHPOINT         = 15,
  	TCODE_OUTPUT_PORTREPLACEMENT  = 20,
  	TCODE_INPUT_PORTREPLACEMENT   = 21,
  	TCODE_AUXACCESS_READ          = 22,
  	TCODE_AUXACCESS_WRITE         = 23,
  	TCODE_AUXACCESS_READNEXT      = 24,
  	TCODE_AUXACCESS_WRITENEXT     = 25,
  	TCODE_AUXACCESS_RESPONSE      = 26,
  	TCODE_RESURCEFULL             = 27,
  	TCODE_INDIRECTBRANCHHISTORY   = 28,
  	TCODE_INDIRECTBRANCHHISTORY_WS = 29,
  	TCODE_REPEATBRANCH            = 30,
  	TCODE_REPEATINSTRUCITON       = 31,
  	TCODE_REPEATSINSTURCIONT_WS   = 32,
  	TCODE_CORRELATION             = 33,
  	TCODE_INCIRCUITTRACE          = 34,

  	TCODE_UNDEFINED
  } TCode;

  typedef enum {
  	EVCODE_ENTERDEBUG	= 0,
  	EVCODE_TRACEDISABLE = 4,
  	EVCODE_ENTERRESET   = 8
  } EVCode;

  typedef enum {
  	SYNC_EVTI               = 0,
  	SYNC_EXIT_RESET         = 1,
  	SYNC_T_CNT              = 2,
  	SYNC_EXIT_DEBUG         = 3,
  	SYNC_I_CNT_OVERFLOW     = 4,
  	SYNC_TRACE_ENABLE       = 5,
  	SYNC_WATCHPINT          = 6,
  	SYNC_FIFO_OVERRUN       = 7,
  	SYNC_EXIT_POWERDOWN     = 9,
  	SYNC_MESSAGE_CONTENTION = 11,
  	SYNC_NONE
  } SyncReason;

  typedef enum {
  	BTYPE_INDIRECT  = 0,
  	BTYPE_EXCEPTION = 1,
  	BTYPE_HARDWARE  = 2,

  	BTYPE_UNDEFINED
  } BType;

  typedef enum {
	  ADDRDISP_WIDTHAUTO = 1,
	  ADDRDISP_SEP  = 2,
  } AddrDisp;

  enum InstType {
		INST_UNKNOWN = 0,
		INST_JAL,
		INST_JALR,
		INST_BEQ,
		INST_BNE,
		INST_BLT,
		INST_BGE,
		INST_BLTU,
		INST_BGEU,
		INST_C_J,
		INST_C_JAL,
		INST_C_JR,
		INST_C_JALR,
		INST_C_BEQZ,
		INST_C_BNEZ,
	};
};

// class Instruction: work with an instruction

#ifdef SWIG
	%ignore Instruction::addressToText(char *dst,size_t len,int labelLevel);
	%ignore Instruction::instructionToText(char *dst,size_t len,int labelLevel);
#endif // SWIG

class Instruction {
public:
	void addressToText(char *dst,size_t len,int labelLevel);
	std::string addressToString(int labelLevel);
//	void opcodeToText();
	void instructionToText(char *dst,size_t len,int labelLevel);
	std::string instructionToString(int labelLevel);

	uint8_t           coreId;
	TraceDqr::ADDRESS      address;
	TraceDqr::RV_INST      instruction;
	char              instructionText[64];
	int               instSize;
	static int        addrSize;
	static uint32_t   addrDispFlags;
	static int        addrPrintWidth;
#ifdef SWIG
	%immutable		addressLabel;
#endif // SWIG
	const char       *addressLabel;
	int               addressLabelOffset;
	bool              haveOperandAddress;
	TraceDqr::ADDRESS      operandAddress;
#ifdef SWIG
	%immutable		operandLabel;
#endif // SWIG
	const char       *operandLabel;
	int               operandLabelOffset;
};

// class Source: Helper class for source code information for an address

class Source {
public:
	std::string  sourceFileToString();
	std::string  sourceFileToString(std::string path);
	std::string  sourceLineToString();
	std::string  sourceFunctionToString();
	uint8_t      coreId;
#ifdef SWIG
	%immutable sourceFile;
	%immutable sourceFunction;
	%immutable sourceLine;
#endif // SWIG
	const char  *sourceFile;
	const char  *sourceFunction;
	const char  *sourceLine;
	unsigned int sourceLineNum;

private:
	const char *stripPath(const char *path);
};

// class NexusMessage: class to hold Nexus messages and convert them to text

#ifdef SWIG
	%ignore NexusMessage::messageToText(char *dst,size_t dst_len,int level);
#endif // SWIG

class NexusMessage {
public:
	NexusMessage();
	bool processITCPrintData(class ITCPrint *itcPrint);
	void messageToText(char *dst,size_t dst_len,int level);
	std::string messageToString(int detailLevel);
	double seconds();

	void dump();

	static uint32_t targetFrequency;

	int                 msgNum;
	TraceDqr::TCode     tcode;
    bool       	        haveTimestamp;
    TraceDqr::TIMESTAMP timestamp;
    TraceDqr::ADDRESS   currentAddress;
    TraceDqr::TIMESTAMP time;

    uint8_t             coreId;

    union {
    	struct {
    		int i_cnt;
    	} directBranch;
    	struct {
    		int          i_cnt;
    		TraceDqr::ADDRESS u_addr;
    		TraceDqr::BType   b_type;
    	} indirectBranch;
    	struct {
    		int             i_cnt;
    		TraceDqr::ADDRESS    f_addr;
    		TraceDqr::SyncReason sync;
    	} directBranchWS;
    	struct {
    		int             i_cnt;
    		TraceDqr::ADDRESS    f_addr;
    		TraceDqr::BType      b_type;
    		TraceDqr::SyncReason sync;
    	} indirectBranchWS;
    	struct {
    		int             i_cnt;
    		TraceDqr::ADDRESS    u_addr;
    		TraceDqr::BType      b_type;
    		uint64_t		history;
    	} indirectHistory;
    	struct {
    		int             i_cnt;
    		TraceDqr::ADDRESS    f_addr;
    		TraceDqr::BType      b_type;
    		uint64_t		history;
    		TraceDqr::SyncReason sync;
    	} indirectHistoryWS;
    	struct {
    		TraceDqr::RCode rCode;
    		union {
    			int i_cnt;
    			uint64_t history;
    			int count;
    		};
    	} resourceFull;
    	struct {
    		int             i_cnt;
    		TraceDqr::ADDRESS    f_addr;
    		TraceDqr::SyncReason sync;
    	} sync;
    	struct {
    		uint8_t etype;
    	} error;
    	struct {
    		uint64_t history;
    		int     i_cnt;
    		uint8_t cdf;
    		uint8_t evcode;
    	} correlation;
    	struct {
    		uint32_t data;
    		uint32_t addr;
    	} auxAccessWrite;
    	struct {
    		uint32_t idTag;
    		uint32_t data;
    	} dataAcquisition;
    	struct {
    		uint32_t process;
    	} ownership;
    };

    int getI_Cnt();
    TraceDqr::ADDRESS    getU_Addr();
    TraceDqr::ADDRESS    getF_Addr();
    TraceDqr::BType      getB_Type();
	TraceDqr::SyncReason getSyncReason();
	uint8_t   getEType();
	uint8_t  getCDF();
	uint8_t  getEVCode();
	uint32_t getData();
	uint32_t getAddr();
	uint32_t getIdTag();
	uint32_t getProcess();
	uint32_t getRCode();
	uint64_t getRData();
	uint32_t getCount();
	uint64_t getHistory();
};

#ifdef SWIG
	%ignore Analytics::toText(char *dst,size_t dst_len,int level);
#endif // SWIG

class Analytics {
public:
	Analytics();
	TraceDqr::DQErr updateTraceInfo(NexusMessage &nm,uint32_t bits,uint32_t meso_bits,uint32_t ts_bits,uint32_t addr_bits);
	TraceDqr::DQErr updateInstructionInfo(uint32_t core_id,uint32_t inst,int instSize);
	int currentTraceMsgNum() { return num_trace_msgs_all_cores; }
	void setSrcBits(int sbits) { srcBits = sbits; }
	void toText(char *dst,int dst_len,int detailLevel);
	std::string toString(int detailLevel);

private:
	TraceDqr::DQErr status;
	uint32_t cores;

	int srcBits;

	uint32_t num_trace_msgs_all_cores;
	uint32_t num_trace_mseo_bits_all_cores;
	uint32_t num_trace_bits_all_cores;
	uint32_t num_trace_bits_all_cores_max;
	uint32_t num_trace_bits_all_cores_min;

	uint32_t num_inst_all_cores;
	uint32_t num_inst16_all_cores;
	uint32_t num_inst32_all_cores;

	uint32_t num_branches_all_cores;

	struct {
		uint32_t num_inst;
		uint32_t num_inst16;
		uint32_t num_inst32;

		uint32_t num_trace_msgs;
		uint32_t num_trace_syncs;
		uint32_t num_trace_dbranch;
		uint32_t num_trace_ibranch;
		uint32_t num_trace_dataacq;
		uint32_t num_trace_dbranchws;
		uint32_t num_trace_ibranchws;
		uint32_t num_trace_ihistory;
		uint32_t num_trace_ihistoryws;
		uint32_t num_trace_resourcefull;
		uint32_t num_trace_correlation;
		uint32_t num_trace_auxaccesswrite;
		uint32_t num_trace_ownership;
		uint32_t num_trace_error;

		uint32_t trace_bits;
		uint32_t trace_bits_max;
		uint32_t trace_bits_min;
		uint32_t trace_bits_mseo;

		uint32_t trace_bits_sync;
		uint32_t trace_bits_dbranch;
		uint32_t trace_bits_ibranch;
		uint32_t trace_bits_dataacq;
		uint32_t trace_bits_dbranchws;
		uint32_t trace_bits_ibranchws;
		uint32_t trace_bits_ihistory;
		uint32_t trace_bits_ihistoryws;
		uint32_t trace_bits_resourcefull;
		uint32_t trace_bits_correlation;
		uint32_t trace_bits_auxaccesswrite;
		uint32_t trace_bits_ownership;
		uint32_t trace_bits_error;

		uint32_t num_trace_ts;
		uint32_t num_trace_uaddr;
		uint32_t num_trace_faddr;
		uint32_t num_trace_ihistory_taken_branches;
		uint32_t num_trace_ihistory_nottaken_branches;
		uint32_t num_trace_resourcefull_i_cnt;
		uint32_t num_trace_resourcefull_hist;
		uint32_t num_trace_resourcefull_count;

		uint32_t trace_bits_ts;
		uint32_t trace_bits_ts_max;
		uint32_t trace_bits_ts_min;

		uint32_t trace_bits_uaddr;
		uint32_t trace_bits_uaddr_max;
		uint32_t trace_bits_uaddr_min;

		uint32_t trace_bits_faddr;
		uint32_t trace_bits_faddr_max;
		uint32_t trace_bits_faddr_min;

		uint32_t trace_bits_hist;
	} core[DQR_MAXCORES];
};

// class Trace: high level class that performs the raw trace data to dissasemble and decorated instruction trace

#ifdef SWIG
	%ignore Trace::NextInstruction(Instruction **instInfo,NexusMessage **msgInfo,Source **srcInfo);
	%ignore Trace::getITCPrintMsg(int core,char *dst,int dstLen,TraceDqr::TIMESTAMP &startTime,TraceDqr::TIMESTAMP &endTime);
	%ignore Trace::flushITCPrintMsg(int core,char *dst,int dstLen,TraceDqr::TIMESTAMP &startTime,TraceDqr::TIMESTAMP &endTime);
	%ignore Trace::analyticsToText(char *dst,int dst_len,int detailLevel);
	%ignore Trace::getITCPrintStr(int core,bool &haveData,TraceDqr::TIMESTAMP &startTime,TraceDqr::TIMESTAMP &endtime);
	%ignore Trace::flushITCPrintStr(int core,bool &haveData,TraceDqr::TIMESTAMP &startTime,TraceDqr::TIMESTAMP &endtime);
#endif // SWIG

class Trace {
public:
    Trace(char *tf_name,bool binaryFlag,char *ef_name,int numAddrBits,uint32_t addrDispFlags,int srcBits,uint32_t freq = 0);
    ~Trace();
	TraceDqr::DQErr setTraceRange(int start_msg_num,int stop_msg_num);
	TraceDqr::DQErr setITCPrintOptions(int buffSize,int channel);

	enum TraceFlags {
		TF_INSTRUCTION = 0x01,
		TF_ADDRESS     = 0x02,
		TF_DISSASEMBLE = 0x04,
		TF_TIMESTAMP   = 0x08,
		TF_TRACEINFO   = 0x10,
	};
	TraceDqr::DQErr getStatus() { return status; }
	TraceDqr::DQErr NextInstruction(Instruction **instInfo, NexusMessage **msgInfo, Source **srcInfo);
	TraceDqr::DQErr NextInstruction(Instruction *instInfo, NexusMessage *msgInfo, Source *srcInfo, int *flags);

	TraceDqr::DQErr haveITCPrintData(int numMsgs[DQR_MAXCORES], bool havePrintData[DQR_MAXCORES]);
	bool        getITCPrintMsg(int core,char *dst, int dstLen,TraceDqr::TIMESTAMP &startTime,TraceDqr::TIMESTAMP &endTime);
	bool        flushITCPrintMsg(int core,char *dst, int dstLen,TraceDqr::TIMESTAMP &startTime,TraceDqr::TIMESTAMP &endTime);
	std::string getITCPrintStr(int core, bool &haveData,TraceDqr::TIMESTAMP &startTime,TraceDqr::TIMESTAMP &endTime);
	std::string flushITCPrintStr(int core, bool &haveData,TraceDqr::TIMESTAMP &startTime,TraceDqr::TIMESTAMP &endTime);

	std::string getITCPrintStr(int core, bool &haveData,double &startTime,double &endTime);
	std::string flushITCPrintStr(int core, bool &haveData,double &startTime,double &endTime);

	const char *getSymbolByAddress(TraceDqr::ADDRESS addr);
	const char *getNextSymbolByAddress();
	int         Disassemble(TraceDqr::ADDRESS addr);
	int         getArchSize();
	int         getAddressSize();
	void analyticsToText(char *dst,int dst_len,int detailLevel) {analytics.toText(dst,dst_len,detailLevel); }
	std::string analyticsToString(int detailLevel) { return analytics.toString(detailLevel); }

private:
	enum state {
		TRACE_STATE_GETFIRSTYNCMSG,
		TRACE_STATE_GETSECONDMSG,
		TRACE_STATE_GETSTARTTRACEMSG,
		TRACE_STATE_COMPUTESTARTINGADDRESS,
		TRACE_STATE_RETIREMESSAGE,
		TRACE_STATE_GETNEXTMSG,
		TRACE_STATE_GETNEXTINSTRUCTION,
		TRACE_STATE_DONE,
		TRACE_STATE_ERROR
	};

	TraceDqr::DQErr       status;
	class SliceFileParser *sfp;
	class ElfReader       *elfReader;
	class Symtab          *symtab;
	class Disassembler    *disassembler;
	class ITCPrint        *itcPrint;
	TraceDqr::ADDRESS     currentAddress[DQR_MAXCORES];
	TraceDqr::ADDRESS	 lastFaddr[DQR_MAXCORES];
	TraceDqr::TIMESTAMP   lastTime[DQR_MAXCORES];
	enum state       state[DQR_MAXCORES];
	bool             readNewTraceMessage;
	int              currentCore;
	int              srcbits;
	bool             bufferItc;

	int              startMessageNum;
	int              endMessageNum;

	Analytics        analytics;

//	need current message number and list of messages??

	NexusMessage     nm;

	NexusMessage     messageInfo;
	Instruction      instructionInfo;
	Source           sourceInfo;

	//	or maybe have this stuff in the nexus messages??

	int i_cnt[DQR_MAXCORES];

	uint32_t                inst = -1;
	int                     inst_size = -1;
	TraceDqr::InstType inst_type = TraceDqr::InstType::INST_UNKNOWN;
	int32_t                 immeadiate = -1;
	bool                    is_branch = false;

	class NexusMessageSync *messageSync[DQR_MAXCORES];

	int decodeInstructionSize(uint32_t inst, int &inst_size);
	int decodeInstruction(uint32_t instruction,int &inst_size,TraceDqr::InstType &inst_type,int32_t &immeadiate,bool &is_branch);

	TraceDqr::ADDRESS computeAddress();
};

#endif /* DQR_HPP_ */

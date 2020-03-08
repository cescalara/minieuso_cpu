#ifndef _DATA_FORMAT_H
#define _DATA_FORMAT_H

/*
 * CPU data format definition 
 *----------------------------*
 * NEW VARIABLE PACKET FORMAT FROM NOV 2017 
 * for storage of packets coming from the Zynq board and ancillary instruments 
 * Francesca Capel: capel.francesca@gmail.com 
 * NB: the Mini-EUSO CPU is little endian
 */

/* new multi event data format */
#include <vector>

#include "minieuso_pdmdata.h"

/*
 * software definitions
 */

#define VERSION 8.7
#define VERSION_DATE_STRING "08/03/2020"

/*
 * instrument definitions 
 */
#define INSTRUMENT "Mini-EUSO"
#define INSTRUMENT_ME_PDM 1 
#define ID_TAG 0xAA55AA55
#define ID_TAG_SUB 0xBB66BB66 
#define RUN_SIZE 25

/*
 * force no padding in structs
 */
#pragma pack(push, 1) 

/*
 * define the size of the run_info text field in CpuFileHeader
 */
#define RUN_INFO_SIZE 512

/** 
 * cpu file header 
 * 12 bytes 
 */
typedef struct
{
  uint32_t spacer = ID_TAG; /* AA55AA55 HEX */
  uint32_t header; /* 'C'(31:24) | instrument_id(23:16) | file_type(15:8) | file_ver(7:0) */
  char run_info[RUN_INFO_SIZE]; /* text describing the run */
  uint32_t run_size; /* number of cpu packets in the run */
} CpuFileHeader; 

/**
 * cpu file trailer
 * 12 bytes 
 */
typedef struct
{
  uint32_t spacer = ID_TAG; /* AA55AA55 HEX */
  uint32_t header; /* 'Q'(31:24) | instrument_id(23:16) | pkt_type(15:8) | pkt_ver(7:0) */
  uint32_t run_size; /* number of cpu packets in the run */
  uint32_t crc; /* checksum */
} CpuFileTrailer; 

/**
 * generic packet header for all cpu packets 
 * the zynq packet has its own header defined in minieuso_pdmdata.h 
 * 16 bytes 
 */
typedef struct
{
  uint32_t spacer = ID_TAG; /* AA55AA55 HEX */
  uint32_t header; /* 'P'(31:24) | instrument_id(23:16) | pkt_type(15:8) | pkt_ver(7:0) */
  uint32_t pkt_size; /* size of packet */
  uint32_t pkt_num; /* counter for each pkt_type, reset each run */
} CpuPktHeader; 

/**
 * generic packet header for all hk sub packets 
 * the zynq packet has its own header defined in minieuso_pdmdata.h 
 * 16 bytes 
 */
typedef struct
{
  uint32_t spacer = ID_TAG_SUB; /* BB66BB66 HEX */
  uint32_t header; /* 'P'(31:24) | instrument_id(23:16) | pkt_type(15:8) | pkt_ver(7:0) */
  uint32_t pkt_size; /* size of packet */
  uint32_t pkt_num; /* counter for each pkt_type, reset each run */
} SubPktHeader; 


/*
 * file types 
 */

#define CPU_FILE_TYPE 'C'
#define SC_FILE_TYPE 'S'  
#define HV_FILE_TYPE 'H'  
#define SC_FILE_VER 1
#define HV_FILE_VER 1
#define CPU_FILE_VER 1


/*
 * packet types 
 */

#define THERM_PACKET_TYPE 'T'
#define HK_PACKET_TYPE 'H'
#define HV_PACKET_TYPE 'V'
#define SC_PACKET_TYPE 'S'
#define CPU_PACKET_TYPE 'P'
#define TRAILER_PACKET_TYPE 'Q'
#define THERM_PACKET_VER 1
#define HK_PACKET_VER 1
#define HV_PACKET_VER 1
#define SC_PACKET_VER 2
#define CPU_PACKET_VER 2

/*
 * for the analog readout 
 */

#define N_CHANNELS_PHOTODIODE 4
#define N_CHANNELS_SIPM 64
#define N_CHANNELS_THERM 10

/*
 * size of the zynq packets 
 */

#define MAX_PACKETS_L1 4
#define MAX_PACKETS_L2 4
#define MAX_PACKETS_L3 1

/**
 * timestamp
 * 4 bytes 
 * unix time in s
 */
typedef struct
{
  uint32_t cpu_time_stamp; 
} CpuTimeStamp;


/**
 * thermistor packet for temperature data
 * 60 bytes 
 */
typedef struct
{
  CpuPktHeader therm_packet_header; /* 16 bytes */
  CpuTimeStamp therm_time; /* 4 bytes */
  float therm_data[N_CHANNELS_THERM]; /* 40 bytes */
} THERM_PACKET;

/**
 * housekeeping packet for sipm and photodiode data 
 * 296 bytes 
 */
typedef struct
{
  SubPktHeader hk_packet_header; /* 16 bytes */
  CpuTimeStamp hk_time; /* 4 bytes */
  float photodiode_data[N_CHANNELS_PHOTODIODE]; /* 16 bytes */ 
  float sipm_data[N_CHANNELS_SIPM]; /* 256 bytes */
  float sipm_single; /* 4 bytes */
} HK_PACKET;

/**
 * zynq packet passed to the CPU every 5.24 s 
 * variable size, depending on configurable N1 and N2 
 * **NB: vector itself is not written to file, 
 * just contents which are contiguous in memory** 
 */
typedef struct
{
  uint8_t N1; /* 1 byte */
  uint8_t N2; /* 1 byte */
  std::vector<Z_DATA_TYPE_SCI_L1_V2> level1_data; /* 294944 * N1 bytes */
  std::vector<Z_DATA_TYPE_SCI_L2_V2> level2_data; /* 589856 * N2 bytes */
  Z_DATA_TYPE_SCI_L3_V2 level3_data; /* 1179684 bytes */
} ZYNQ_PACKET;

/**
 * CPU packet for incoming data every 5.24 s 
 * variable size 
 */
typedef struct
{
  CpuPktHeader cpu_packet_header; /* 16 bytes */
  CpuTimeStamp cpu_time; /* 4 bytes */
  HK_PACKET hk_packet; /* 296 bytes */
  ZYNQ_PACKET zynq_packet; /* variable size */
} CPU_PACKET;

/**
 * CPU file to store one run 
 * shown here as demonstration only 
 * variable size 
 */
typedef struct
{
  CpuFileHeader cpu_file_header; /* 12 bytes */
  CPU_PACKET cpu_run_payload[RUN_SIZE]; /* variable size */
  CpuFileTrailer cpu_file_trailer; /* 12 bytes */
} CPU_FILE;

/**
 * SC packet to store S-curve from Zynq 
 * 9437220 bytes
 */
typedef struct
{
  CpuPktHeader sc_packet_header; /* 16 bytes */
  CpuTimeStamp sc_time; /* 4 bytes */
  uint16_t sc_start; /* 2 bytes */
  uint16_t sc_step; /* 2 bytes */
  uint16_t sc_stop; /* 2 bytes */
  uint16_t sc_acc; /* 2 bytes */
  Z_DATA_TYPE_SCURVE_V1 sc_data; /* 9437192 bytes */
} SC_PACKET;

/**
 * SC file to store a single S-curve
 * shown here as demonstration only 
 * 9437244 bytes (~9 MB) 
 */
typedef struct
{
  CpuFileHeader cpu_file_header; /* 12 bytes */
  SC_PACKET scurve_packet; /* 9437220 bytes */
  CpuFileTrailer cpu_file_trailer; /* 12 bytes */
} SC_FILE;

/**
 * HV packet for HV switching data 
 * variable file size 
 */
typedef struct
{
  CpuPktHeader hv_packet_header; /* 16 bytes */
  CpuTimeStamp hv_time; /* 4 bytes */
  uint32_t N; /* 4 bytes */
  ZynqBoardHeader zbh; /* 8 bytes */
  std::vector<DATA_TYPE_HVPS_LOG_V1> hvps_log; /* N * 16 bytes */
} HV_PACKET;

/**
 * HV file to store a single HVPS log 
 * shown here as demonstration only 
 * variable file size 
 */
typedef struct
{
  CpuFileHeader cpu_file_header; /* 12 bytes */
  HV_PACKET hv_packet; /* 1600028 bytes */
  CpuFileTrailer cpu_file_trailer; /* 12 bytes */
} HV_FILE;

/**
 * return to normal packing
 */
#pragma pack(pop) 

#endif /* _DATA_FORMAT_H */

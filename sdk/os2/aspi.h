//-------------------------//
//     ASPI SRB Header     //
//-------------------------//

#pragma pack(1)

typedef struct _ASPI_SRBHDR {                 /* ASHD */
 UCHAR         CommandCode;
 UCHAR         ASPIStatus;
 UCHAR         AdapterIndex;
 UCHAR         ASPIReqFlags;
} ASPI_SRB_HEADER,
  *NPASPI_SRB_HEADER, 
  FAR *PASPI_SRB_HEADER;

//-------------------------------------------------//
//  Command Values in ASPI_SRB_HEADER->CommandCode //
//-------------------------------------------------//
#define ASPI_CMD_ADAPTER_INQUIRY        0x00
#define ASPI_CMD_GET_DEVICE_TYPE        0x01
#define ASPI_CMD_EXECUTE_IO             0x02
#define ASPI_CMD_ABORT_IO               0x03
#define ASPI_CMD_RESET_DEVICE           0x04
#define ASPI_CMD_SET_ADAPTER_PARMS      0x05

//-------------------------------------------------------//
//   ASPI Status Values in ASPI_SRB_HEADER->ASPIStatus   //
//-------------------------------------------------------//
#define ASPI_STATUS_IN_PROGRESS         0x00
#define ASPI_STATUS_NO_ERROR            0x01
#define ASPI_STATUS_ABORTED             0x02
#define ASPI_STATUS_ERROR               0x04
#define ASPI_STATUS_INVALID_COMMAND     0x80
#define ASPI_STATUS_INVALID_ADAPTER     0x81
#define ASPI_STATUS_INVALID_TARGET      0x82

//-------------------------------------------------------//
//   ASPI Flags in ASPI_SRB_HEADER->ASPIReqFlags         //
//-------------------------------------------------------//
#define ASPI_REQFLAG_POST_ENABLE        0x01
#define ASPI_REQFLAG_LINKED_SRB         0x02
#define ASPI_REQFLAG_RESIDUAL           0x04
#define ASPI_REQFLAG_DIRECTION_BITS     0x18
#define ASPI_REQFLAG_DIR_TO_HOST        0x08
#define ASPI_REQFLAG_DIR_TO_TARGET      0x10
#define ASPI_REQFLAG_DIR_NO_DATA_XFER   0x18
#define ASPI_REQFLAG_SG_ENABLE          0x20

//*********************************************************************//
//   ASPI ADAPTER INQUIRY SRB           (for ASPI_CMD_ADAPTER_INQUIRY) //
//*********************************************************************//
typedef struct _ASPI_SRB_ADAPTER_INQUIRY {           /* ASAI */
 ASPI_SRB_HEADER        SRBHdr;
 UCHAR                  Reserved_1[4];
 UCHAR                  AdapterCount;
 UCHAR                  AdapterTargetID;
 UCHAR                  ManagerName[16];
 UCHAR                  AdapterName[16];
 UCHAR                  AdapterParms[16];
} ASPI_SRB_INQUIRY, 
  *NPASPI_SRB_INQUIRY, 
  FAR *PASPI_SRB_INQUIRY;

//**********************************************************************//
//   ASPI GET DEVICE TYPE SRB          (for ASPI_CMD_DEVICE_TYPE)       //
//**********************************************************************//
typedef struct _ASPI_SRB_DEVICE_TYPE {               /* ASDT */
 ASPI_SRB_HEADER        SRBHdr;
 UCHAR                  Reserved_1[4];
 UCHAR                  DeviceTargetID;
 UCHAR                  DeviceTargetLUN;
 UCHAR                  DeviceType;
} ASPI_SRB_DEVICE_TYPE, 
  *NPASPI_SRB_DEVICE_TYPE, 
  FAR *PASPI_SRB_DEVICE_TYPE;

//**********************************************************************//
//    ASPI EXECUTE IO SRB               (for ASPI_CMD_EXECUTE_IO)       //
//**********************************************************************//
typedef struct _ASPI_SRB_EXECUTE_IO {                /* ASEI */
 ASPI_SRB_HEADER        SRBHdr;
 USHORT                 SGListLen;
 UCHAR                  Reserved_1[2];
 UCHAR                  DeviceTargetID;
 UCHAR                  DeviceTargetLUN;
 ULONG                  DataXferLen;
 UCHAR                  SenseDataLen;
 ULONG                  ppDataBuffer;
 ULONG                  ppNxtSRB;
 UCHAR                  SCSICDBLen;
 UCHAR                  HostStatus;
 UCHAR                  TargetStatus;
 VOID              (FAR *RM_PostAddress)(USHORT DataSeg,PASPI_SRB_HEADER pSRBH);
 USHORT                 RM_DataSeg;
 VOID              (FAR *PM_PostAddress)(USHORT DataSeg,PASPI_SRB_HEADER pSRBH);
 USHORT                 PM_DataSeg;
 ULONG                  ppSRB;
 UCHAR                  ASPIWorkSpace[22];
 UCHAR                  SCSICDBStart[1];
} ASPI_SRB_EXECUTE_IO, 
  *NPASPI_SRB_EXECUTE_IO, 
  FAR *PASPI_SRB_EXECUTE_IO;

//-----------------------------------------------------//
//  Status returned in ASPI_SRB_EXECUTE_IO->HostStatus //
//-----------------------------------------------------//
#define ASPI_HSTATUS_NO_ERROR               0x00
#define ASPI_HSTATUS_SELECTION_TIMEOUT      0x11
#define ASPI_HSTATUS_DATA_OVERRUN           0x12
#define ASPI_HSTATUS_BUS_FREE               0x13
#define ASPI_HSTATUS_BUS_PHASE_ERROR        0x14
#define ASPI_HSTATUS_BAD_SGLIST             0x1A       //@53040

//-------------------------------------------------------//
// Status returned in ASPI_SRB_EXECUTE_IO->TargetStatus  //
//-------------------------------------------------------//
#define ASPI_TSTATUS_NO_ERROR               0x00
#define ASPI_TSTATUS_CHECK_CONDITION        0x02
#define ASPI_TSTATUS_BUSY                   0x08
#define ASPI_TSTATUS_RESERV_CONFLICT        0x18

//***********************************************************************//
//     ASPI ABORT IO SRB                 (for ASPI_CMD_ABORT_IO)         //
//***********************************************************************//
typedef struct _ASPI_SRB_ABORT_IO {                  /* ASAI */
 ASPI_SRB_HEADER        SRBHdr;
 UCHAR                  Reserved_1[4];
 ULONG                  ppSRB;
} ASPI_SRB_ABORT_IO, 
  *NPASPI_SRB_ABORT_IO, 
  FAR *PASPI_SRB_ABORT_IO;

//**********************************************************************//
//   ASPI RESET DEVICE SRB             (for ASPI_RESET_DEVICE)          //
//**********************************************************************//
typedef struct _ASPI_SRB_RESET_DEVICE {              /* ASRD */
 ASPI_SRB_HEADER        SRBHdr;
 UCHAR                  Reserved_1[4];
 UCHAR                  DeviceTargetID;
 UCHAR                  DeviceTargetLUN;
 UCHAR                  Reserved_2[14];
 UCHAR                  HostStatus;
 UCHAR                  TargetStatus;
 VOID            (FAR * RM_PostAddress) (USHORT DataSeg,PASPI_SRB_HEADER pSRBH);
 USHORT                 RM_DataSeg;
 VOID            (FAR * PM_PostAddress)(USHORT DataSeg,PASPI_SRB_HEADER pSRBH);
 USHORT                 PM_DataSeg;
 UCHAR                  ASPIWorkSpace[22];
} ASPI_SRB_RESET_DEVICE, 
  *NPASPI_SRB_RESET_DEVICE, 
  FAR *PASPI_SRB_RESET_DEVICE;

//**********************************************************************//
//  ASPI SET HOST ADAPTER PARAMETERS  (for ASPI_CMD_SET_ADAPTER_PARMS)  //
//**********************************************************************//
typedef struct _ASPI_SRB_ADAPTER_PARMS {             /* ASAP */
 ASPI_SRB_HEADER        SRBHdr;
 UCHAR                  Reserved_1[4];
 UCHAR                  AdapterParms[16];
} ASPI_SRB_ADAPTER_PARMS, 
  *NPASPI_SRB_ADAPTER_PARMS, 
  FAR *PASPI_SRB_ADAPTER_PARMS;

#pragma pack()

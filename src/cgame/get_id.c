//  diskid32.cpp


//  for displaying the details of hard drives in a command window


//  06/11/00  Lynn McGuire  written with many contributions from others,
//                            IDE drives only under Windows NT/2K and 9X,
//                            maybe SCSI drives later
//  11/20/03  Lynn McGuire  added ReadPhysicalDriveInNTWithZeroRights
//  10/26/05  Lynn McGuire  fix the flipAndCodeBytes function
//  01/22/08  Lynn McGuire  incorporate changes from Gonzalo Diethelm,
//                             remove media serial number code since does 
//                             not work on USB hard drives or thumb drives
//  01/29/08  Lynn McGuire  add ReadPhysicalDriveInNTUsingSmart
//____________________________________________________________________________

#include "cg_local.h"
#ifdef GS_UUID
#include "get_id.h"


#ifdef WIN32
// IP ++++++++++++++++++++++++++++++
/*
#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
//#else
//#include <sys/socket.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <unistd.h>
#endif*/

// HardDriveSerial  ++++++++++++++++
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#define Rectangle LCC_Rectangle
#include <windows.h>
#undef Rectangle
#include <winioctl.h>


// #############################################################################

	//  special include from the MS DDK
//#include "c:\win2kddk\inc\ddk\ntddk.h"
//#include "c:\win2kddk\inc\ntddstor.h"

//#define  TITLE   "DiskId32"



char HardDriveSerialNumber [1024];
char HardDriveModelNumber [1024];



   //  Required to ensure correct PhysicalDrive IOCTL structure setup
#pragma pack(1)


#define  IDENTIFY_BUFFER_SIZE  512


   //  IOCTL commands
#define  DFP_GET_VERSION          0x00074080
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition

#define SMART_GET_VERSION               CTL_CODE(IOCTL_DISK_BASE, 0x0020, METHOD_BUFFERED, FILE_READ_ACCESS)
#define SMART_SEND_DRIVE_COMMAND        CTL_CODE(IOCTL_DISK_BASE, 0x0021, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define SMART_RCV_DRIVE_DATA            CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


   //  GETVERSIONOUTPARAMS contains the data returned from the 
   //  Get Driver Version function.
typedef struct _GETVERSIONOUTPARAMS
{
   BYTE bVersion;      // Binary driver version.
   BYTE bRevision;     // Binary driver revision.
   BYTE bReserved;     // Not used.
   BYTE bIDEDeviceMap; // Bit map of IDE devices.
   DWORD fCapabilities; // Bit mask of driver capabilities.
   DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;


   //  Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS 
#define  CAP_IDE_ID_FUNCTION             1  // ATA ID command supported
#define  CAP_IDE_ATAPI_ID                2  // ATAPI ID command supported
#define  CAP_IDE_EXECUTE_SMART_FUNCTION  4  // SMART commannds supported


   //  Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.

   // The following struct defines the interesting part of the IDENTIFY
   // buffer:
typedef struct _IDSECTOR
{
   USHORT  wGenConfig;
   USHORT  wNumCyls;
   USHORT  wReserved;
   USHORT  wNumHeads;
   USHORT  wBytesPerTrack;
   USHORT  wBytesPerSector;
   USHORT  wSectorsPerTrack;
   USHORT  wVendorUnique[3];
   CHAR    sSerialNumber[20];
   USHORT  wBufferType;
   USHORT  wBufferSize;
   USHORT  wECCSize;
   CHAR    sFirmwareRev[8];
   CHAR    sModelNumber[40];
   USHORT  wMoreVendorUnique;
   USHORT  wDoubleWordIO;
   USHORT  wCapabilities;
   USHORT  wReserved1;
   USHORT  wPIOTiming;
   USHORT  wDMATiming;
   USHORT  wBS;
   USHORT  wNumCurrentCyls;
   USHORT  wNumCurrentHeads;
   USHORT  wNumCurrentSectorsPerTrack;
   ULONG   ulCurrentSectorCapacity;
   USHORT  wMultSectorStuff;
   ULONG   ulTotalAddressableSectors;
   USHORT  wSingleWordDMA;
   USHORT  wMultiWordDMA;
   BYTE    bReserved[128];
} IDSECTOR, *PIDSECTOR;


typedef struct _SRB_IO_CONTROL
{
   ULONG HeaderLength;
   UCHAR Signature[8];
   ULONG Timeout;
   ULONG ControlCode;
   ULONG ReturnCode;
   ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;


   // Define global buffers.
BYTE IdOutCmd [sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];


//void PrintIdeInfo (int drive, DWORD diskdata [256]);
BOOL DoIDENTIFY (HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, BYTE,
                 PDWORD);


   //  Max number of drives assuming primary/secondary, master/slave topology
#define  MAX_IDE_DRIVES  16


int ReadPhysicalDriveInNTWithAdminRights (void)
{
   int done = FALSE;
   int drive = 0;

   for (drive = 0; drive < MAX_IDE_DRIVES; drive++)
   {
      HANDLE hPhysicalDriveIOCTL = 0;

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      char driveName [256];

      sprintf (driveName, "\\\\.\\PhysicalDrive%d", drive);

         //  Windows NT, Windows 2000, must have admin rights
      hPhysicalDriveIOCTL = CreateFile (driveName,
                               GENERIC_READ | GENERIC_WRITE, 
                               FILE_SHARE_READ | FILE_SHARE_WRITE , NULL,
                               OPEN_EXISTING, 0, NULL);
      // if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
      //    printf ("Unable to open physical drive %d, error code: 0x%lX\n",
      //            drive, GetLastError ());

      if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
      {

      }
      else
      {
         GETVERSIONOUTPARAMS VersionParams;
         DWORD               cbBytesReturned = 0;

            // Get the version, etc of PhysicalDrive IOCTL
         memset ((void*) &VersionParams, 0, sizeof(VersionParams));

         if ( ! DeviceIoControl (hPhysicalDriveIOCTL, DFP_GET_VERSION,
                   NULL, 
                   0,
                   &VersionParams,
                   sizeof(VersionParams),
                   &cbBytesReturned, NULL) )
         {         

         }

            // If there is a IDE device at number "i" issue commands
            // to the device
         if (VersionParams.bIDEDeviceMap <= 0)
         {

         }
         else
         {
            BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
            SENDCMDINPARAMS  scip;
            //SENDCMDOUTPARAMS OutCmd;

			   // Now, get the ID sector for all IDE devices in the system.
               // If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
               // otherwise use the IDE_ATA_IDENTIFY command
            bIDCmd = (VersionParams.bIDEDeviceMap >> drive & 0x10) ? \
                      IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

            memset (&scip, 0, sizeof(scip));
            memset (IdOutCmd, 0, sizeof(IdOutCmd));

            if ( DoIDENTIFY (hPhysicalDriveIOCTL, 
                       &scip, 
                       (PSENDCMDOUTPARAMS)&IdOutCmd, 
                       (BYTE) bIDCmd,
                       (BYTE) drive,
                       &cbBytesReturned))
            {
               DWORD diskdata [256];
               int ijk = 0;
               USHORT *pIdSector = (USHORT *)
                             ((PSENDCMDOUTPARAMS) IdOutCmd) -> bBuffer;

               for (ijk = 0; ijk < 256; ijk++)
                  diskdata [ijk] = pIdSector [ijk];

               //PrintIdeInfo (drive, diskdata);

               done = TRUE;
            }
	    }

         CloseHandle (hPhysicalDriveIOCTL);
      }
   }

   return done;
}



//
// IDENTIFY data (from ATAPI driver source)
//

#pragma pack(1)

typedef struct _IDENTIFY_DATA {
    USHORT GeneralConfiguration;            // 00 00
    USHORT NumberOfCylinders;               // 02  1
    USHORT Reserved1;                       // 04  2
    USHORT NumberOfHeads;                   // 06  3
    USHORT UnformattedBytesPerTrack;        // 08  4
    USHORT UnformattedBytesPerSector;       // 0A  5
    USHORT SectorsPerTrack;                 // 0C  6
    USHORT VendorUnique1[3];                // 0E  7-9
    USHORT SerialNumber[10];                // 14  10-19
    USHORT BufferType;                      // 28  20
    USHORT BufferSectorSize;                // 2A  21
    USHORT NumberOfEccBytes;                // 2C  22
    USHORT FirmwareRevision[4];             // 2E  23-26
    USHORT ModelNumber[20];                 // 36  27-46
    UCHAR  MaximumBlockTransfer;            // 5E  47
    UCHAR  VendorUnique2;                   // 5F
    USHORT DoubleWordIo;                    // 60  48
    USHORT Capabilities;                    // 62  49
    USHORT Reserved2;                       // 64  50
    UCHAR  VendorUnique3;                   // 66  51
    UCHAR  PioCycleTimingMode;              // 67
    UCHAR  VendorUnique4;                   // 68  52
    UCHAR  DmaCycleTimingMode;              // 69
    USHORT TranslationFieldsValid:1;        // 6A  53
    USHORT Reserved3:15;
    USHORT NumberOfCurrentCylinders;        // 6C  54
    USHORT NumberOfCurrentHeads;            // 6E  55
    USHORT CurrentSectorsPerTrack;          // 70  56
    ULONG  CurrentSectorCapacity;           // 72  57-58
    USHORT CurrentMultiSectorSetting;       //     59
    ULONG  UserAddressableSectors;          //     60-61
    USHORT SingleWordDMASupport : 8;        //     62
    USHORT SingleWordDMAActive : 8;
    USHORT MultiWordDMASupport : 8;         //     63
    USHORT MultiWordDMAActive : 8;
    USHORT AdvancedPIOModes : 8;            //     64
    USHORT Reserved4 : 8;
    USHORT MinimumMWXferCycleTime;          //     65
    USHORT RecommendedMWXferCycleTime;      //     66
    USHORT MinimumPIOCycleTime;             //     67
    USHORT MinimumPIOCycleTimeIORDY;        //     68
    USHORT Reserved5[2];                    //     69-70
    USHORT ReleaseTimeOverlapped;           //     71
    USHORT ReleaseTimeServiceCommand;       //     72
    USHORT MajorRevision;                   //     73
    USHORT MinorRevision;                   //     74
    USHORT Reserved6[50];                   //     75-126
    USHORT SpecialFunctionsEnabled;         //     127
    USHORT Reserved7[128];                  //     128-255
} IDENTIFY_DATA, *PIDENTIFY_DATA;

#pragma pack()



int ReadPhysicalDriveInNTUsingSmart (void)
{
   int done = FALSE;
   int drive = 0;
   int ijk;

   for (drive = 0; drive < MAX_IDE_DRIVES; drive++)
   {
      HANDLE hPhysicalDriveIOCTL = 0;

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      char driveName [256];

      sprintf (driveName, "\\\\.\\PhysicalDrive%d", drive);

         //  Windows NT, Windows 2000, Windows Server 2003, Vista
      hPhysicalDriveIOCTL = CreateFile (driveName,
                               GENERIC_READ | GENERIC_WRITE, 
                               FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
							   NULL, OPEN_EXISTING, 0, NULL);
      // if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
      //    printf ("Unable to open physical drive %d, error code: 0x%lX\n",
      //            drive, GetLastError ());

      if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
      {

      }
      else
      {
         GETVERSIONINPARAMS GetVersionParams;
         DWORD cbBytesReturned = 0;

            // Get the version, etc of PhysicalDrive IOCTL
         memset ((void*) & GetVersionParams, 0, sizeof(GetVersionParams));

         if ( ! DeviceIoControl (hPhysicalDriveIOCTL, SMART_GET_VERSION,
                   NULL, 
                   0,
     			   &GetVersionParams, sizeof (GETVERSIONINPARAMS),
				   &cbBytesReturned, NULL) )
         {         

         }
         else
         {
			 	// Print the SMART version
           	// PrintVersion (& GetVersionParams);
	           // Allocate the command buffer
			ULONG CommandSize = sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE;
        	PSENDCMDINPARAMS Command = (PSENDCMDINPARAMS) malloc (CommandSize);
			DWORD BytesReturned = 0;

	           // Retrieve the IDENTIFY data
	           // Prepare the command
#define ID_CMD          0xEC            // Returns ID sector for ATA
			Command -> irDriveRegs.bCommandReg = ID_CMD;

	        if ( ! DeviceIoControl (hPhysicalDriveIOCTL, 
				                    SMART_RCV_DRIVE_DATA, Command, sizeof(SENDCMDINPARAMS),
									Command, CommandSize,
									&BytesReturned, NULL) )
            {
		           // Print the error
		        //PrintError ("SMART_RCV_DRIVE_DATA IOCTL", GetLastError());
	        } 
			else
			{
        	       // Print the IDENTIFY data
                DWORD diskdata [256];
                USHORT *pIdSector = (USHORT *)
                             (PIDENTIFY_DATA) ((PSENDCMDOUTPARAMS) Command) -> bBuffer;

                for (ijk = 0; ijk < 256; ijk++)
                   diskdata [ijk] = pIdSector [ijk];

                //PrintIdeInfo (drive, diskdata);
                done = TRUE;
			}
	           // Done
            CloseHandle (hPhysicalDriveIOCTL);
			free (Command);
		 }
      }
   }

   return done;
}


//  Required to ensure correct PhysicalDrive IOCTL structure setup
#pragma pack(4)

#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma pack(4)

	//  function to decode the serial numbers of IDE hard drives
	//  using the IOCTL_STORAGE_QUERY_PROPERTY command 
char * flipAndCodeBytes (const char * str,
			 int pos,
			 int flip,
			 char * buf)
{
   int i;
   int j = 0;
   int k = 0;

   buf [0] = '\0';
   if (pos <= 0)
      return buf;

   if ( ! j)
   {
      char p = 0;

      // First try to gather all characters representing hex digits only.
      j = 1;
      k = 0;
      buf[k] = 0;
      for (i = pos; j && str[i] != '\0'; ++i)
      {
	 char c = tolower(str[i]);

	 if (isspace(c))
	    c = '0';

	 ++p;
	 buf[k] <<= 4;

	 if (c >= '0' && c <= '9')
	    buf[k] |= (unsigned char) (c - '0');
	 else if (c >= 'a' && c <= 'f')
	    buf[k] |= (unsigned char) (c - 'a' + 10);
	 else
	 {
	    j = 0;
	    break;
	 }

	 if (p == 2)
	 {
	    if (buf[k] != '\0' && ! isprint(buf[k]))
	    {
	       j = 0;
	       break;
	    }
	    ++k;
	    p = 0;
	    buf[k] = 0;
	 }

      }
   }

   if ( ! j)
   {
      // There are non-digit characters, gather them as is.
      j = 1;
      k = 0;
      for (i = pos; j && str[i] != '\0'; ++i)
      {
	     char c = str[i];

	     if ( ! isprint(c))
	     {
	        j = 0;
	        break;
	     }

	     buf[k++] = c;
      }
   }

   if ( ! j)
   {
      // The characters are not there or are not printable.
      k = 0;
   }

   buf[k] = '\0';

   if (flip)
      // Flip adjacent characters
      for (j = 0; j < k; j += 2)
      {
	     char t = buf[j];
	     buf[j] = buf[j + 1];
	     buf[j + 1] = t;
      }

   // Trim any beginning and end space
   i = j = -1;
   for (k = 0; buf[k] != '\0'; ++k)
   {
      if (! isspace(buf[k]))
      {
	     if (i < 0)
	        i = k;
	     j = k;
      }
   }

   if ((i >= 0) && (j >= 0))
   {
      for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
         buf[k - i] = buf[k];
      buf[k - i] = '\0';
   }

   return buf;
}



#define IOCTL_DISK_GET_DRIVE_GEOMETRY_EX CTL_CODE(IOCTL_DISK_BASE, 0x0028, METHOD_BUFFERED, FILE_ANY_ACCESS)


int ReadPhysicalDriveInNTWithZeroRights (void)
{
   int done = FALSE;
   int drive = 0;

   for (drive = 0; drive < MAX_IDE_DRIVES; drive++)
   {
      HANDLE hPhysicalDriveIOCTL = 0;

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      char driveName [256];

      sprintf (driveName, "\\\\.\\PhysicalDrive%d", drive);

         //  Windows NT, Windows 2000, Windows XP - admin rights not required
      hPhysicalDriveIOCTL = CreateFile (driveName, 0,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);
      if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
      {

      }
      else
      {
		 STORAGE_PROPERTY_QUERY query;
         int cbBytesReturned = 0;
		 char buffer [10000];

         memset ((void *) & query, 0, sizeof (query));
		 query.PropertyId = StorageDeviceProperty;
		 query.QueryType = PropertyStandardQuery;

		 memset (buffer, 0, sizeof (buffer));

         if ( DeviceIoControl (hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY,
                   & query,
                   sizeof (query),
				   & buffer,
				   sizeof (buffer),
                   & cbBytesReturned, NULL) )
         {         
			 STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR *) & buffer;
			 char serialNumber [1000];
			 char modelNumber [1000];
             char vendorId [1000];
	         char productRevision [1000];


             flipAndCodeBytes (buffer,
                               descrip -> VendorIdOffset,
			                   0, vendorId );
	         flipAndCodeBytes (buffer,
			                   descrip -> ProductIdOffset,
			                   0, modelNumber );
	         flipAndCodeBytes (buffer,
			                   descrip -> ProductRevisionOffset,
			                   0, productRevision );
	         flipAndCodeBytes (buffer,
			                   descrip -> SerialNumberOffset,
			                   1, serialNumber );

			 if (0 == HardDriveSerialNumber [0] &&
						//  serial number must be alphanumeric
			            //  (but there can be leading spaces on IBM drives)
				   (isalnum (serialNumber [0]) || isalnum (serialNumber [19])))
			 {
				strcpy (HardDriveSerialNumber, serialNumber);
				strcpy (HardDriveModelNumber, modelNumber);
				done = TRUE;
			 }
#ifdef PRINTING_TO_CONSOLE_ALLOWED
             printf ("\n**** STORAGE_DEVICE_DESCRIPTOR for drive %d ****\n"
		             "Vendor Id = [%s]\n"
		             "Product Id = [%s]\n"
		             "Product Revision = [%s]\n"
		             "Serial Number = [%s]\n",
		             drive,
		             vendorId,
		             modelNumber,
		             productRevision,
		             serialNumber);
#endif
	           // Get the disk drive geometry.
	         memset (buffer, 0, sizeof(buffer));
	         if ( ! DeviceIoControl (hPhysicalDriveIOCTL,
			          IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
			          NULL,
			          0,
			          &buffer,
			          sizeof(buffer),
			          &cbBytesReturned,
			          NULL))
	         {

          	 }
	         else
	         {         
	            DISK_GEOMETRY_EX* geom = (DISK_GEOMETRY_EX*) &buffer;
	            int fixed = (geom->Geometry.MediaType == FixedMedia);
	            __int64 size = geom->DiskSize.QuadPart;
				     
#ifdef PRINTING_TO_CONSOLE_ALLOWED
	            printf ("\n**** DISK_GEOMETRY_EX for drive %d ****\n"
		                "Disk is%s fixed\n"
		                "DiskSize = %I64d\n",
		                drive,
		                fixed ? "" : " NOT",
		                size);
#endif
	        }
         }
		 else
		 {
			 DWORD err = GetLastError ();
#ifdef PRINTING_TO_CONSOLE_ALLOWED
			 printf ("\nDeviceIOControl IOCTL_STORAGE_QUERY_PROPERTY error = %d\n", err);
#endif
		 }

         CloseHandle (hPhysicalDriveIOCTL);
      }
   }

   return done;
}


   // DoIDENTIFY
   // FUNCTION: Send an IDENTIFY command to the drive
   // bDriveNum = 0-3
   // bIDCmd = IDE_ATA_IDENTIFY or IDE_ATAPI_IDENTIFY
BOOL DoIDENTIFY (HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
                 PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
                 PDWORD lpcbBytesReturned)
{
      // Set up data structures for IDENTIFY command.
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;
   pSCIP -> irDriveRegs.bFeaturesReg = 0;
   pSCIP -> irDriveRegs.bSectorCountReg = 1;
   //pSCIP -> irDriveRegs.bSectorNumberReg = 1;
   pSCIP -> irDriveRegs.bCylLowReg = 0;
   pSCIP -> irDriveRegs.bCylHighReg = 0;

      // Compute the drive number.
   pSCIP -> irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

      // The command can either be IDE identify or ATAPI identify.
   pSCIP -> irDriveRegs.bCommandReg = bIDCmd;
   pSCIP -> bDriveNumber = bDriveNum;
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;

   return ( DeviceIoControl (hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
               (LPVOID) pSCIP,
               sizeof(SENDCMDINPARAMS) - 1,
               (LPVOID) pSCOP,
               sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
               lpcbBytesReturned, NULL) );
}


//  ---------------------------------------------------

   // (* Output Bbuffer for the VxD (rt_IdeDinfo record) *)
typedef struct _rt_IdeDInfo_
{
    BYTE IDEExists[4];
    BYTE DiskExists[8];
    WORD DisksRawInfo[8*256];
} rt_IdeDInfo, *pt_IdeDInfo;



#define  m_cVxDFunctionIdesDInfo  1


//  ---------------------------------------------------


int ReadDrivePortsInWin9X (void)
{
   int done = FALSE;
   unsigned long int i = 0;

   HANDLE VxDHandle = 0;
   pt_IdeDInfo pOutBufVxD = 0;
   DWORD lpBytesReturned = 0;

   BOOL status;

   // 1. Make an output buffer for the VxD
   rt_IdeDInfo info;
   pOutBufVxD = &info;

		//  set the thread priority high so that we get exclusive access to the disk
   status =
		// SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
		SetPriorityClass (GetCurrentProcess (), REALTIME_PRIORITY_CLASS);
		// SetPriorityClass (GetCurrentProcess (), HIGH_PRIORITY_CLASS);



      // *****************
      // KLUDGE WARNING!!!
      // HAVE to zero out the buffer space for the IDE information!
      // If this is NOT done then garbage could be in the memory
      // locations indicating if a disk exists or not.
   ZeroMemory (&info, sizeof(info));

      // 1. Try to load the VxD
       //  must use the short file name path to open a VXD file
   VxDHandle = CreateFile ("\\\\.\\IDE21201.VXD", 0, 0, 0,
							0, FILE_FLAG_DELETE_ON_CLOSE, 0);

   if (VxDHandle != INVALID_HANDLE_VALUE)
   {
         // 2. Run VxD function
      DeviceIoControl (VxDHandle, m_cVxDFunctionIdesDInfo,
					0, 0, pOutBufVxD, sizeof(pt_IdeDInfo), &lpBytesReturned, 0);

         // 3. Unload VxD
      CloseHandle (VxDHandle);
   }
//   else
//		MessageBox (NULL, "ERROR: Could not open IDE21201.VXD file", 
//					TITLE, MB_ICONSTOP);

      // 4. Translate and store data
   for (i=0; i<8; i++)
   {
      if((pOutBufVxD->DiskExists[i]) && (pOutBufVxD->IDEExists[i/2]))
      {
			int j;
			DWORD diskinfo [256];
			for ( j = 0; j < 256; j++) 
				diskinfo [j] = pOutBufVxD -> DisksRawInfo [i * 256 + j];

            // process the information for this buffer
		   //PrintIdeInfo (i, diskinfo);
			done = TRUE;
      }
   }

		//  reset the thread priority back to normal
   // SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_NORMAL);
   SetPriorityClass (GetCurrentProcess (), NORMAL_PRIORITY_CLASS);

   return done;
}


#define  SENDIDLENGTH  sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE


int ReadIdeDriveAsScsiDriveInNT (void)
{
   int done = FALSE;
   int controller = 0;

   for (controller = 0; controller < 16; controller++)
   {
      HANDLE hScsiDriveIOCTL = 0;
      char   driveName [256];

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      sprintf (driveName, "\\\\.\\Scsi%d:", controller);

         //  Windows NT, Windows 2000, any rights should do
      hScsiDriveIOCTL = CreateFile (driveName,
                               GENERIC_READ | GENERIC_WRITE, 
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);
      // if (hScsiDriveIOCTL == INVALID_HANDLE_VALUE)
      //    printf ("Unable to open SCSI controller %d, error code: 0x%lX\n",
      //            controller, GetLastError ());

      if (hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
      {
         int drive = 0;

         for (drive = 0; drive < 2; drive++)
         {
            char buffer [sizeof (SRB_IO_CONTROL) + SENDIDLENGTH];
            SRB_IO_CONTROL *p = (SRB_IO_CONTROL *) buffer;
            SENDCMDINPARAMS *pin =
                   (SENDCMDINPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
            DWORD dummy;
   
            memset (buffer, 0, sizeof (buffer));
            p -> HeaderLength = sizeof (SRB_IO_CONTROL);
            p -> Timeout = 10000;
            p -> Length = SENDIDLENGTH;
            p -> ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
            strncpy ((char *) p -> Signature, "SCSIDISK", 8);
  
            pin -> irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
            pin -> bDriveNumber = drive;

            if (DeviceIoControl (hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT, 
                                 buffer,
                                 sizeof (SRB_IO_CONTROL) +
                                         sizeof (SENDCMDINPARAMS) - 1,
                                 buffer,
                                 sizeof (SRB_IO_CONTROL) + SENDIDLENGTH,
                                 &dummy, NULL))
            {
               SENDCMDOUTPARAMS *pOut =
                    (SENDCMDOUTPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
               IDSECTOR *pId = (IDSECTOR *) (pOut -> bBuffer);
               if (pId -> sModelNumber [0])
               {
                  DWORD diskdata [256];
                  int ijk = 0;
                  USHORT *pIdSector = (USHORT *) pId;
          
                  for (ijk = 0; ijk < 256; ijk++)
                     diskdata [ijk] = pIdSector [ijk];

                  //PrintIdeInfo (controller * 2 + drive, diskdata);

                  done = TRUE;
               }
            }
         }
         CloseHandle (hScsiDriveIOCTL);
      }
   }

   return done;
}


//char *workedmethod;
//long getHardDriveComputerID ()
void getHardDriveComputerID ( void )
{
   int done = FALSE;
   // char string [1024];
   __int64 id = 0;
   OSVERSIONINFO version;

	strcpy (HardDriveSerialNumber, "");
	strcpy (HardDriveModelNumber, "");

	HardDriveSerialNumber[0] = 0;
	HardDriveModelNumber[0] = 0;


   memset (&version, 0, sizeof (version));
   version.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
   GetVersionEx (&version);
   if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		//  this works under WinNT4 or Win2K if you have admin rights

		//printf ("\nTrying to read the drive IDs using physical access with admin rights\n");
		done = ReadPhysicalDriveInNTWithAdminRights ();
		//workedmethod = "ReadPhysicalDriveInNTWithAdminRights";


		//  this should work in WinNT or Win2K if previous did not work
		//  this is kind of a backdoor via the SCSI mini port driver into
		//     the IDE drives

		//printf ("\nTrying to read the drive IDs using the SCSI back door\n");
		//if ( ! done) {
			done = ReadIdeDriveAsScsiDriveInNT ();
		//	workedmethod = "ReadIdeDriveAsScsiDriveInNT";
		//}


		//  this works under WinNT4 or Win2K or WinXP if you have any rights
		//printf ("\nTrying to read the drive IDs using physical access with zero rights\n");
		//if ( ! done) {
			done = ReadPhysicalDriveInNTWithZeroRights ();
		//	workedmethod = "ReadPhysicalDriveInNTWithZeroRights";
		//}


		//  this works under WinNT4 or Win2K or WinXP or Windows Server 2003 or Vista if you have any rights

		//printf ("\nTrying to read the drive IDs using Smart\n");
		//if ( ! done) {
			done = ReadPhysicalDriveInNTUsingSmart ();
		//	workedmethod = "ReadPhysicalDriveInNTUsingSmart";
		//}
   }
   else
   {
         //  this works under Win9X and calls a VXD
      int attempt = 0;

         //  try this up to 10 times to get a hard drive serial number
      for (attempt = 0;
           attempt < 10 && ! done && 0 == HardDriveSerialNumber [0];
		   attempt++) {
         done = ReadDrivePortsInWin9X ();
		//workedmethod = "ReadDrivePortsInWin9X";

	  }
   }


   if ( ! done) {
		//return -1;
		return;
		//workedmethod = "none";
   }

}


char *GetUUID_WIN32( void )
{
	//long id = getHardDriveComputerID();
	char *ids;
	char UUID [33];
	char *HDserial;
	char *HDmodel;
	char HDserialEdit [33]; // alphanumeric
	char HDmodelEdit [33]; // alphanumeric
	int slen;
	int mlen;
	int i, j, k;


	ids = "";

	// get info
	getHardDriveComputerID();


	if(!(HardDriveSerialNumber [0])) {
		//return '\0';
		char *macadr = GetMAC();
		Q_strncpyz( HardDriveSerialNumber, macadr, sizeof(HardDriveSerialNumber) );
		if(!( HardDriveSerialNumber[0] )) {
			return "";
		}
	}


	HDserial = HardDriveSerialNumber;
	HDmodel = HardDriveModelNumber;
	slen = strlen( HDserial );
	mlen = strlen( HDmodel );
	if( slen > 32 )
		slen = 32;
	if( mlen > 32 )
		mlen = 32;


// -----------------------------------------------------
	strcpy (HDserialEdit, "");
	j = 0;
	for( i=0; i<slen; i++ ) {
		if( Q_isalphanumeric( HardDriveSerialNumber[i] ) ) {
			HDserialEdit[j] = HardDriveSerialNumber[i];
			j++;
		} else {
			HDserialEdit[j] = ':';
			j++;
		}
	}
	HDserialEdit[j] = '\0';


	strcpy (HDmodelEdit, "");
	j = 0;
	for( i=0; i<mlen; i++ ) {
		if( Q_isalphanumeric( HardDriveModelNumber[i] ) ) {
			HDmodelEdit[j] = HardDriveModelNumber[i];
			j++;
		} else {
			HDmodelEdit[j] = '#';
			j++;
		}
	}
	HDmodelEdit[j] = '\0';
// -----------------------------------------------------


	Q_strncpyz( UUID, HDserialEdit, sizeof(UUID) );

	if( slen < 32 ) {
		for( k=0; ((k<mlen) && ((slen+k)<32)); k++ ) {
			UUID[slen+k] = HDmodelEdit[k];
		}
		UUID[slen+k] = '\0';
	}

	//ids = UUID;
	ids = va("%s", UUID);


	//CG_Printf("^1*GetUUID: ^5%s\n", ids);

	return ids;
}



// #############################################################################################
// #############################################################################################


// GetMACAdapters.cpp : Defines the entry point for the console application.
//
// Author:	Khalid Shaikh [Shake@ShakeNet.com]
// Date:	April 5th, 2002
//
// This program fetches the MAC address of the localhost by fetching the 
// information through GetAdapatersInfo.  It does not rely on the NETBIOS
// protocol and the ethernet adapter need not be connect to a network.
//
// Supported in Windows NT/2000/XP
// Supported in Windows 95/98/Me
//
// Supports multiple NIC cards on a PC.

#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")


char *ConvertMACaddress(unsigned char MACData[])
{
   static char string [256];
//#ifdef PRINTING_TO_CONSOLE_ALLOWED
//	CG_Printf("MAC Address: %02X-%02X-%02X-%02X-%02X-%02X\n", 
//		MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
//#endif

   //sprintf (string, "%02X-%02X-%02X-%02X-%02X-%02X", MACData[0], MACData[1], 
	//		   MACData[2], MACData[3], MACData[4], MACData[5]);

   Com_sprintf(string, sizeof(string), "%02X-%02X-%02X-%02X-%02X-%02X", MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
   return string;
}


// Fetches the MAC address and prints it
char *GetMACaddress( void )
{
	char *MACaddress;
	IP_ADAPTER_INFO AdapterInfo[16];		// Allocate information
											// for up to 16 NICs

	DWORD dwBufLen = sizeof(AdapterInfo);	// Save memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(		// Call GetAdapterInfo
		AdapterInfo,						// [out] buffer to receive data
		&dwBufLen);							// [in] size of receive data buffer

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;	// Contains pointer to
													// current adapter info

	assert(dwStatus == ERROR_SUCCESS);	// Verify return value is
										// valid, no buffer overflow

	if( dwStatus != ERROR_SUCCESS )
		return '\0';

	do {
		MACaddress = ConvertMACaddress( pAdapterInfo->Address );

		pAdapterInfo = pAdapterInfo->Next;    // Progress through linked list
	}
	while(pAdapterInfo);                    // Terminate if last adapter

	return MACaddress;
}


#endif // #ifdef WIN32


// #############################################################################################
// #############################################################################################
// #############################################################################################



char *GetMserial( void )
{
	char *out;

#ifdef WIN32
	getHardDriveComputerID();
	out = HardDriveSerialNumber;
#else
	out = "";
#endif

	return out;
}


char *GetMmodel( void )
{
	char *out;

#ifdef WIN32
	getHardDriveComputerID();
	out = HardDriveModelNumber;
#else
	out = "";
#endif


	return out;
}


#ifdef __linux__

#include <fcntl.h> // open
#include <unistd.h> //close()

#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <errno.h>



//std::string getDiskSerialNumber()
char *getDiskSN_linux( void )
{
	struct hd_driveid id;
	int fd;
	char *testHDS;

	if ((fd = open("/dev/hda", O_RDONLY)) == -1)
		return "error_opening"; //error opening /dev/hda
   
	if (ioctl(fd, HDIO_GET_IDENTITY, &id) == -1)
		return "error_retrieving"; //error retrieving indentification
   
	//return std::string((const char*)id.serial_no);

	testHDS = va("%s", id.serial_no);

	CG_Printf("^6Serial number - %s", testHDS);

	return testHDS;
}


//#include <stdio.h>
//#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>

unsigned char cMacAddr[8]; // Server's MAC address

static int GetSvrMacAddress( char *pIface )
{
	int nSD; // Socket descriptor
	struct ifreq sIfReq; // Interface request
	struct if_nameindex *pIfList; // Ptr to interface name index
	struct if_nameindex *pListSave; // Ptr to interface name index

	// Initialize this function
	pIfList = (struct if_nameindex *)NULL;
	pListSave = (struct if_nameindex *)NULL;
	#ifndef SIOCGIFADDR
	// The kernel does not support the required ioctls
	return( 0 );
	#endif

	// Create a socket that we can use for all of our ioctls
	nSD = socket( PF_INET, SOCK_STREAM, 0 );
	if ( nSD < 0 )
	{
		// Socket creation failed, this is a fatal error
		CG_Printf( "File %s: line %d: Socket failed\n", __FILE__, __LINE__ );
		return( 0 );
	}

	// Obtain a list of dynamically allocated structures
	pIfList = pListSave = if_nameindex();

	// Walk thru the array returned and query for each interface's
	// address
	//for ( pIfList; *(char *)pIfList != 0; pIfList++ )
	for ( ; *(char *)pIfList != 0; pIfList++ )
	{
		// Determine if we are processing the interface that we
		// are interested in
		if ( strcmp(pIfList->if_name, pIface) )
			continue;

		strncpy( sIfReq.ifr_name, pIfList->if_name, IF_NAMESIZE );

		// Get the MAC address for this interface
		if ( ioctl(nSD, SIOCGIFHWADDR, &sIfReq) != 0 )
		{
			// We failed to get the MAC address for the interface
			CG_Printf( "File %s: line %d: Ioctl failed\n", __FILE__, __LINE__ );
			return( 0 );
		}
		memmove( (void *)&cMacAddr[0], (void *)&sIfReq.ifr_ifru.ifru_hwaddr.sa_data[0], 6 );
		break;
	}


	if_freenameindex( pListSave );
		close( nSD );

	return( 1 );
}

//int main( int argc, char * argv[] )
char *getMAC_linux( void )
{
	char *macaddress;
	bzero( (void *)&cMacAddr[0], sizeof(cMacAddr) );

	macaddress = "";

	if ( !GetSvrMacAddress("eth0") )
	{
		// We failed to get the local host's MAC address
		CG_Printf( "Fatal error: Failed to get local host's MAC address\n" );
	}
	//printf
	CG_Printf( "HWaddr %02X:%02X:%02X:%02X:%02X:%02X\n",
	cMacAddr[0], cMacAddr[1], cMacAddr[2],
	cMacAddr[3], cMacAddr[4], cMacAddr[5] );
	
	Com_Printf( "HWaddr %02X:%02X:%02X:%02X:%02X:%02X\n",
	cMacAddr[0], cMacAddr[1], cMacAddr[2],
	cMacAddr[3], cMacAddr[4], cMacAddr[5] );

	macaddress = va( "HWaddr %02X:%02X:%02X:%02X:%02X:%02X\n",
	cMacAddr[0], cMacAddr[1], cMacAddr[2],
	cMacAddr[3], cMacAddr[4], cMacAddr[5] );

	return macaddress;
}


#endif //#ifdef __linux__


// ++++++++++++++++++++++++++++++++++++++++++++++++++++


char *GetMAC( void )
{
	char *mac;

#ifdef WIN32
	mac = GetMACaddress();
#elif __linux__
	mac = getMAC_linux();
#else
	mac = "";
#endif

	return mac;
}


char *GetUUID_Encrypt( void )
{
	char *uuid;
	char *uuidout;
	char in[33]; // trailing 0
	char edit[32];
	char out[32];
	int i, j, k;
	int fulllen;
	int halflen;

	uuid = "";

	CG_Printf("^1*UUID_Enc\n");

#ifdef WIN32
	uuid = GetUUID_WIN32();
#elif __linux__

	CG_Printf("^1*UUID_Enc: __linux__^5\n");

	return getDiskSN_linux();

//	return "NOUUID_LINUX";
	//return '\0';
#else
	return "NOUUID_UNIX";
	//return '\0';
#endif

	return "NOUUID_WIN32";

	if( !uuid || !*uuid ) {
		return '\0';
	}
	//CG_Printf("^1*GetUUID_enc: inuuid: ^5%s\n", uuid);

	//Q_strncpyz( in, uuid, sizeof(in) );
	strcpy( in, uuid );
	in[strlen(uuid)] = 0;

	if( !in[0] ) {
		return '\0';
	}

	//CG_Printf("^1*GetUUID_enc: in: ^5%s\n", in);

	strcpy( edit, "" );
	for ( i=0; i<(int)strlen(in); i++ )
	{
		switch (in[i])
		{
            case '0': edit[i] = 'X'; break;
            case '1': edit[i] = 'A'; break;
            case '2': edit[i] = 'Y'; break;
            case '3': edit[i] = '9'; break;
            case '4': edit[i] = 'T'; break;
            case '5': edit[i] = 'G'; break;
            case '6': edit[i] = 'O'; break;
            case '7': edit[i] = 'P'; break;
            case '8': edit[i] = 'I'; break;
            case '9': edit[i] = '5'; break;
            case 'a': case 'A': edit[i] = 'D'; break;
            case 'b': case 'B': edit[i] = '6'; break;
            case 'c': case 'C': edit[i] = 'Q'; break;
            case 'd': case 'D': edit[i] = 'B'; break;
            case 'e': case 'E': edit[i] = 'R'; break;
            case 'f': case 'F': edit[i] = '2'; break;
            case 'g': case 'G': edit[i] = 'H'; break;
            case 'h': case 'H': edit[i] = 'S'; break;
            case 'i': case 'I': edit[i] = '3'; break;
            case 'j': case 'J': edit[i] = 'F'; break;
            case 'k': case 'K': edit[i] = '8'; break;
            case 'l': case 'L': edit[i] = 'L'; break;
            case 'm': case 'M': edit[i] = '1'; break;
            case 'n': case 'N': edit[i] = 'U'; break;
            case 'o': case 'O': edit[i] = 'C'; break;
            case 'p': case 'P': edit[i] = 'Z'; break;
            case 'q': case 'Q': edit[i] = '0'; break;
            case 'r': case 'R': edit[i] = 'V'; break;
            case 's': case 'S': edit[i] = '7'; break;
            case 't': case 'T': edit[i] = 'W'; break;
            case 'u': case 'U': edit[i] = 'N'; break;
            case 'v': case 'V': edit[i] = 'E'; break;
            case 'w': case 'W': edit[i] = 'K'; break;
			case 'x': case 'X': edit[i] = '#'; break;
            case 'y': case 'Y': edit[i] = '4'; break;
			case 'z': case 'Z': edit[i] = 'J'; break;

			case '#': edit[i] = 'M'; break;

			// no spaces or wierd characters
			default: edit[i] = '_'; break;
		}
	}
	edit[i] = '\0';

	// --------------------------------
	strcpy( out, "" );
	out[0] = 0;

	fulllen = strlen(edit);
	halflen = (int)floor(fulllen*0.5);

	k = 0;
	for( j=halflen; j<fulllen; j++ ) {
		out[k] = edit[j];
		k++;
	}

	for( j=0; j<halflen; j++ ) {
		out[k] = edit[j];
		k++;
	}
	if( k > 32 ) { return '\0'; }
	out[k] = '\0';
	// --------------------------------

	//uuidout = va("%s",out);
	uuidout = out;

	if( !uuidout || !*uuidout ) { return '\0'; }

	//CG_Printf( "^1UUID: ^7\'%s\' (%d)\n", uuidout, strlen(uuidout) );

	return uuidout;
}


#endif
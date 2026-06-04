#pragma once


#include <stdint.h>


#define CFE_FS_HDR_DESC_MAX_LEN         32
#define DS_TOTAL_FNAME_BUFSIZE          64


typedef struct CFE_FS_Header
{
    uint32_t ContentType;   /**< \brief Identifies the content type (='cFE1'=0x63464531)*/
    uint32_t SubType;       /**< \brief Type of \c ContentType, if necessary */
                          /**< Standard SubType definitions can be found
                               \link #CFE_FS_SubType_ES_ERLOG here \endlink */
    uint32_t Length;        /**< \brief Length of this header to support external processing */
    uint32_t SpacecraftID;  /**< \brief Spacecraft that generated the file */
    uint32_t ProcessorID;   /**< \brief Processor that generated the file */
    uint32_t ApplicationID; /**< \brief Application that generated the file */

    uint32_t TimeSeconds;    /**< \brief File creation timestamp (seconds) */
    uint32_t TimeSubSeconds; /**< \brief File creation timestamp (sub-seconds) */

    char Description[CFE_FS_HDR_DESC_MAX_LEN]; /**< \brief File description */

} CFE_FS_Header_t;


typedef struct
{
    uint32_t  CloseSeconds;                               /**< \brief Time when file was closed */
    uint32_t  CloseSubsecs;        

    uint16_t  FileTableIndex;                             /**< \brief Destination file table index */
    uint16_t  FileNameType;                               /**< \brief Filename type - count vs time */

    char    FileName[DS_TOTAL_FNAME_BUFSIZE];           /**< \brief On-board filename */

} DS_FileHeader_t;



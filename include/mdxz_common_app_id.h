/*** 
 * @Author       : lv zhipeng
 * @Date         : 2023-03-13 15:53:07
 * @LastEditors  : lv zhipeng
 * @LastEditTime : 2023-03-14 15:17:14
 * @FilePath     : /LIBS/include/mdxz_common_app_id.h
 * @Description  : This document contains the definitions of all MDXZ 
 * APP IDs, which are used for IPC and other scenarios.
 * @
 * @Copyright (c) 2023 by MDXZ, All Rights Reserved. 
 */

#ifndef __MDXZ_COMMON_APP_ID_H
#define __MDXZ_COMMON_APP_ID_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    /*----------common----------------------------------------------------*/
    MDXZ_APP_ID_MIN    = -1,
    MDXZ_APP_IPC_TESTA = 1,
    MDXZ_APP_IPC_TESTB = 2,

}MdxzAppId_e;


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif


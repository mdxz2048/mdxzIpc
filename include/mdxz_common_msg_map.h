#ifndef _MDXZ_COMMON_MSG_MAP_H_
#define _MDXZ_COMMON_MSG_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MDXZ_APP_MSG_START                                = -1,
    MDXZ_APP_MSG_RESERV_START                         = 1,
    MDXZ_APP_MSG_RESERV_END                           = 200,
	/* add by lvzhipeng for ipc test.20230314 begin */
	MDXZ_APP_IPC_MSG_NORSP							   = 201,
	/* add by lvzhipeng for ipc test.20230314 end */
    MDXZ_APP_MSG_END                                  = 10000,
}MdxzIpcMsg_e;

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif


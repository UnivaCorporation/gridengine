#ifndef __SGE_SCHEDD_TEXT_H
#define __SGE_SCHEDD_TEXT_H
/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

/*
** Code was former
** in sge_text.h ( sge_text() is now sge_schedd_text() )
*/

/* Info strings used in scheduler (detailed information)
* and Info strings used in qstat (we group jobs)
*
* Info strings used in scheduler and info strings used in qstat must
* have same sequence because we use offset to map messages
*/
enum { 
   SCHEDD_INFO_CANNOTRUNATHOST_SSS = 0 ,
   SCHEDD_INFO_HASNOPERMISSION_SS,
   SCHEDD_INFO_HASINCORRECTPRJ_SSS,
   SCHEDD_INFO_HASNOPRJ_S,
   SCHEDD_INFO_EXCLPRJ_SSS,
   SCHEDD_INFO_QUEUENOTREQUESTABLE_S,
   SCHEDD_INFO_NOTINHARDQUEUELST_S,
   SCHEDD_INFO_NOTPARALLELQUEUE_S,
   SCHEDD_INFO_NOTINQUEUELSTOFPE_SS,
   SCHEDD_INFO_NOTACKPTQUEUE_SS,
   SCHEDD_INFO_NOTINQUEUELSTOFCKPT_SS,
   SCHEDD_INFO_QUEUENOTINTERACTIVE_S,
   SCHEDD_INFO_NOTASERIALQUEUE_S,
   SCHEDD_INFO_NOTPARALLELJOB_S,
   SCHEDD_INFO_NOTREQFORCEDRES_SS,
   SCHEDD_INFO_WOULDSETQEUEINALARM_DS,
   SCHEDD_INFO_NOSLOTSINQUEUE_S,
   SCHEDD_INFO_CANNOTRUNINQUEUE_SSS,
   SCHEDD_INFO_NORESOURCESPE_,
   SCHEDD_INFO_CANNOTRUNGLOBALLY_SS,
   SCHEDD_INFO_NOFORCEDRES_SS,
   SCHEDD_INFO_NOGLOBFORCEDRES_SS,
   SCHEDD_INFO_CKPTNOTFOUND_,
   SCHEDD_INFO_PESLOTSNOTINRANGE_S,
   SCHEDD_INFO_TOTALPESLOTSNOTINRANGE_S,
   SCHEDD_INFO_NOACCESSTOPE_S,
   SCHEDD_INFO_QUEUEINALARM_SS,
   SCHEDD_INFO_QUEUEOVERLOADED_SS,
   SCHEDD_INFO_ALLALARMOVERLOADED_,
   SCHEDD_INFO_TURNEDOFF_,
   SCHEDD_INFO_JOBLIST_,
   SCHEDD_INFO_EXECTIME_,
   SCHEDD_INFO_JOBINERROR_,
   SCHEDD_INFO_JOBHOLD_,
   SCHEDD_INFO_USRGRPLIMIT_,
   SCHEDD_INFO_JOBDEPEND_,
   SCHEDD_INFO_NOMESSAGE_,
   SCHEDD_INFO_QUEUEFULL_,
   SCHEDD_INFO_QUEUESUSP_,
   SCHEDD_INFO_QUEUEDISABLED_, 
   SCHEDD_INFO_QUEUENOTAVAIL_,
   SCHEDD_INFO_INSUFFICIENTSLOTS_,
   SCHEDD_INFO_PEALLOCRULE_S,
   SCHEDD_INFO_NOPEMATCH_,
   SCHEDD_INFO_CLEANUPNECESSARY_S,
   SCHEDD_INFO_MAX_AJ_INSTANCES_,

   /* */
   SCHEDD_INFO_CANNOTRUNATHOST,
   SCHEDD_INFO_HASNOPERMISSION,
   SCHEDD_INFO_HASINCORRECTPRJ,
   SCHEDD_INFO_HASNOPRJ,
   SCHEDD_INFO_EXCLPRJ,
   SCHEDD_INFO_QUEUENOTREQUESTABLE,
   SCHEDD_INFO_NOTINHARDQUEUELST,
   SCHEDD_INFO_NOTPARALLELQUEUE,
   SCHEDD_INFO_NOTINQUEUELSTOFPE,
   SCHEDD_INFO_NOTACKPTQUEUE,
   SCHEDD_INFO_NOTINQUEUELSTOFCKPT,
   SCHEDD_INFO_QUEUENOTINTERACTIVE,
   SCHEDD_INFO_NOTASERIALQUEUE,
   SCHEDD_INFO_NOTPARALLELJOB,
   SCHEDD_INFO_NOTREQFORCEDRES,
   SCHEDD_INFO_WOULDSETQEUEINALARM,
   SCHEDD_INFO_NOSLOTSINQUEUE,
   SCHEDD_INFO_CANNOTRUNINQUEUE,
   SCHEDD_INFO_NORESOURCESPE,
   SCHEDD_INFO_CANNOTRUNGLOBALLY,
   SCHEDD_INFO_NOFORCEDRES,
   SCHEDD_INFO_NOGLOBFORCEDRES,
   SCHEDD_INFO_CKPTNOTFOUND,
   SCHEDD_INFO_PESLOTSNOTINRANGE,
   SCHEDD_INFO_TOTALPESLOTSNOTINRANGE,
   SCHEDD_INFO_NOACCESSTOPE,
   SCHEDD_INFO_QUEUEINALARM,
   SCHEDD_INFO_QUEUEOVERLOADED,
   SCHEDD_INFO_ALLALARMOVERLOADED,
   SCHEDD_INFO_TURNEDOFF,
   SCHEDD_INFO_JOBLIST,
   SCHEDD_INFO_EXECTIME,
   SCHEDD_INFO_JOBINERROR,
   SCHEDD_INFO_JOBHOLD,
   SCHEDD_INFO_USRGRPLIMIT,
   SCHEDD_INFO_JOBDEPEND,
   SCHEDD_INFO_NOMESSAGE,
   SCHEDD_INFO_QUEUEFULL,
   SCHEDD_INFO_QUEUESUSP,
   SCHEDD_INFO_QUEUEDISABLED,
   SCHEDD_INFO_QUEUENOTAVAIL,
   SCHEDD_INFO_INSUFFICIENTSLOTS,
   SCHEDD_INFO_PEALLOCRULE,
   SCHEDD_INFO_NOPEMATCH,
   SCHEDD_INFO_CLEANUPNECESSARY,
   SCHEDD_INFO_MAX_AJ_INSTANCES,

   TOOBIG /* don't move from last position! */
};
#define SCHEDD_INFO_OFFSET (SCHEDD_INFO_CANNOTRUNATHOST-SCHEDD_INFO_CANNOTRUNATHOST_SSS)

const char *sge_schedd_text(int number);

#endif /* __SGE_SCHEDD_TEXT_H */


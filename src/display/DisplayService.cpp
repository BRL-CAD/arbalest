//
// Created by sadeep on 6/28/19.
//

#ifndef DisplayServiceL
#define DisplayServiceL

#include <bn.h>
#include "raytrace.h"
#include <brlcad/Halfspace.h>
#include <brlcad/Ellipsoid.h>

#include <brlcad/ConstDatabase.h>
#include <brlcad/MemoryDatabase.h>
#include <brlcad/Database.h>

using namespace BRLCAD;

struct bu_list vhead;

class DisplayService{
public:
    static  bn_vlist * getVList(MemoryDatabase*  m_database){

        struct rt_view_info info;
        BU_LIST_INIT(&vhead);

        rt_i*  m_rtip = m_database->m_rtip;
        resource* m_resp = m_database->m_resp;
        rt_wdb* m_wdbp = m_database->m_wdbp;


        BRLCAD::ConstDatabase::TopObjectIterator it = m_database->FirstTopObject();
        directory* pDir;
        struct rt_db_internal intern;
        while (it.Good()) {

            pDir = db_lookup(m_rtip->rti_dbip, it.Name(), 0);
            rt_db_get_internal(&intern, pDir, m_rtip->rti_dbip, 0, m_resp);
            if(intern.idb_meth && intern.idb_meth->ft_plot)
                intern.idb_meth->ft_plot(&vhead,&intern,m_wdbp->wdb_initial_tree_state.ts_ttol,m_wdbp->wdb_initial_tree_state.ts_tol,&info);



            ++it;
        }


        bn_vlist *vlist =  ((struct bn_vlist *)&vhead);

        return vlist;
    }
};


#endif
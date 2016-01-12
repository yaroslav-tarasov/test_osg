#include "stdafx.h" 

#include "net_layer/net_worker.h"
#include "session_impl.h"

namespace net_layer
{
    using namespace net;

   ses_srv*    create_session  (binary::bytes_cref data, bool local, double initial_time)
   {
       return new session_impl(
#if 0
           server_,
           boost::bind(&impl::session_send, this, cs.name, _1, _2, local), 
           boost::bind(&time_synchronizer::time, &sync_), 
           local, 
#endif
           session_impl::time_func_f(),
           initial_time
#if 0
           boost::bind(&net_srv::impl::safe_call, this, _1)
#endif
           );
   }     

}


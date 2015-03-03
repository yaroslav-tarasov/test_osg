#pragma once
// #include "systems.h"
// #include "systems/systems_base.h"

#include "net_layer/net_server.h"
//#include "common/qt_dispatch.h"
#include "reflection/proc/binary.h"


namespace kernel
{

#pragma pack(push, 1)

struct kernel_msg
{
    binary::size_type sys_kind;
    binary::bytes_t   bytes   ;

    kernel_msg(binary::size_type sys_kind, binary::bytes_cref bytes)
        : sys_kind  (sys_kind)
        , bytes     (bytes   )
    {
    }

    kernel_msg() {}
};

#pragma pack(pop)

REFL_STRUCT(kernel_msg)
    REFL_ENTRY(sys_kind)
    REFL_ENTRY(bytes   )
REFL_END()
} // kernel


namespace kernel 
{

//! сервис обмена сообщениями
struct msg_service
{
    typedef function<void(binary::bytes_cref /*bytes*/, bool /*sure*/)> remote_send_f;

    msg_service(net_layer::ses_srv* srv = 0)
        : ses_srv_    (srv)
        , remote_send_(srv ? boost::bind(&net_layer::ses_srv::send, srv, _1, _2) : remote_send_f())
    {
    }

    msg_service(remote_send_f const& f)
        : ses_srv_    (nullptr)
        , remote_send_(f)
    {
    }

    void register_sys   (system* sys) { clients_.insert(sys); }
    void unregister_sys (system* sys) { clients_.erase (sys); }

    void send_message(system_kind kind, binary::bytes_cref bytes, bool sure)
    {
        using namespace net_layer;

        // sending to other clients of this network client
        bool was_empty = msg_queue_.empty();
        msg_queue_.emplace(send_atom(bytes, kind, sure));

        if (was_empty)
            while (!msg_queue_.empty())
            {
                send_atom msg = std::move(msg_queue_.front());

                if (remote_send_)
                {
                    // sending to session
                    remote_send_(binary::wrap(kernel_msg(msg.src_kind, msg.data)), msg.sure);
                }
                else 
                {
                    clients_t clients = clients_;
                    for (auto it = clients.begin(); it != clients.end(); ++it)
                        if ((*it)->kind() != msg.src_kind) // prevent loopback
                            (*it)->on_msg(msg.data);
                }
            
                msg_queue_.pop();
            }
    }

// loopback means the message was sent from current process
    void on_remote_recv(binary::bytes_cref bytes, bool loopback) 
    {       
        kernel_msg msg;
        binary::unwrap(bytes, msg);

        clients_t clients = clients_;
        for (auto it = clients.begin(); it != clients.end(); ++it)
            if (!loopback || (*it)->kind() != msg.sys_kind)
                (*it)->on_msg(msg.bytes);
    }

    net_layer::ses_srv* ses_srv() const 
    {
        return ses_srv_;
    }

private:
    struct send_atom
    {
        binary::bytes_t data;
        system_kind     src_kind;   
        bool            sure;


        send_atom(binary::bytes_cref data, system_kind kind, bool sure)
            : data    (data)
            , src_kind(kind)
            , sure    (sure)
        {
        }

        send_atom(send_atom&& other)
            : data(move(other.data))
            , src_kind(other.src_kind)
            , sure(other.sure)
        {
        }
    };

    typedef 
        std::queue<send_atom> 
        msg_queue_t;

private:
    msg_queue_t msg_queue_;

private:
    net_layer::ses_srv* ses_srv_;
    remote_send_f       remote_send_;
    
private:
    typedef set<system*> clients_t;
    clients_t            clients_;
};


struct msg_service_reg
{
    msg_service_reg(msg_service& srv, system* sys)
        : service_(srv)
        , sys_    (sys)
    {
        service_.register_sys(sys_);
    }

    ~msg_service_reg()
    {
        service_.unregister_sys(sys_);
    }

    void operator()(system_kind kind, binary::bytes_cref bytes, bool sure)
    {
        service_.send_message(kind, bytes, sure);
    }

    net_layer::ses_srv* ses_srv() const 
    {
        return service_.ses_srv();
    }

private:
    msg_service&    service_;
    system*         sys_;
};

} // kernel

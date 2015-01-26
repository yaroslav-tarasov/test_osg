#pragma once

#include <boost/signals2/signal.hpp>

//! @@@ важно
//! держатель соединения, связанный с системой сигналов и слотов, на которой строится система сообщений
//! вероятно, используется для автоматического разрыва соединений сигналов и слотов при уничтожении объекта,
//! т.к. сами объекты соединений не нужны как поля классов; а так они просто хранятся в едином объекте-массиве
struct connection_holder 
{
    typedef boost::signals2::connection         conn_t;
    typedef boost::signals2::scoped_connection  scoped_conn_t;

    typedef std::unique_ptr<scoped_conn_t> scoped_conn_ptr;
    typedef std::vector<scoped_conn_ptr>   scoped_conn_ptrs;

    //! оператор добавления соединения во внутренний массив
    connection_holder& operator<<(conn_t const& conn)
    {
        ptrs_.emplace_back(scoped_conn_ptr(new scoped_conn_t(conn)));
        return *this;
    }

    void release()
    {
        ptrs_ = scoped_conn_ptrs();
    }

private:
    //! внутренний динамический массив соединений
    scoped_conn_ptrs ptrs_;
};

//! @@@ важно
//! объявление события (СИГНАЛА, т.е. ИСХОДЯЩЕЕ); 
//! внутри: объявляем 
//!  ТИП СЛОТА      'имя_slot_type', 
//!  ОБЪЕКТ СИГНАЛА 'имя_signal_', и
//!  ФУНКИЦЮ        'subscribe_имя(slot)' 
//! которая соединяет передаваемый ей ОБЪЕКТ СЛОТА с нашим ОБЪЕКТОМ СИГНАЛА
//! мы объявляем ИСХОДЯЩЕЕ событие, то есть это то что можем сгенерировать отсюда, вызвав вручную name_signal_() с аргументами
//! нам все равно что там вызовется (может и ничего), это забота подписчиков на это событие
//! аналог в Qt - emit, по идее можно было бы сделать аналогичный макрос EMIT_EVENT(name)
//! тот, кто хочет подписаться на событие данного объекта, вручную вызывает subscribe_name, 
//! аналог в Qt - connect, аналогично можно было бы сделать макрос SUBSCRIBE_EVENT(name, slot)
//! ОТПИСКА ОТ СОБЫТИЯ реализуется другим способом - через метод .release() объекта соединения, который возвращает функция subscribe_*

#define DECLARE_EVENT(name,arglist)                                                     \
    typedef boost::signals2::signal<void arglist>::slot_type name##_slot_type;          \
    virtual boost::signals2::connection subscribe_##name( name##_slot_type const& slot )\
    {                                                                                   \
        return name##_signal_.connect(slot);                                            \
    }                                                                                   \
                                                                                        \
    boost::signals2::signal<void arglist> name##_signal_

//! обертка для подписывания на события
#define SUBSCRIBE_EVENT(obj,name,handler) \
    obj->subscribe_##name(handler)

//! обертка для генерации событий
#define EMIT_EVENT_ARGS(name,arglist) \
    name_##signal_(arglist)

#define EMIT_EVENT(name) \
    name_##signal_()

/*  DEPRECATED

    / * be careful, verify the module of to_track allocation still alive while signal is called. * /\
    / * Even if you unsubscribe!! (because of lazy unsubscription in boost::signals2) * / \
                                                                                        \
    template<class T>                                                                   \
    void subscribe_##name( name##_slot_type slot, boost::shared_ptr<T> to_track)        \
    {                                                                                   \
        name##_signal_.connect(slot.track(to_track));                                   \
    }                                                                                   \
*/
    





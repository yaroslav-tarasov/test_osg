#pragma once
#include "network/msg_base.h"


namespace airport
{
#if 0
    namespace lighting
    {

        DEFINE_WRAPED_ENUM( light_type_t,
            ((lt_approach , "Approach lights"))
            ((lt_apsr     , "Approach sider"))
            ((lt_flash    , "Flash"))
            ((lt_threshold, "Threshold"))
            ((lt_papi     , "Papi"))
            ((lt_tdz      , "Touch down zone"))
            ((lt_rwye     , "Runway edge"))
            ((lt_rcl      , "Runway center-line"))
            ((lt_stop     , "Taxiway stop"))
            ((lt_txcl     , "Taxiway centerline"))
            ((lt_txe      , "Taxiway edge")) 
            ((lt_last     , "Last value not for using"))
            )
         

        struct rw_bulb_t
        {
            cg::point_3f p_;
            cg::colorab    color;
        };


        typedef clight_type_t type_t;

        struct state_t
        {
            state_t() : value(0) {}
            explicit state_t(uint32_t value) : value(value) {}
            operator uint32_t() const { return value; }
            
            uint32_t value;

            REFL_INNER(state_t)
                REFL_NUM(value , 0, 100, 1)
            REFL_END()
        };  

        struct tx_t
        {
            uint16_t edge;
            uint16_t center;
            explicit tx_t(uint16_t edge=0,uint16_t center=0):edge(edge),center(center){};
        };
        
        REFL_STRUCT(tx_t)
            REFL_NUM(edge , 0, 100, 1)
            REFL_NUM(center , 0, 100, 1)
        REFL_END()

        struct rwy_t
        {
            typedef map<type_t,state_t> lights_t;

            std::string suffix;  // or pseudo name  something like (L,R,C)
            uint16_t    num;
            int         course;  // second course = norm(first +/- 180)
            lights_t    approach_1;
            lights_t    approach_2;
            lights_t    rw_l;
            rwy_t(uint16_t num=0,int course=0,std::string suffix=""):num(num),course(course),suffix(suffix){};
        };
                       
        REFL_STRUCT(rwy_t)
            REFL_SER(suffix)
            REFL_ENTRY_NAMED_RO(num,"Runway number")
            REFL_SER(course)
            REFL_ENTRY_NAMED(approach_1,(boost::str(boost::format("%02d") % (cg::round(float(obj.course / 10)))) + obj.suffix).c_str())
            REFL_ENTRY_NAMED(approach_2,(boost::str(boost::format("%02d") % (cg::round(float(cg::norm360(obj.course + 180) / 10)))) + obj.suffix).c_str())
            REFL_ENTRY(rw_l)
        REFL_END()



        struct navi_lights_control_t
        {
            typedef map<type_t,state_t>             control_t;
            typedef vector<rwy_t>                   rwys_t;
            typedef std::map<std::string,tx_t>      tx_lights_t;
            typedef std::map<std::string,state_t>   stop_lights_t;
            typedef airport::lighting::type_t       type_t;
            typedef airport::lighting::state_t      state_t;

            static inline void sochi( rwys_t&            rwys,
                                      tx_lights_t&       tx_lights,
                                      stop_lights_t&     stop_lights)
            {
                rwys =rwys_t(); tx_lights = tx_lights_t(); stop_lights = stop_lights_t();
                
                for (short rwi=0;rwi<2;rwi++)
                {
                    rwy_t rwyx(rwi+1,rwi==0?59:22,"");

                    rwyx.approach_1.insert(std::make_pair(type_t(lt_approach),state_t(100)));         // Зависит от курса
                    rwyx.approach_1.insert(std::make_pair(type_t(lt_apsr    ),state_t(100)));         // Зависит от курса
                    rwyx.approach_1.insert(std::make_pair(type_t(lt_flash   ),state_t(100)));         // Зависит от курса
                    rwyx.approach_1.insert(std::make_pair(type_t(lt_papi    ),state_t(100)));         // Зависит от курса
                    rwyx.approach_1.insert(std::make_pair(type_t(lt_tdz     ),state_t(100)));         // Зависит от курса
                    rwyx.approach_1.insert(std::make_pair(type_t(lt_threshold ),state_t(100)));       // Зависит от курса

                    rwyx.approach_2.insert(std::make_pair(type_t(lt_approach),state_t(100)));         // Зависит от курса
                    rwyx.approach_2.insert(std::make_pair(type_t(lt_apsr    ),state_t(100)));         // Зависит от курса
                    rwyx.approach_2.insert(std::make_pair(type_t(lt_flash   ),state_t(100)));         // Зависит от курса
                    rwyx.approach_2.insert(std::make_pair(type_t(lt_papi    ),state_t(100)));         // Зависит от курса
                    rwyx.approach_2.insert(std::make_pair(type_t(lt_tdz     ),state_t(100)));         // Зависит от курса
                    rwyx.approach_2.insert(std::make_pair(type_t(lt_threshold     ),state_t(100)));   // Зависит от курса
                    
                    rwyx.rw_l.insert(std::make_pair(type_t(lt_rwye    ),state_t(100)));
                    rwyx.rw_l.insert(std::make_pair(type_t(lt_rcl     ),state_t(100)));

                    rwys.push_back(rwyx);
                }


                tx_lights.insert(std::make_pair("A",tx_t(100)));
                tx_lights.insert(std::make_pair("B",tx_t(100)));
                tx_lights.insert(std::make_pair("C",tx_t(100)));
                tx_lights.insert(std::make_pair("D",tx_t(100)));
                tx_lights.insert(std::make_pair("E",tx_t(100)));
                tx_lights.insert(std::make_pair("F",tx_t(100)));
                tx_lights.insert(std::make_pair("G",tx_t(100)));
                tx_lights.insert(std::make_pair("H",tx_t(100)));
                tx_lights.insert(std::make_pair("K",tx_t(100)));
                tx_lights.insert(std::make_pair("L",tx_t(100)));
                tx_lights.insert(std::make_pair("M",tx_t(100)));
                tx_lights.insert(std::make_pair("N",tx_t(100)));
                tx_lights.insert(std::make_pair("P",tx_t(100)));
                tx_lights.insert(std::make_pair("R",tx_t(100)));
                tx_lights.insert(std::make_pair("S",tx_t(100)));


/*                stop_lights.insert(std::pair<std::string,state_t>("G",state_t(100)));
                stop_lights.insert(std::pair<std::string,state_t>("A",state_t(100)));
                stop_lights.insert(std::pair<std::string,state_t>("N",state_t(100)));  */ 

            }
            
            navi_lights_control_t()
            {  
                // FIXME  Dummy staff
                control.clear();
                control.insert(std::make_pair(type_t(lt_approach),state_t(100)));   // Зависит от курса
                control.insert(std::make_pair(type_t(lt_apsr    ),state_t(100)));   // Зависит от курса
                control.insert(std::make_pair(type_t(lt_flash   ),state_t(100)));   // Зависит от курса
                control.insert(std::make_pair(type_t(lt_papi    ),state_t(100)));   // Зависит от курса
                control.insert(std::make_pair(type_t(lt_tdz     ),state_t(100)));   // Зависит от курса
                control.insert(std::make_pair(type_t(lt_threshold),state_t(100)));
                control.insert(std::make_pair(type_t(lt_rwye    ),state_t(100)));
                control.insert(std::make_pair(type_t(lt_rcl     ),state_t(100)));
                control.insert(std::make_pair(type_t(lt_stop    ),state_t(100)));   // Интенсивность всех
                control.insert(std::make_pair(type_t(lt_txcl    ),state_t(100)));   // Интенсивность всех
                control.insert(std::make_pair(type_t(lt_txe     ),state_t(100)));   // Интенсивность всех
                
                sochi(rwys,tx_lights,stop_lights);
            }

            rwys_t            rwys;
            tx_t              tx_power;
            tx_lights_t       tx_lights;
            stop_lights_t     stop_lights;
            
            control_t control;


            //template<typename T> 
                //navi_lights_control_t(const T& nlc)
            
            template<typename T> 
                navi_lights_control_t& operator=(const T& nlc)
            {
                navi_lights_control_t::navi_lights_control(nlc);
                return *this;
            }            
            
            navi_lights_control_t& operator=(const navi_lights_control_t& nlc)
            {
                navi_lights_control_t::navi_lights_control(nlc);
                return *this;
            }

        private:
            template <typename T>
            void navi_lights_control(const T& nlc)
            {

                auto it = nlc.control.begin();
                for(;it!= nlc.control.end(); ++it)
                {
                    this->control.find(it->first)->second=control_t::mapped_type(it->second);
                }

// Для одного типа
                // this->tx_power =  nlc.tx_power;
// Обобщая 
                this->tx_power.center =  nlc.tx_power.center;
                this->tx_power.edge   =  nlc.tx_power.edge;

                auto it_new = nlc.rwys.begin();
                for(;it_new!= nlc.rwys.end(); ++it_new)
                {
                    auto rwy_it = std::find_if(this->rwys.begin(),this->rwys.end(),
                        [&it_new](const rwy_t& r)->bool{return r.suffix== it_new->suffix && r.course == it_new->course;});

                    if(rwy_it!=this->rwys.end())
                    {
                        auto it_apr1 = it_new->approach_1.begin() ;
                        for(;it_apr1!= it_new->approach_1.end(); ++it_apr1)
                        {
                            rwy_it->approach_1.find(it_apr1->first)->second=rwy_t::lights_t::mapped_type(it_apr1->second);
                        }

                        auto it_apr2 = it_new->approach_2.begin() ;
                        for(;it_apr2!= it_new->approach_2.end(); ++it_apr2)
                        {
                            rwy_it->approach_2.find(it_apr2->first)->second=rwy_t::lights_t::mapped_type(it_apr2->second);
                        }

                        auto it_rw = it_new->rw_l.begin() ;
                        for(;it_rw!= it_new->rw_l.end(); ++it_rw)
                        {
                            rwy_it->rw_l.find(it_rw->first)->second=rwy_t::lights_t::mapped_type(it_rw->second);
                        }

                    }

                }

                auto it_tx = nlc.tx_lights.begin();
                for(;it_tx!= nlc.tx_lights.end(); ++it_tx)
                {
                    // Для одного типа
                    // this->tx_lights.find(it_tx->first)->second=it_tx->second;
                    // Обобщая
                    auto it = this->tx_lights.find(it_tx->first);
                    if(it!=tx_lights.end())
                    {
                        it->second.center =  it_tx->second.center;
                        it->second.edge   =  it_tx->second.edge;
                    }
                }

                auto it_stop = nlc.stop_lights.begin();
                for(;it_stop!= nlc.stop_lights.end(); ++it_stop)
                {
                     this->stop_lights.find(it_stop->first)->second=stop_lights_t::mapped_type(it_stop->second);
                }

            
            }
 
            private:
            template<typename T> 
                operator T () const;


        };


        REFL_STRUCT(navi_lights_control_t)
            REFL_SER(control)
            REFL_ENTRY(rwys)
            REFL_ENTRY_NAMED(tx_power,"Power of taxi lights")
            REFL_ENTRY(tx_lights)
            REFL_ENTRY(stop_lights)
        REFL_END()



    } // namespace lighting
#endif
struct settings_t
{
    settings_t()
        : icao_code("UUEE")
    {}

    string   icao_code;
    point_3  lights_offset;
#if 0
    lighting::navi_lights_control_t  navi_lights_control ;
    //lighting::navi_lights_t          navi_lights ;
#endif

};



namespace msg
{

//! сообщение аэропорта
enum msg_type
{
    mt_settings
};

//! тело сообщения аэропорта
struct settings_msg
    : network::msg_id<mt_settings>
{
    settings_t settings;

    settings_msg(settings_t const& settings)
        : settings(settings)
    {
    }
    
    settings_msg()
    {
    }
};

REFL_STRUCT(settings_msg)
    REFL_ENTRY(settings)
REFL_END()
} // messages 


//! а это вот такой способ определения модели по коду (очевидно заглушка)
inline string get_model(string icao_code)
{
    if (icao_code == "UUEE")
        return "sheremetyevo";
    else if (icao_code == "URSS")
        return "adler";
    else if (icao_code == "UMMS" || icao_code == "UMMM")
        return "minsk";
    else if (icao_code == "UUOL"  )
        return "lipetsk";
    else if (icao_code == "URKE"  )
        return "eisk";
    else if (icao_code == "UUWW"  )
        return "vnukovo";

    return "";
}

// FIXME Здесь хардкод и там хардкод
inline bool valid_icao(string icao_code)
{
    return icao_code == "UUEE" || 
           icao_code == "URSS" ||
           icao_code == "UHWW" ||
           icao_code == "UMMS" ||
           icao_code == "UMMM" ||
           icao_code == "UUOL" ||
           icao_code == "URKE" 
           ;     
}

inline string get_icao_code(string name)
{
    if (name == "Шереметьево")
        return "UUEE";
    else if (name == "Адлер")
        return "URSS";
    else if (name == "Минск")
        return "UMMS";
    else if (name == "Липецк"  )
        return "UUOL";
    else if (name == "Ейск"  )
        return "URKE";
    else if (name == "Внуково"  )
        return "UUWW";

    return "";
}


//! смещение огней по коду аэропорта (нет нормальной базы данных, объединяющей всю информацию об объектах и средства ее редактирования)
inline point_3f lights_offset(string icao_code)
{
    if (icao_code == "UUEE")
        return point_3f(18, 17, .2f);
    else if (icao_code == "URSS")
        return point_3f(0 , 0 , .2f);

    return point_3f();
}

REFL_STRUCT(settings_t)
    REFL_ENTRY(icao_code)
    REFL_SER(lights_offset)
#if 0
    REFL_ENTRY(navi_lights_control)
#endif
REFL_END()

} // namespace airport




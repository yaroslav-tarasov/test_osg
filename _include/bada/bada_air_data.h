#pragma once

#include "bada_enums.h"

namespace bada
{

//! интерфейс получения параметров ВС для набора высоты
struct climb_data
{
    virtual ~climb_data(){}

    virtual std::pair<int, int> getStandardCAS() const = 0 ;
    virtual double              getMach()        const = 0 ;

    virtual double                     getVstallTO()                    const = 0 ;
    virtual double                     getMaxThrustCoeffs(unsigned idx) const = 0 ;  // [Ctc1 Ctc2 Ctc3 Ctc4 Ctc5]
    virtual std::pair<double, double>  getCf()                          const = 0 ;  // [Cf1 Cf2]
    virtual double                     getFuelConsumAtHeight(int)       const = 0 ;
    virtual int                        getTASatHeight(int)              const = 0 ;
    // virtual int                        getROCDatHeight(int)             const = 0 ;
} ;

//! интерфейс получения параметров ВС для крейсерского полета
struct cruise_data
{
    virtual ~cruise_data(){}

    virtual std::pair<int, int> getStandardCAS() const = 0 ;
    virtual double              getMach()        const = 0 ;

    virtual double getCf()                    const = 0 ;  // Cfcr
    //virtual double getFuelConsumAtHeight(int) const = 0 ;  //
    virtual int    getTASatHeight(int)        const = 0 ;
} ;

//! интерфейс получения параметров ВС для снижения
struct descent_data
{
    virtual ~descent_data(){}

    virtual std::pair<int, int> getStandardCAS() const = 0 ;
    virtual double              getMach()        const = 0 ;

    virtual double                    getApproachThrustCoeff()    const = 0 ; // Ct_des_app
    virtual double                    getLandingThrustCoeff()     const = 0 ; // Ct_des_ld
    virtual double                    getHdes()                   const = 0 ; // Hp_des
    virtual double                    getVstallLD()               const = 0 ;
    virtual std::pair<double, double> getThrustCoeffs()           const = 0 ; // [Ct_des_low Ct_des_high]
    virtual std::pair<double, double> getVelRef()                 const = 0 ; // [Vdes_ref Mdes_ref]
    virtual std::pair<double, double> getCf()                     const = 0 ; // [Cf3 Cf4]
    virtual double                    getFuelConsumAtHeight(int)  const = 0 ; 
    virtual int                       getTASatHeight(int)         const = 0 ;
    virtual int                       getROCDatHeight(int)        const = 0 ;
} ;

//! интерфейс получения информации о синонимах
struct synonim_data
{
    virtual ~synonim_data(){}

    virtual std::string          getSynonim()   const = 0 ;
    virtual std::string          getManufacturer()   const = 0 ;
    virtual std::string          getModelName()   const = 0 ;
};

//! интерфейс получения параметров ВС из БАДА
struct air_data
{
    virtual ~air_data(){}

    virtual climb_data   const & getDataCLS()      const = 0 ;
    virtual cruise_data  const & getDataCRS()      const = 0 ;
    virtual descent_data const & getDataDS()       const = 0 ;
    virtual int                  get_n_eng()       const = 0 ;
    virtual engine_kind          getEngineType()   const = 0 ;
    virtual turbulence_kind      getWakeCategory() const = 0 ;
    virtual double               get_m_ref()       const = 0 ;
    virtual double               get_m_min()       const = 0 ;
    virtual double               get_m_max()       const = 0 ;
    virtual double               get_m_pyld()      const = 0 ;
    virtual double               get_Gw()          const = 0 ;
    virtual double               get_Gt()          const = 0 ;
    virtual double               get_Vmo()         const = 0 ;
    virtual double               get_Mmo()         const = 0 ;
    virtual double               get_hmo()         const = 0 ;
    virtual double               get_hmax()        const = 0 ;
    virtual double               get_Vstall_cr()   const = 0 ;
    virtual double               get_Vstall_ic()   const = 0 ;
    virtual double               get_Vstall_to()   const = 0 ;
    virtual double               get_Vstall_ap()   const = 0 ;
    virtual double               get_Vstall_ld()   const = 0 ;
    virtual double               get_Ctc_1()       const = 0 ;
    virtual double               get_Ctc_2()       const = 0 ;
    virtual double               get_Ctc_3()       const = 0 ;
    virtual double               get_Ctc_4()       const = 0 ;
    virtual double               get_Ctc_5()       const = 0 ;
    virtual double               get_Ct_des_low()  const = 0 ;
    virtual double               get_Ct_des_high() const = 0 ;
    virtual double               get_Hp_des()      const = 0 ;
    virtual double               get_Ct_des_app()  const = 0 ;
    virtual double               get_Ct_des_ld()   const = 0 ;
    virtual double               get_Vdes_ref()    const = 0 ;
    virtual double               get_Mdes_ref()    const = 0 ;
    virtual double               get_Cf_1()        const = 0 ;
    virtual double               get_Cf_2()        const = 0 ;
    virtual double               get_Cf_3()        const = 0 ;
    virtual double               get_Cf_4()        const = 0 ;
    virtual double               get_Cfr()         const = 0 ;
    virtual double               get_S()           const = 0 ;
    virtual double               get_Clbo()        const = 0 ;
    virtual double               get_K()           const = 0 ;
    virtual double               get_Cm16()        const = 0 ;
    virtual double               get_Cd0_cr()      const = 0 ;
    virtual double               get_Cd2_cr()      const = 0 ;
    virtual double               get_Cd0_ap()      const = 0 ;
    virtual double               get_Cd2_ap()      const = 0 ;
    virtual double               get_Cd0_ld()      const = 0 ;
    virtual double               get_Cd2_ld()      const = 0 ;
    virtual double               get_Cd0_ldg()     const = 0 ;
    virtual double               get_TOL()         const = 0 ;
    virtual double               get_LDL()         const = 0 ;
    virtual double               get_span()        const = 0 ;
    virtual double               get_length()      const = 0 ;
};

} // end bada
#pragma once

namespace bada
{

struct glb_data
{
    virtual ~glb_data(){}

    virtual int      getAccLong()                 const = 0 ;
    virtual int      getAccNorm()                 const = 0 ;
    virtual double   getCdesExp()                 const = 0 ;
    virtual double   getCThTo()                   const = 0 ;
    virtual double   getCThCr()                   const = 0 ;
    virtual int      getBackTrack()               const = 0 ;
    virtual int      getVtaxi()                   const = 0 ;
    virtual int      getVapron()                  const = 0 ;
    virtual int      getVgate()                   const = 0 ;
    virtual int      getAngBankNom(unsigned idx)  const = 0 ;
    virtual int      getAngBankMax(unsigned idx)  const = 0 ;
    virtual int      getHmax(std::string const &) const = 0 ;

    virtual std::pair<std::string, double> getCVmin(unsigned idx) const = 0 ;
//     virtual std::vector<int>                            const & getV(std::pair<std::string, std::string> const &) const = 0 ;
// 
//     virtual double getVhold(unsigned idx)       const = 0 ;
//     virtual double getCred(std::string const &) const = 0 ;
} ;

} // end bada
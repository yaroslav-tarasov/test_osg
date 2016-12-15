#pragma once

#if 0
struct large_int
{
    large_int(long val=0)
    {
        _value.HighPart = 0;
        _value.LowPart  = val;
    }

    operator LARGE_INTEGER () const {return _value;}
    LARGE_INTEGER* operator&()      {return &_value;}
    LARGE_INTEGER _value;
};

class  high_res_timer
{
public:
     high_res_timer()
         : _last_point(0)
     {
        ::QueryPerformanceFrequency(&_frequency);
        set_point();
     }
    
     double set_point() 
     {   
         large_int point;
         ::QueryPerformanceCounter(&point);
         double interval = static_cast<double>(point._value.QuadPart - _last_point._value.QuadPart) / static_cast<double>(_frequency._value.QuadPart);
         _last_point = point;
         return interval;
     }

     double get_delta() 
     {   
         large_int point;
         ::QueryPerformanceCounter(&point);
         return static_cast<double>(point._value.QuadPart - _last_point._value.QuadPart) / static_cast<double>(_frequency._value.QuadPart);
     }

private:
     large_int     _frequency;
     large_int     _last_point;
};
#else

class  high_res_timer
{
public:
    
    inline double set_point() 
    {   
        auto interval = pc_.time_s();
        pc_.reset();
        return interval;
    }

    inline double get_delta() 
    {   
        return pc_.time_s();
    }

private:
    PerformanceCounter  pc_;
};

#endif


struct time_measure_helper_t
{
    typedef std::function<bool(double)>  output_condition_t;

    time_measure_helper_t(const std::string& header, const output_condition_t& cond )
        : header (header)
        , cond   (cond)
    {}

    ~time_measure_helper_t()
    {
        force_log fl;
        
        double delta = hr_timer.get_delta();
        //if(cond(delta))
        LOG_ODS_MSG( header << "  time: "<< delta << "\n");
    }

    high_res_timer hr_timer;
    std::string    header;
    output_condition_t cond;
};
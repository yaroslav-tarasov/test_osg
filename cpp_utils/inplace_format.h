#pragma once 

namespace cpp_utils
{

template<size_t buf_size, size_t format_size>
struct inplace_format_t
    : noncopyable
{   
    inplace_format_t(const char* format)
        : format_sign_ ('%')
        , write_pos_   (buf_)
        , format_len_  (strlen(format))
        , format_end_  (format_ + format_len_)
    {
        assert(format_len_ < format_size);
        memcpy(format_, format, format_len_ + 1);

        format_start_ = format_;
    }

    template<class T>
    inplace_format_t& operator%(T const& arg)
    {
        // too many arguments provided
        assert(format_start_ != end());

        fmt_ptr format_end = next_format_sign(format_start_);

        // first argument 
        if (*format_start_ != format_sign_)
            format_end = next_format_sign(format_end);

        if (format_end != end())
            *format_end = 0;

        write_pos_ += sprintf(write_pos_, format_start_, arg);

        // too small buf_size for your format
        assert(write_pos_ < buf_ + buf_size);

        if (format_end !=end())
            *format_end = format_sign_;

        format_start_ = format_end;
        return *this;
    }

    inplace_format_t& operator%(std::string const& arg)
    {
        return *this % arg.c_str();
    }

    const char* c_str() const
    {
        // inplace_format needs more arguments then provided
        assert(format_start_ == end()); 
        return buf_;
    }

    std::string str() const 
    {
        return std::string(c_str());
    }

private:
    typedef char        fmt_sym;
    typedef fmt_sym*    fmt_ptr;
    typedef char        str_sym;
    typedef str_sym*    str_ptr;

private:
    fmt_ptr end() const 
    { 
        return format_end_;
    }

    fmt_ptr next_format_sign(fmt_ptr it) const 
    {
        it = (it == end()) ? end() : it + 1;

        while (it != end() && *it != format_sign_ && *it) 
            ++it;

        return it;
    }

private:
    fmt_sym         format_[format_size]; 
    size_t          format_len_;
    fmt_ptr         format_start_;
    fmt_ptr         format_end_;
    const fmt_sym   format_sign_;

private:
    mutable str_sym buf_[buf_size];
    str_ptr         write_pos_;
};

typedef 
    inplace_format_t<0x400, 0x400> 
    inplace_format;

inline std::string str(inplace_format const& fmt)
{
    return fmt.str();
}

} // cpp_utils


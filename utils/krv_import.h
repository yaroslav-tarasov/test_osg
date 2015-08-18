#pragma once

namespace krv
{
        struct data
        {
            float x;
            float y; 
            float h;
            float fi;
            float fiw;
            float kr;
            float v;
            float w; 
            float vb;
            float tg;
            float time;
        };

        __forceinline std::ostream &operator <<(std::ostream &os, const data &kp) {
            using namespace std;

            for(size_t i = 0 ; i < sizeof(kp)/sizeof(float); ++i)
                os << *((float*)(&kp) + i*sizeof(float))  << "  ";
            return os;
        }

        struct value_getter
        {
            value_getter(std::string const& line)
            {
                boost::split(values_, line, boost::is_any_of(" \t="), boost::token_compress_on);
            }

            template <class T>
            T get(size_t index)
            {
                return boost::lexical_cast<T>(values_[index]);
            }

            bool valid()
            {
                return values_.size()>0;
            }

        private:
            std::vector<std::string> values_;
        };



        struct  data_getter
        {
            std::vector<data>    kd_;
            std::vector<cg::point_3> kp_;

            data_getter(const std::string& file_name = std::string("log_AFL319.txt") )
            {
                std::ifstream ifs(file_name);

                int num =0;
                while (ifs.good())
                {
                    char buf[0x400] = {};
                    ifs.getline(buf, 0x400);

                    std::string line = buf;
                    value_getter items(line);
                    data kd;

                    if(items.valid())
                    {
                        kd.x = items.get<float>(1);
                        kd.y = items.get<float>(3); 
                        kd.h = items.get<float>(5);
                        //kd.fi = items.get<float>(7);
                        kd.fiw = items.get<float>(7);
                        kd.kr  = items.get<float>(9);
                        //kd.v  = items.get<float>(13);
                        //kd.w  = items.get<float>(15); 
                        //kd.vb = items.get<float>(17);
                        kd.tg   = items.get<float>(15);
                        kd.time = items.get<float>(17);

                        kd_.push_back(kd);
                        kp_.push_back( cg::point_3(kd.x,kd.y,kd.h));
                    }


                    // std::cout << line;
                } 


            }
        };
}
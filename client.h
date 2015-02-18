#pragma once

struct client_interface
{

    virtual ~client_interface() {}
    //virtual atc::airport::data_t get_airport_data(const string &name) = 0;
    virtual bool is_ready() const = 0;

    enum place_t
    {
        CITY = 0,
        VILLAGE
    };

    virtual std::vector<cg::geo_point_2> get_places(const std::string &name, place_t type) = 0;
};

typedef polymorph_ptr<client_interface> client_interface_ptr;

class client_impl;

using boost::property_tree::ptree;

class client
    : public client_interface
{

public:
    client(const ptree &config);
    ~client();

//    atc::airport::data_t get_airport_data(const string &name)
//    {
//        return impl_->get_airport_data(name);
//    }

    bool is_ready() const;
    std::vector<cg::geo_point_2> get_places(const std::string &name, place_t type);

private:
    boost::shared_ptr<client_impl> impl_;
};

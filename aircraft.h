#pragma once


namespace phys
{
    class cpr : public osg::Vec3
    {
    public:
        inline cpr():osg::Vec3() {}
        inline cpr(osg::Vec3::value_type x,osg::Vec3::value_type y,osg::Vec3::value_type z): osg::Vec3(x,y,z) {}
        inline cpr(const osg::Vec2f& v2,osg::Vec3::value_type zz): osg::Vec3(v2,zz) {}
        inline cpr(osg::Vec3 const& v3 ):osg::Vec3(v3) {}
        inline cpr(osg::Vec3 & v3):osg::Vec3(v3) {}
        inline cpr(cg::cpr const& c3 ) { _v[0] = c3.course;_v[1] = c3.pitch;_v[3] = c3.roll;  }

    };

    class quaternion : public osg::Quat
    {
    public:
        inline quaternion(): osg::Quat() {}
        inline quaternion( osg::Quat::value_type x, osg::Quat::value_type y, osg::Quat::value_type z, osg::Quat::value_type w): osg::Quat( x, y, z, w ) {}
        inline quaternion( const osg::Vec4f& v ): osg::Quat( v ) {}
        inline quaternion( const osg::Vec4d& v ): osg::Quat( v ) {}
        inline quaternion( const cg::quaternion q) { _v[0] = q.get_w();_v[1] = q.get_v().x;_v[3] = q.get_v().y;_v[3] = q.get_v().z;  }
    };

    class point_3 : public osg::Vec3
    {
    public:
        inline point_3():osg::Vec3() {}
        inline point_3(osg::Vec3::value_type x,osg::Vec3::value_type y,osg::Vec3::value_type z): osg::Vec3(x,y,z) {}
        inline point_3(const osg::Vec2f& v2,osg::Vec3::value_type zz): osg::Vec3(v2,zz) {}
        inline point_3(osg::Vec3 const& v3 ):osg::Vec3(v3) {}
        inline point_3(osg::Vec3 & v3):osg::Vec3(v3) {}
        inline point_3(cg::point_3 const& v3 ) { _v[0] = v3.x;_v[1] = v3.y;_v[3] = v3.z;  }
//      point_3(osg::Vec3 & v3):osg::Vec3(v3) {}
        
    };


	namespace aircraft
	{

        struct params_t
        {
            params_t()
                : min_aerodynamic_speed(50)
                , roll_sliding(0.1)
                , roll_omega_y(1.)
                , roll_omega_z(1.)

                , course_sliding(0.1)
                , course_omega_z(1.)
                , course_omega_y(1.)

                , pitch_drag(1.)
                , pitch_attack(0.1)
                , pitch_omega_x(1.)
                , pitch_attack_derivative(1.)
                , elevator(200)
                , rudder(200)
                , ailerons(200)
                , aa0(-1.)
                , S(0.0),chord(0.0),length(0.0),mass(0.0),wingspan(0.0)
                , Cl(0.0)
                , Cd0(0.0)
                , Cd2(0.0)
                , ClAOA(0.0)
                , Cs(0.0)
                , thrust(0.0)

            {}

            double S;
            double chord;
            double length;
            double mass;
            double wingspan;

            double min_aerodynamic_speed;

            double Cl;
            double Cd0;
            double Cd2;
            double ClAOA;
            double Cs;
            double aa0;

            double thrust;
            double elevator;
            double rudder;
            double ailerons;

            double roll_sliding;
            double roll_omega_y;
            double roll_omega_z;

            double course_sliding;
            double course_omega_z;
            double course_omega_y;

            double pitch_drag;
            double pitch_attack;
            double pitch_omega_x;
            double pitch_attack_derivative;
        };  

	struct contact_info_t
	{
		contact_info_t() {}
		contact_info_t( point_3 const& offset, point_3 const& vel )
			: vel(vel), offset(offset)
		{}

		point_3 vel;
		point_3 offset;
	};

	struct info
	{
		virtual ~info() {}

		virtual decart_position get_position() const = 0;
		virtual decart_position get_wheel_position( size_t i ) const = 0;
        virtual double Ixx() const = 0;
        virtual double Iyy() const = 0;
        virtual double Izz() const = 0;
		virtual params_t const& params() const = 0;
		virtual double drag() const = 0;
		virtual double lift() const = 0;
		virtual double thrust() const = 0;
		//virtual bool has_contact() const = 0;
		//virtual std::vector<contact_info_t> get_body_contacts() const = 0;
		//virtual bool has_wheel_contact(size_t id) const = 0;
		//virtual double wheel_skid_info(size_t id) const = 0;
	};  

	struct control : info
	{
		virtual size_t add_wheel( double mass, double width, double radius, point_3 const& offset, cpr const & orien, bool has_damper, bool is_front ) = 0;
		virtual void   remove_wheel(size_t id) = 0;

		//virtual void set_control_manager(std::function<void(double)> f) = 0;

		virtual void   set_steer   (double steer) = 0;
        virtual double get_steer () = 0;
		virtual void   set_brake   (double brake) = 0;
		virtual void   set_thrust  (double thrust) = 0;
		virtual void   set_elevator(double elevator) = 0;
		virtual void   set_ailerons(double ailerons) = 0;
		virtual void   set_rudder  (double rudder) = 0;
		virtual void   set_wind    (cg::point_3 const& wind) = 0;
		virtual void   apply_force (point_3 const& f) = 0;
		virtual void   update_aerodynamics(double dt) = 0;
		virtual void   reset_suspension() = 0;
	};


	}

};
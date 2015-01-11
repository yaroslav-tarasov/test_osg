#pragma once


namespace phys
{
	typedef osg::Vec3d decart_position;
	typedef osg::Vec3d point_3;
	typedef osg::Quat  cpr;
	namespace aircraft
	{

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

		//virtual decart_position get_position() const = 0;
		//virtual decart_position get_wheel_position( size_t i ) const = 0;
		//virtual double Ixx() const = 0;
		//virtual double Iyy() const = 0;
		//virtual double Izz() const = 0;
		//virtual params_t const& params() const = 0;
		//virtual double drag() const = 0;
		//virtual double lift() const = 0;
		//virtual double thrust() const = 0;
		//virtual bool has_contact() const = 0;
		//virtual std::vector<contact_info_t> get_body_contacts() const = 0;
		//virtual bool has_wheel_contact(size_t id) const = 0;
		//virtual double wheel_skid_info(size_t id) const = 0;
	};  

	struct control : info
	{
		virtual size_t add_wheel( double mass, double width, double radius, point_3 const& offset, cpr const & orien, bool has_damper, bool is_front ) = 0;
		//virtual void remove_wheel(size_t id) = 0;

		//virtual void set_control_manager(std::function<void(double)> f) = 0;

		//virtual void set_steer   (double steer) = 0;
		//virtual void set_brake   (double brake) = 0;
		//virtual void set_thrust  (double thrust) = 0;
		//virtual void set_elevator(double elevator) = 0;
		//virtual void set_ailerons(double ailerons) = 0;
		//virtual void set_rudder  (double rudder) = 0;
		//virtual void set_wind    (point_3 const& wind) = 0;
		//virtual void apply_force (point_3 const& f) = 0;
		//virtual void update_aerodynamics(double dt) = 0;
		//virtual void reset_suspension() = 0;
	};


	}

};
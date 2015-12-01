#pragma once

#ifndef Q_MOC_RUN
#define  BOOST_MOVE_USE_STANDARD_LIBRARY_MOVE
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp> 
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <boost/variant.hpp>
#include <boost/foreach.hpp>

#include <boost/circular_buffer.hpp>

#include <boost/graph/astar_search.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random.hpp>
#include <boost/random.hpp>
#include <boost/graph/graphviz.hpp>
                             
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <boost/signals2.hpp> 
////////////////////////////////////////////////

using boost::none;

using boost::scoped_ptr;
using boost::scoped_array;
using boost::intrusive_ptr;
using boost::shared_ptr;
using boost::shared_array;
using boost::weak_ptr;
using boost::make_shared;
using boost::enable_shared_from_this;

using boost::noncopyable;
using boost::optional;
using boost::in_place;

using boost::format;
using boost::wformat;

using boost::lexical_cast;

using boost::bind;
using boost::function;

using boost::any;
using boost::any_cast;
using boost::bad_any_cast;
using boost::static_pointer_cast;

using boost::circular_buffer;
using boost::signals2::scoped_connection;
using boost::signals2::connection;
#endif // Q_MOC_RUN

#ifdef _DEBUG
#pragma comment(lib, "boost_thread-vc100-mt-gd-1_50.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-gd-1_50.lib")
#pragma comment(lib, "boost_system-vc100-mt-gd-1_50.lib")
#else
#pragma comment(lib, "boost_thread-vc100-mt-1_50.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-1_50.lib")
#pragma comment(lib, "boost_system-vc100-mt-1_50.lib")
#endif
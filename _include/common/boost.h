#pragma once

#ifndef Q_MOC_RUN
// #define BOOST_MOVE_USE_STANDARD_LIBRARY_MOVE
#define BOOST_OPTIONAL_USE_OLD_DEFINITION_OF_NONE 
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

#include <boost/utility/in_place_factory.hpp>
#include <boost/none_t.hpp>


#include <boost/math/special_functions/cbrt.hpp>
#include <boost/math/special_functions/atanh.hpp>
////////////////////////////////////////////////

using boost::none;
using boost::none_t;

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

#if (BOOST_VERSION == 105000)

#ifdef _DEBUG
#pragma comment(lib, "boost_thread-vc100-mt-gd-1_50.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-gd-1_50.lib")
#pragma comment(lib, "boost_system-vc100-mt-gd-1_50.lib")
#else
#pragma comment(lib, "boost_thread-vc100-mt-1_50.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-1_50.lib")
#pragma comment(lib, "boost_system-vc100-mt-1_50.lib")
#endif

#else
#if (BOOST_VERSION == 105900)

#ifdef _DEBUG
#pragma comment(lib, "boost_thread-vc100-mt-gd-1_59.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-gd-1_59.lib")
#pragma comment(lib, "boost_system-vc100-mt-gd-1_59.lib")
#else
#pragma comment(lib, "boost_thread-vc100-mt-1_59.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-1_59.lib")
#pragma comment(lib, "boost_system-vc100-mt-1_59.lib")
#endif

#endif

#endif
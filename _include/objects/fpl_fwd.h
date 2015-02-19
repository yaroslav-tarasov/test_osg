#pragma once

namespace fpl
{
struct settings_t;
struct info ;

typedef polymorph_ptr<info>   info_ptr;
typedef boost::weak_ptr<info> info_wptr;
}
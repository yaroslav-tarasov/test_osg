#pragma once
#include "cpp_utils/polymorph_ptr.h"

namespace kernel
{

struct system;
struct system_session;

struct base_presentation;
struct tree_object;

struct chart_system;
struct chart_presentation;

struct history_prs;
struct history_sys;

struct visual_system;
struct visual_system_props;
struct visual_control;
struct visual_presentation;
struct visual_object;

struct model_system;
struct model_presentation;

struct objects_factory;
struct object_collection;

struct vcs_system;
struct ext_ctrl_sys;

typedef polymorph_ptr<system> system_ptr;
typedef polymorph_ptr<system_session> system_session_ptr;

typedef polymorph_ptr<base_presentation>    base_presentation_ptr;
typedef polymorph_ptr<tree_object>          tree_object_ptr;
typedef boost::weak_ptr<tree_object>        tree_object_wptr;

typedef polymorph_ptr<chart_system>         chart_system_ptr;
typedef polymorph_ptr<chart_presentation>   chart_presentation_ptr;

typedef polymorph_ptr<history_prs>          history_prs_ptr;
typedef polymorph_ptr<history_sys>          history_sys_ptr;

typedef polymorph_ptr<visual_system>        visual_system_ptr;
typedef polymorph_ptr<visual_system_props>  visual_system_props_ptr;
typedef polymorph_ptr<visual_control>       visual_control_ptr;
typedef polymorph_ptr<visual_presentation>  visual_presentation_ptr;
typedef polymorph_ptr<visual_object>        visual_object_ptr;

typedef polymorph_ptr<model_system>         model_system_ptr;
typedef polymorph_ptr<model_presentation>   model_presentation_ptr;

typedef polymorph_ptr<objects_factory>      objects_factory_ptr;
typedef polymorph_ptr<object_collection>    object_collection_ptr;

typedef polymorph_ptr<vcs_system>           vcs_system_ptr;
typedef polymorph_ptr<ext_ctrl_sys>         ext_ctrl_sys_ptr;

} // namespace kernel

#if !defined(STATIC_SYSTEM_API)
#ifdef SYSTEMS_LIB
#   define SYSTEMS_API __HELPER_DL_EXPORT
#else
#   define SYSTEMS_API __HELPER_DL_IMPORT
#endif
#else
#   define SYSTEMS_API
#endif

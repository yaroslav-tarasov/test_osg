#pragma once

enum RenderBinOrder
{
    // rendered first - sky-dome, stars, clouds, etc...
    RENDER_BIN_SKYDOME                  = -6, // global sky dome
    RENDER_BIN_STARS                    = -5, // stars
    RENDER_BIN_SUN_MOON                 = -4, // sun. moon and other planet
    RENDER_BIN_CLOUDS                   = -3, // sky clouds
	RENDER_BIN_LIGHTNING                = -2, // sky lightning
    RENDER_BIN_SKYFOG                   = -1, // global sky fog layer

    // rendered next - solid models 
    RENDER_BIN_SCENE                    = 0,  

    // rendered next - special bin for depth obtaining
    RENDER_BIN_AFTER_MODELS             = 6,  // special bin for depth texture to obtain actual depth
    
    // rendered next - local weather, particles effects, global weather
    RENDER_BIN_LOCAL_WEATHER            = 7,  // local fog and local bank cloud
	RENDER_BIN_LIGHTS                   = 8,
	RENDER_BIN_PARTICLE_EFFECTS         = 11,  // any particle effect (must take local fog into account)
    RENDER_BIN_GLOBAL_WEATHER           = 17,  // global rain/snow/hail around viewer

    // GUI at last
    RENDER_BIN_GUI                      = 21, // GUI

    // Image calibration
    RENDER_BIN_IMAGE_CALIBRATION        = 100 // Always on top
};


#pragma once

namespace rtr
{
namespace shaders
{
	// intersection shaders:
	bool Checkerboard(const rtr::payload& payload);

	// color shaders:

    rtr::material EarthTexture(const rtr::payload& payload, const rtr::material* mat);

}
}

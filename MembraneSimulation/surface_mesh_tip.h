/*
	This file contains a definition of a point, as would be simulating a filament tip.
	Currently the point has absolute position, and no counter-interaction is imposed on the point.
*/

#pragma once

#include"math_public.h"
#include"surface_mesh.h"

namespace MS {

	class tip_facet_interaction {
	public:
		double d;
		Vec3 d_d[3];
		Vec3 dp_d;
	};
	class filament_tip {
	public:
		math_public::Vec3 *point;
		std::vector<facet*> n_facets; // neighbor facet list

		filament_tip(math_public::Vec3 *np) :point(np) {}

		void get_neighbor_facets(const surface_mesh& sm);
		tip_facet_interaction get_facet_interaction(const facet& f);
		
	};
	extern math_public::Vec3 *po;
	double update_len(double param); // should be removed when considering real polymers

	extern std::vector<facet*> po_neighbor;
}

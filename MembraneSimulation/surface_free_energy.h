#pragma once

#include"surface_mesh.h"

namespace MS {
	double h_curv_h(vertex * v); // Free energy - Curvature - Mean
	double h_curv_g(vertex * v); // Free energy - Curvature - Gaussian
	double h_tension(vertex * v); // Free energy - Surface tension
	//double h_osmotic(vertex * v); // Free energy - Osmotic pressure
	//double h_potential(vertex * v); // Free energy - Potential energy

	double d_h_curv_h(vertex *v, int c_index); // Partial Derivative - ...
	double d_h_curv_g(vertex *v, int c_index); // Partial Derivative - ...
	double d_h_tension(vertex *v, int c_index); // Partial Derivative - ...
	//double d_h_

	double h_all(vertex * v);
	double d_h_all(vertex *v, int c_index);
}

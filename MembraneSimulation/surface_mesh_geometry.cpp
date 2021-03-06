#define _USE_MATH_DEFINES

#include"common.h"
#include"surface_mesh.h"

using namespace MS;
using namespace math_public;

#define USE_VONOROI_CELL true


vertex::vertex(Vec3 *npoint) {
	point = npoint;
	point_last = new Vec3(0,0,0);
}

vertex::~vertex() {
	// "point" would not be deleted, since the point might be passed to another vertex or shared by another stucture.
	// "point" has to be manually released before vertex destructs itself.
	delete point_last;
}

int vertex::count_neighbors() {
	neighbors = n.size();
	for (int i = 0; i < neighbors; i++) {
		neighbor_indices_map[n[i]] = i;
	}
	return neighbors;
}

int vertex::gen_next_prev_n() {
	count_neighbors();
	for (int i = 0; i < neighbors; i++) {
		nn.push_back(n[i < neighbors - 1 ? i + 1 : 0]);
		np.push_back(n[i > 0 ? i - 1 : neighbors - 1]);
	}
	return 0;
}

int vertex::dump_data_vectors(int size) {
	for (int i = 0; i < size; i++) {
		// theta
		theta.push_back(0), sin_theta.push_back(0);
		d_theta.push_back(Vec3()), dn_theta.push_back(Vec3()), dnn_theta.push_back(Vec3());
		d_sin_theta.push_back(Vec3()), dn_sin_theta.push_back(Vec3()), dnn_sin_theta.push_back(Vec3());
		// theta2
		theta2.push_back(0), cot_theta2.push_back(0);
		d_theta2.push_back(Vec3()), dn_theta2.push_back(Vec3()), dnp_theta2.push_back(Vec3());
		d_cot_theta2.push_back(Vec3()), dn_cot_theta2.push_back(Vec3()), dnp_cot_theta2.push_back(Vec3());
		// theta3
		theta3.push_back(0), cot_theta3.push_back(0);
		d_theta3.push_back(Vec3()), dn_theta3.push_back(Vec3()), dnn_theta3.push_back(Vec3());
		d_cot_theta3.push_back(Vec3()), dn_cot_theta3.push_back(Vec3()), dnn_cot_theta3.push_back(Vec3());
		// distances
		r_p_n.push_back(0), d_r_p_n.push_back(Vec3()), dn_r_p_n.push_back(Vec3());
		r_p_np.push_back(0), d_r_p_np.push_back(Vec3()), dnp_r_p_np.push_back(Vec3());
		r_p_nn.push_back(0), d_r_p_nn.push_back(Vec3()), dnn_r_p_nn.push_back(Vec3());

		// Derivatives around a vertex
		dn_area.push_back(Vec3());
		dn_curv_h.push_back(Vec3());
		dn_curv_g.push_back(Vec3());

		// normal
		dn_n_vec.push_back(Mat3());

		// volume integrand
		dn_volume_op.push_back(Vec3());
	}

	return 0;
}

/*
int MS::vertex::fill_vectors_with_zeroes() {
	for (int i = 0, len = n.size(); i < len; i++) {
		theta[i] = 0; dx_theta[i] = dy_theta[i] = dz_theta[i] = 0;
		sin_theta[i] = 0; dx_sin_theta[i] = dy_sin_theta[i] = dz_sin_theta[i] = 0;
		theta2[i] = 0; dx_theta2[i] = dy_theta2[i] = dz_theta2[i] = 0;
		cot_theta2[i] = 0; dx_cot_theta2[i] = dy_cot_theta2[i] = dz_cot_theta2[i] = 0;
		theta3[i] = 0; dx_theta3[i] = dy_theta3[i] = dz_theta3[i] = 0;
		cot_theta3[i] = 0; dx_cot_theta3[i] = dy_cot_theta3[i] = dz_cot_theta3[i] = 0;
		r_p_n[i] = 0; dx_r_p_n[i] = dy_r_p_n[i] = dz_r_p_n[i] = 0;
		r_p_n_prev[i] = 0; dx_r_p_n_prev[i] = dy_r_p_n_prev[i] = dz_r_p_n_prev[i] = 0;
		r_p_n_next[i] = 0; dx_r_p_n_next[i] = dy_r_p_n_next[i] = dz_r_p_n_next[i] = 0;
		return 0;
	}
}
*/


void vertex::calc_angle() {
	for (int i = 0; i < neighbors; i++) {
		// Distances
		r_p_n[i] = dist(*point, *(n[i]->point));
		d_r_p_n[i] = (*point - *(n[i]->point)) / r_p_n[i];
		dn_r_p_n[i] = (*(n[i]->point) - *point) / r_p_n[i];

		r_p_np[i] = dist(*point, *(np[i]->point));
		d_r_p_np[i] = (*point - *(np[i]->point)) / r_p_np[i];
		dnp_r_p_np[i] = (*(np[i]->point) - *point) / r_p_np[i];

		r_p_nn[i] = dist(*point, *(nn[i]->point));
		d_r_p_nn[i]= (*point - *(nn[i]->point)) / r_p_nn[i];
		dnn_r_p_nn[i] = (*(nn[i]->point) - *point) / r_p_nn[i];

		// Find theta
		double inner_product = dot(*(nn[i]->point) - *point, *(n[i]->point) - *point);
		Vec3 d_inner_product = 2 * (*point) - *(nn[i]->point) - *(n[i]->point);
		Vec3 dn_inner_product = *(nn[i]->point) - *point;
		Vec3 dnn_inner_product = *(n[i]->point) - *point;

		double cos_theta = inner_product / (r_p_n[i] * r_p_nn[i]);
		Vec3 d_cos_theta = (r_p_n[i] * r_p_nn[i] * d_inner_product - inner_product*(r_p_n[i] * d_r_p_nn[i] + d_r_p_n[i] * r_p_nn[i])) / (r_p_n[i] * r_p_n[i] * r_p_nn[i] * r_p_nn[i]);
		Vec3 dn_cos_theta = (r_p_n[i] * dn_inner_product - inner_product*dn_r_p_n[i]) / (r_p_n[i] * r_p_n[i] * r_p_nn[i]);
		Vec3 dnn_cos_theta = (r_p_nn[i] * dnn_inner_product - inner_product*dnn_r_p_nn[i]) / (r_p_nn[i] * r_p_nn[i] * r_p_n[i]);

		sin_theta[i] = sqrt(1 - cos_theta*cos_theta);
		theta[i] = acos(cos_theta);
		d_theta[i] = -d_cos_theta / sin_theta[i];
		dn_theta[i] = -dn_cos_theta / sin_theta[i];
		dnn_theta[i] = -dnn_cos_theta / sin_theta[i];
		d_sin_theta[i] = cos_theta*d_theta[i];
		dn_sin_theta[i] = cos_theta*dn_theta[i];
		dnn_sin_theta[i] = cos_theta*dnn_theta[i];

		// Find theta2
		double r_n_np = dist(*(n[i]->point), *(np[i]->point)); // derivative with regard to p is 0
		Vec3 dn_r_n_np = (*(n[i]->point) - *(np[i]->point)) / r_n_np;
		Vec3 dnp_r_n_np = (*(np[i]->point) - *(n[i]->point)) / r_n_np;
		double inner_product2 = dot(*(n[i]->point) - *(np[i]->point), *point - *(np[i]->point));
		Vec3 d_inner_product2 = *(n[i]->point) - *(np[i]->point);
		Vec3 dn_inner_product2 = *point - *(np[i]->point);
		Vec3 dnp_inner_product2 = 2 * *(np[i]->point) - *point - *(n[i]->point);

		double cos_theta2 = inner_product2 / (r_p_np[i] * r_n_np);
		Vec3 d_cos_theta2 = (r_p_np[i] * d_inner_product2 - inner_product2*d_r_p_np[i]) / (r_p_np[i] * r_p_np[i] * r_n_np);
		Vec3 dn_cos_theta2 = (r_n_np * dn_inner_product2 - inner_product2*dn_r_n_np) / (r_n_np * r_n_np * r_p_np[i]);
		Vec3 dnp_cos_theta2= (r_p_np[i] * r_n_np * dnp_inner_product2 - inner_product2*(dnp_r_p_np[i] * r_n_np + r_p_np[i] * dnp_r_n_np)) / (r_p_np[i] * r_p_np[i] * r_n_np*r_n_np);

		double sin_theta2 = sqrt(1 - cos_theta2*cos_theta2);
		theta2[i] = acos(cos_theta2);
		d_theta2[i] = -d_cos_theta2 / sin_theta2;
		dn_theta2[i] = -dn_cos_theta2 / sin_theta2;
		dnp_theta2[i] = -dnp_cos_theta2 / sin_theta2;

		cot_theta2[i] = cos_theta2 / sin_theta2;
		d_cot_theta2[i] = -d_theta2[i] / (sin_theta2*sin_theta2);
		dn_cot_theta2[i] = -dn_theta2[i] / (sin_theta2*sin_theta2);
		dnp_cot_theta2[i] = -dnp_theta2[i] / (sin_theta2*sin_theta2);

		// Find theta3
		double r_n_nn = dist(*(n[i]->point), *(nn[i]->point)); // derivative with regard to p is 0
		Vec3 dn_r_n_nn = (*(n[i]->point) - *(nn[i]->point)) / r_n_nn;
		Vec3 dnn_r_n_nn = (*(nn[i]->point) - *(n[i]->point)) / r_n_nn;
		double inner_product3 = dot(*(n[i]->point) - *(nn[i]->point), *point - *(nn[i]->point));
		Vec3 d_inner_product3 = *(n[i]->point) - *(nn[i]->point);
		Vec3 dn_inner_product3 = *point - *(nn[i]->point);
		Vec3 dnn_inner_product3 = 2 * *(nn[i]->point) - *point - *(n[i]->point);

		double cos_theta3 = inner_product3 / (r_p_nn[i] * r_n_nn);
		Vec3 d_cos_theta3 = (r_p_nn[i] * d_inner_product3 - inner_product3*d_r_p_nn[i]) / (r_p_nn[i] * r_p_nn[i] * r_n_nn);
		Vec3 dn_cos_theta3 = (r_n_nn * dn_inner_product3 - inner_product3*dn_r_n_nn) / (r_n_nn * r_n_nn * r_p_nn[i]);
		Vec3 dnn_cos_theta3 = (r_p_nn[i] * r_n_nn * dnn_inner_product3 - inner_product3*(dnn_r_p_nn[i] * r_n_nn + r_p_nn[i] * dnn_r_n_nn)) / (r_p_nn[i] * r_p_nn[i] * r_n_nn*r_n_nn);

		double sin_theta3 = sqrt(1 - cos_theta3*cos_theta3);
		theta3[i] = acos(cos_theta3);
		d_theta3[i] = -d_cos_theta3 / sin_theta3;
		dn_theta3[i] = -dn_cos_theta3 / sin_theta3;
		dnn_theta3[i] = -dnn_cos_theta3 / sin_theta3;

		cot_theta3[i] = cos_theta3 / sin_theta3;
		d_cot_theta3[i] = -d_theta3[i] / (sin_theta3*sin_theta3);
		dn_cot_theta3[i] = -dn_theta3[i] / (sin_theta3*sin_theta3);
		dnn_cot_theta3[i] = -dnn_theta3[i] / (sin_theta3*sin_theta3);

	}
}

double vertex::calc_area() {
	/*****************************************************************************
	Must be used after angles are calculated.

	Use only Vonoroi area even for obtuse triangles.

	The area around vertex i is:
	A = 1/8 * \sum_{v_j is neighbour of v_i} (cot \alpha_ij + cot \beta_ij ) * |r_i - r_j|^2
	*****************************************************************************/
	if (USE_VONOROI_CELL) {
		area = 0;
		d_area.set(0, 0, 0);
		for (int i = 0; i < neighbors; i++) {
			dn_area[i].set(0, 0, 0);
		}
		for (int i = 0; i < neighbors; i++) {
			double dis2 = r_p_n[i] * r_p_n[i];
			area += (cot_theta2[i] + cot_theta3[i])*dis2;
			d_area += (d_cot_theta2[i] + d_cot_theta3[i])*dis2 + (cot_theta2[i] + cot_theta3[i]) * 2 * r_p_n[i] * d_r_p_n[i];
			dn_area[i] += (dn_cot_theta2[i] + dn_cot_theta3[i])*dis2 + (cot_theta2[i] + cot_theta3[i]) * 2 * r_p_n[i] * dn_r_p_n[i];
			int i_n = neighbor_indices_map.at(nn[i]), i_p = neighbor_indices_map.at(np[i]);
			dn_area[i_n] += (dnn_cot_theta3[i])*dis2;
			dn_area[i_p] += (dnp_cot_theta2[i])*dis2;
		}
		area /= 8; d_area /= 8;
		for (int i = 0; i < neighbors; i++) {
			dn_area[i] /= 8;
		}
		return area;
	}
	else {
		return 0;
	}
}

double vertex::calc_curv_h() {
	/*****************************************************************************
	Must be used after the angles and the area is calculated.
	This function could also calculate the normalized normal vector.

	Use only Vonoroi area even for obtuse triangles.

	Locally, Laplace-Beltrami operator (vector) K = 2 * k_h * n, where n is the normalized normal vector.
	And K = 1/(2A) * \sum_{v_j is neighbour of v_i} (cot \alpha_ij + cot \beta_ij ) * (r_i - r_j)	
	*****************************************************************************/
	if (USE_VONOROI_CELL) {
		Vec3 K;
		Mat3 d_diff(Eye3), dn_diff(-Eye3);
		Mat3 d_K;
		std::vector<Mat3> dn_K(neighbors);
		for (int i = 0; i < neighbors; i++) {
			Vec3 diff = *point - *(n[i]->point);
			K += (cot_theta2[i] + cot_theta3[i])*diff;
			d_K += (d_cot_theta2[i] + d_cot_theta3[i]).tensor(diff) + (cot_theta2[i] + cot_theta3[i])*d_diff;
			dn_K[i] += (dn_cot_theta2[i] + dn_cot_theta3[i]).tensor(diff) + (cot_theta2[i] + cot_theta3[i])*dn_diff;

			int i_n = neighbor_indices_map[nn[i]], i_p = neighbor_indices_map[np[i]];
			dn_K[i_n] += dnn_cot_theta3[i].tensor(diff);
			dn_K[i_p] += dnp_cot_theta2[i].tensor(diff);
		}

		// Convert K to K/2A
		d_K = (area*d_K - d_area.tensor(K)) / (2 * area*area);
		for (int i = 0; i < neighbors; i++) {
			dn_K[i] = (area*dn_K[i] - dn_area[i].tensor(K)) / (2 * area*area);
		}
		K /= 2 * area;

		K.calc_norm(); // Must be used before using norm
		curv_h = K.norm / 2;
		d_curv_h = (d_K*K) / (2 * K.norm);
		for (int i = 0; i < neighbors; i++) {
			dn_curv_h[i] = (dn_K[i] * K) / (2 * K.norm);
		}
		//n_vec = K / K.norm; // We no longer calculate normal vector here because it would be very inaccurate when |K| is close to 0.

		return curv_h;
	}
	else {
		return 0;
	}
}

double vertex::calc_curv_g() {
	/*****************************************************************************
	Must be used after the angles and the area is calculated.

	Use only Vonoroi area even for obtuse triangles.

	Locally, k_g * A = 2\pi - \sum_{angle j around v_i} theta_j
	*****************************************************************************/
	if (USE_VONOROI_CELL) {
		double a = 2 * M_PI;
		Vec3 d_a;
		std::vector<Vec3> dn_a(neighbors);
		for (int i = 0; i < neighbors; i++) {
			a -= theta[i];
			d_a -= d_theta[i];
			dn_a[i] -= dn_theta[i];
			int i_n = neighbor_indices_map[nn[i]];
			dn_a[i_n] -= dnn_theta[i];
		}
		d_curv_g = (area*d_a - a*d_area) / (area*area);
		for (int i = 0; i < neighbors; i++) {
			dn_curv_g[i] = (area*dn_a[i] - a*dn_area[i]) / (area*area);
		}
		curv_g = a / area;
		return curv_g;
	}
	else {
		return 0;
	}
}
void vertex::calc_normal() {
	/*****************************************************************************
	This function calculates the normal vector around a vertex using angle weighted pseudo normal method.
	
	Must be used after
		- the angles surrounding the vertex have been calculated
		- the normal vectors of the surrounding facets have been calculated
	*****************************************************************************/
	Vec3 sum(0, 0, 0);
	for (int i = 0; i < neighbors; i++) {
		sum += f[i]->n_vec * theta[i];
	}
	n_vec = sum / sum.get_norm();

	// determine derivative of "sum"
	Mat3 d_sum;
	Mat3 *dn_sum = new Mat3[neighbors];
	for (int i = 0; i < neighbors; i++) {
		// Find the index j on the facet which points to the central vertex
		int j = 0;
		while (j < 3 && f[i]->v[j] != this) j++;
		// Give the derivative to the central vertex
		d_sum += f[i]->d_n_vec[j] * theta[i] + d_theta[i].tensor(f[i]->n_vec);
		// Give the derivative to the neighbor vertex
		dn_sum[i] += f[i]->d_n_vec[loop_add(j, 1, 3)] * theta[i] + dn_theta[i].tensor(f[i]->n_vec);
		// Give the derivative to the next neighbor vertex
		dn_sum[loop_add(i, 1, neighbors)] += f[i]->d_n_vec[loop_add(j, 2, 3)] * theta[i] + dnn_theta[i].tensor(f[i]->n_vec);
	}

	Mat3 param1 = (Eye3 - n_vec.tensor(n_vec)) / sum.norm;
	d_n_vec = d_sum * param1;
	for (int i = 0; i < neighbors; i++) {
		dn_n_vec[i] = dn_sum[i] * param1;
	}

	delete[] dn_sum;
}

double vertex::calc_volume_op() {
	/*****************************************************************************
	Must be used after
		- the normal vector at the vertex is calculated
		- the area is calculated
	*****************************************************************************/
	// Setting up the vector field with divergence 1.
	Div1VecField.set(point->x / 3, point->y / 3, point->z / 3);
	d_Div1VecField = Mat3(1.0/3, 0, 0, 0, 1.0/3, 0, 0, 0, 1.0/3);

	double field_in_normal_dir = dot(Div1VecField, n_vec);
	volume_op = field_in_normal_dir*area;
	d_volume_op = (d_Div1VecField * n_vec + d_n_vec * Div1VecField) * area + field_in_normal_dir*d_area;
	for (int i = 0; i < neighbors; i++) {
		dn_volume_op[i] = (dn_n_vec[i] * Div1VecField) * area + field_in_normal_dir * dn_area[i];
	}
	return volume_op;
}

void vertex::update_geo() {
	calc_angle();
	calc_area();
	calc_curv_h();
	//calc_curv_g();
	calc_normal();
	calc_volume_op();
}

void vertex::make_initial() {
	area0 = area;
}

void vertex::make_last() {
	point_last->x = point->x;
	point_last->y = point->y;
	point_last->z = point->z;
}

void facet::calc_vec() {
	v1 = *(v[1]->point) - *(v[0]->point);
	v2 = *(v[2]->point) - *(v[0]->point);
	r12 = v2 - v1;
	v1.calc_norm();
	v2.calc_norm();
	r12.calc_norm();
}
void facet::calc_normal() {
	Vec3 res = cross(v1, v2);
	n_vec = res / res.get_norm();
	Mat3 d0_res = -r12.to_skew_cross(),
		d1_res = v2.to_skew_cross(),
		d2_res = -v1.to_skew_cross(); // Notebook page 68
	Mat3 temp = (Eye3 - n_vec.tensor(n_vec)) / res.norm;
	d_n_vec[0] = d0_res*temp;
	d_n_vec[1] = d1_res*temp;
	d_n_vec[2] = d2_res*temp;
}
void facet::calc_area_and_projmat() {

	// Calculate the area of the triangle.
	S = cross(v1, v2).get_norm() / 2;
	if (S <= 0) {
		LOG(WARNING) << "Facet area is not positive. S = " << S;
	}

	// alpha and beta need to satisfy the perpendicular condition
	// A * (alpha, beta)' = B
	// So (alpha, beta)' = A^(-1) * B
	double dot12 = dot(v1, v2);
	Vec3 d0_dot12 = -v2 - v1, d1_dot12 = v2, d2_dot12 = v1; // Already taken into account those "Eye"-derivatives.
	Vec3 d0_norm2_v1 = -v1 * 2, d1_norm2_v1 = v1 * 2, d0_norm_v1 = -v1 / v1.norm, d1_norm_v1 = v1 / v1.norm;
	Vec3 d0_norm2_v2 = -v2 * 2, d2_norm2_v2 = v2 * 2, d0_norm_v2 = -v2 / v2.norm, d2_norm_v2 = v2 / v2.norm;
	Vec3 d1_norm2_r12 = -r12 * 2, d2_norm2_r12 = r12 * 2, d1_norm_r12 = -r12 / r12.norm, d2_norm_r12 = r12 / r12.norm;

	d_S[0] = (-v1.norm2*v2 - v2.norm2*v1 + dot12*(v1 + v2)) / S / 4;
	d_S[1] = (v2.norm2*v1 - dot12*v2) / S / 4;
	d_S[2] = (v1.norm2*v2 - dot12*v1) / S / 4;

	// det(A) = |v1|^2 |v2|^2 - (v1 * v2)^2, but theoretically this is essentially S2^2
	double det_A = S * S * 4;
	double det_A2 = det_A*det_A;
	Vec3 d0_det_A = 8 * S * d_S[0],
		d1_det_A = 8 * S * d_S[1],
		d2_det_A = 8 * S * d_S[2];
	AR11 = v2.norm2 / det_A;
	AR12 = -dot12 / det_A; // AR21 = AR12
	AR22 = v1.norm2 / det_A;
	d_AR11[0] = (det_A*d0_norm2_v2 - v2.norm2*d0_det_A) / det_A2;
	d_AR11[1] = -v2.norm2*d1_det_A / det_A2;
	d_AR11[2] = (det_A*d2_norm2_v2 - v2.norm2*d2_det_A) / det_A2;
	d_AR12[0] = -(det_A*d0_dot12 - dot12*d0_det_A) / det_A2;
	d_AR12[1] = -(det_A*d1_dot12 - dot12*d1_det_A) / det_A2;
	d_AR12[2] = -(det_A*d2_dot12 - dot12*d2_det_A) / det_A2;
	d_AR22[0] = (det_A*d0_norm2_v1 - v1.norm2*d0_det_A) / det_A2;
	d_AR22[1] = (det_A*d1_norm2_v1 - v1.norm2*d1_det_A) / det_A2;
	d_AR22[2] = -v1.norm2*d2_det_A / det_A2;

}
void facet::update_geo() {
	calc_vec();
	calc_normal();
	calc_area_and_projmat();
}
bool facet::operator==(const facet& operand) {
	int first_index = 0;
	while (first_index < 3) {
		if (v[0] == operand.v[first_index]) { // found a common vertex
			for (int i = 1; i < 3; i++) {
				if (v[i] != operand.v[math_public::loop_add(first_index, i, 3)])
					return false;
			}
			return true;
		}
		++first_index;
	}

	return false;
}


bool edge::operator==(const edge& operand) {
	int first_index = 0;
	while (first_index < 2) {
		if (v[0] == operand.v[first_index]) { // found a common vertex
			for (int i = 1; i < 2; i++) {
				if (v[i] != operand.v[math_public::loop_add(first_index, i, 2)])
					return false;
			}
			return true;
		}
		++first_index;
	}

	return false;
}

void MS::edge::calc_normal() {
	/*****************************************************************************
	This function calculates the normal vector at an edge using angle weighted pseudo normal method.

	Must be used after the normal vectors of the sharing facets have been calculated
	*****************************************************************************/

	Vec3 sum = f[0]->n_vec + f[1]->n_vec;
	n_vec = sum / sum.get_norm();
}
void MS::edge::update_geo() {
	calc_normal();
}

void MS::surface_mesh::update_geo() {
	int N;
	N = facets.size();
	for (int i = 0; i < N; i++) {
		facets[i]->update_geo();
	}
	N = vertices.size();
	for (int i = 0; i < N; i++) {
		vertices[i]->update_geo();
	}
	N = edges.size();
	for (int i = 0; i < N; i++) {
		edges[i]->update_geo();
	}
}

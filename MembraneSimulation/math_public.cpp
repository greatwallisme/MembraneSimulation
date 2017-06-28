#include"common.h"
#include"math_public.h"

using namespace math_public;

test::TestCase math_public::Vec3::test_case("Vec3", []() {
	{
		test_case.new_step("Check norm");
		Vec3 op(3, 4, 12);
		double ex = 13;
		test_case.assert_bool(equal(op.norm, ex));
	}
	{
		test_case.new_step("Check plus");
		Vec3 op1(1, 2, 3), op2(-3, -2, -1);
		Vec3 ex(-2, 0, 2);
		test_case.assert_bool((op1 + op2).equal_to(ex));
	}
	{
		test_case.new_step("Check minus");
		Vec3 op1(1, 2, 3), op2(-3, -2, -1);
		Vec3 ex(4, 4, 4);
		test_case.assert_bool((op1 - op2).equal_to(ex));
	}
	{
		test_case.new_step("Check multiplication");
		Vec3 op1(1, 2, 3); double op2 = -0.5;
		Vec3 ex1(-0.5, -1, -1.5), ex2(-2, -4, -6);
		test_case.assert_bool((op1 * op2).equal_to(ex1));
		test_case.assert_bool((op2 * op1).equal_to(ex1));
		test_case.assert_bool((op1 / op2).equal_to(ex2));
	}
	{
		test_case.new_step("Check compound operators");
		Vec3 op1(1, 2, 3), op2(-3, -2, -1); double op3 = -2;
		Vec3 ex1(-2, 0, 2); double ex2 = sqrt(8);
		Vec3 ex3(1, 2, 3); double ex4 = sqrt(14);
		Vec3 ex5(-2, -4, -6); double ex6 = sqrt(56);
		Vec3 ex7(1, 2, 3); double ex8 = sqrt(14);
		op1 += op2;
		test_case.assert_bool(op1.equal_to(ex1));
		test_case.assert_bool(equal(op1.norm, ex2));
		op1 -= op2;
		test_case.assert_bool(op1.equal_to(ex3));
		test_case.assert_bool(equal(op1.norm, ex4));
		op1 *= op3;
		test_case.assert_bool(op1.equal_to(ex5));
		test_case.assert_bool(equal(op1.norm, ex6));
		op1 /= op3;
		test_case.assert_bool(op1.equal_to(ex7));
		test_case.assert_bool(equal(op1.norm, ex8));
	}
	{
		test_case.new_step("Check dot product");
		Vec3 op1(1, 2, 3), op2(-3, -2, -1);
		double ex = -10;
		test_case.assert_bool(equal(op1*op2, ex));
	}
	{
		test_case.new_step("Check cross product");
		Vec3 op1(1, 2, 3), op2(-3, -2, -1);
		Vec3 ex(4, -8, 4);
		test_case.assert_bool(op1.cross(op2).equal_to(ex));
		test_case.assert_bool(equal(op1*ex, 0));
		test_case.assert_bool(equal(op2*ex, 0));
	}
});

test::TestCase math_public::Mat3::test_case("Mat3", []() {
	{
		test_case.new_step("Check linear transformation");
		Mat3 op1(1, 2, 3, 4, 5, 6, 7, 8, 9); Vec3 op2(10, 11, 12);
		Vec3 ex(68, 167, 266);
		test_case.assert_bool((op1*op2).equal_to(ex));
	}
	{
		test_case.new_step("Check matrix multiplication");
		Mat3 op1(1, 2, 3, 4, 5, 6, 7, 8, 9), op2(9, 8, 7, 6, 5, 4, 3, 2, 1);
		Mat3 ex(30, 24, 18, 84, 69, 54, 138, 114, 90);
		test_case.assert_bool((op1*op2).equal_to(ex));
	}
})
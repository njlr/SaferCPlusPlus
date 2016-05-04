
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/*
This example file has become quite large and holds examples for many data types. Your best bet is probably to use a find/search to
get to the data type your interested in.
*/


//define MSE_SAFER_SUBSTITUTES_DISABLED /* This will replace all the classes with their native/standard counterparts. */

/* Each of the following will replace a subset of the classes with their native/standard counterparts. */
//define MSE_MSTDVECTOR_DISABLED
//define MSE_REGISTEREDPOINTER_DISABLED
//define MSE_SAFERPTR_DISABLED /* MSE_SAFERPTR_DISABLED implies MSE_REGISTEREDPOINTER_DISABLED too. */
//define MSE_PRIMITIVES_DISABLED
//define MSE_REFCOUNTINGPOINTER_DISABLED
//define MSE_SCOPEPOINTER_DISABLED
//define MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

//include "msetl.h"
#include "msemsevector.h"
#include "msemstdvector.h"
#include "mseivector.h"
#include "msevector_test.h"
#include "mseregistered.h"
#include "mserelaxedregistered.h"
#include "mserefcounting.h"
#include "mserefcountingofregistered.h"
#include "mserefcountingofrelaxedregistered.h"
#include "msescope.h"
#include <algorithm>
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>

/* This block of includes is required for the mse::TRegisteredRefWrapper example */
#include <algorithm>
#include <list>
#include <vector>
#include <iostream>
#include <numeric>
#include <random>
#include <functional>

class H {
public:
	/* Just an example of a templated member function. In this case it's a static one, but it doesn't have to be.
	You might consider templating pointer parameter types to give the caller some flexibility as to which kind of
	(smart/safe) pointer they want to use. */
	template<typename _Tpointer>
	static int foo4(_Tpointer A_ptr) { return A_ptr->b; }
	template<typename _Tpointer, typename _Tvector>
	static int foo5(_Tpointer A_ptr, _Tvector& vector_ref) {
		int tmp = A_ptr->b;
		int retval = 0;
		vector_ref.clear();
		if (A_ptr) {
			retval = A_ptr->b;
		}
		else {
			retval = -1;
		}
		return retval;
	}
	template<class _TString1Pointer, class _TString2Pointer>
	static std::string foo6(_TString1Pointer i1ptr, _TString2Pointer i2ptr) {
		return (*i1ptr) + (*i2ptr);
	}
protected:
	~H() {}
};

int main(int argc, char* argv[])
{
	mse::msevector_test msevector_test;
	msevector_test.run_all();

	{
		/**********************/
		/*   mstd::vector<>   */
		/**********************/

		/* mse::mstd::vector<> is an almost "completely safe" (bounds checked, iterator checked and memory managed)
		implementation of std::vector. Here we'll demonstate the safety of the insert() member function. */

		double a1[3] = { 1.0, 2.0, 3.0 };
		double *d_pointer1 = &(a1[0]);
		double a2[3] = { 4.0, 5.0, 360 };
		double *d_pointer2 = &(a2[0]);
		mse::mstd::vector<double> v1;
		//v1.insert(v1.begin(), d_pointer1, d_pointer2); /* not good */
		/* std::vector supports "naked" pointers as parameters to the insert() member
		function so mse::mstd::vector does also. Unfortunately there is no way to ensure
		that the naked pointer parameters have valid values. */

#ifdef MSVC2010_COMPATIBILE
		mse::mstd::vector<double> v2(a1, a1+3);
		mse::mstd::vector<double> v3(a2, a2+3);
		mse::mstd::vector<double> v4;
#else /*MSVC2010_COMPATIBILE*/
		mse::mstd::vector<double> v2 = { 1.0, 2.0, 3.0 };
		mse::mstd::vector<double> v3 = { 4.0, 5.0, 360 };
		mse::mstd::vector<double> v4;
#endif /*MSVC2010_COMPATIBILE*/
		try {
			v4.insert(v4.begin(), v2.begin(), v3.begin());
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by a comparision of incompatible "safe" iterators. */
		}
	}

	{
		/* Here we're demonstrating mse::mstd::vector<>'s "java-like" safety due to it's "managed" lifespan.  */

		typedef mse::mstd::vector<int> vint_type;
		mse::mstd::vector<vint_type> vvi;
		{
			vint_type vi;
			vi.push_back(5);
			vvi.push_back(vi);
		}
		auto vi_it = vvi[0].begin();
		vvi.clear();
		/* At this point, the vint_type object is cleared from vvi, but it has not been deallocated/destructed yet because it
		"knows" that there is an iterator, namely vi_it, that is still referencing it. At the moment, std::shared_ptrs are being
		used to achieve this. */
		auto value = (*vi_it); /* So this is actually ok. vi_it still points to a valid item. */
		assert(5 == value);
		vint_type vi2;
		vi_it = vi2.begin();
		/* The vint_type object that vi_it was originally pointing to is now deallocated/destructed, because vi_it no longer
		references it. */
	}

	{
		/*****************************/
		/*   msevector<>::ipointer   */
		/*****************************/

		/* mse::msevector<> is another vector that is highly compatible with std::vector<>. But mse::msevector<> also
		supports a new type of iterator called "ipointer". ipointers make more (intuitive) sense than standard vector
		iterators. Upon insert or delete, ipointers continue to point to the same item, not (necessarily) the same
		position. And they don't become "invalid" upon insert or delete, unless the item they point to is deleted. They
		support all the standard iterator operators, but also have member functions with "friendlier" names. */

#ifdef MSVC2010_COMPATIBILE
		int a1[4] = { 1, 2, 3, 4 };
		mse::msevector<int> v1(a1, a1+4);
#else /*MSVC2010_COMPATIBILE*/
		mse::msevector<int> v1 = { 1, 2, 3, 4 };
#endif /*MSVC2010_COMPATIBILE*/

		mse::msevector<int> v = v1;

		{
			mse::msevector<int>::ipointer ip1 = v.ibegin();
			ip1 += 2;
			assert(3 == (*ip1));
			auto ip2 = v.ibegin(); /* ibegin() returns an ipointer */
			v.erase(ip2); /* remove the first item */
			assert(3 == (*ip1)); /* ip1 continues to point to the same item, not the same position */
			ip1--;
			assert(2 == (*ip1));
			for (mse::msevector<int>::cipointer cip = v.cibegin(); v.ciend() != cip; cip++) {
				/* You might imagine what would happen if cip were a regular vector iterator. */
				v.insert(v.ibegin(), (*cip));
			}
		}
		v = v1;
		{
			/* This code block is equivalent to the previous code block, but uses ipointer's more "readable" interface
			that might make the code a little more clear to those less familiar with C++ syntax. */
			mse::msevector<int>::ipointer ip_vit1 = v.ibegin();
			ip_vit1.advance(2);
			assert(3 == ip_vit1.item());
			auto ip_vit2 = v.ibegin();
			v.erase(ip_vit2);
			assert(3 == ip_vit1.item());
			ip_vit1.set_to_previous();
			assert(2 == ip_vit1.item());
			mse::msevector<int>::cipointer cip(v);
			for (cip.set_to_beginning(); cip.points_to_an_item(); cip.set_to_next()) {
				v.insert_before(v.ibegin(), (*cip));
			}
		}

		/* Btw, ipointers are compatible with stl algorithms, like any other stl iterators. */
		std::sort(v.ibegin(), v.iend());

		/* And just to be clear, mse::msevector<> retains it's original (high performance) stl::vector iterators. */
		std::sort(v.begin(), v.end());

		/* mse::msevector<> also provides "safe" (bounds checked) versions of the original stl::vector iterators. */
		std::sort(v.ss_begin(), v.ss_end());

		/* mse::ivector<> is another vector for when safety and "correctness" are more of a priority than performance
		or compatibility. It is completely safe like mse::mstd::vector<> but only supports the "ipointer" iterators.
		It does not support the (problematic) standard vector iterator behavior. */
		mse::ivector<int> iv = { 1, 2, 3, 4 };
		std::sort(iv.begin(), iv.end());
		mse::ivector<int>::ipointer ivip = iv.begin();
	}

	{
		/*******************************/
		/*   CInt, CSize_t and CBool   */
		/*******************************/

		/* The unsigned types like size_t can cause insidious bugs due to the fact that they can cause signed integers to be
		implicitly converted to unsigned. msetl provides substitutes for size_t and int that change the implicit conversion to
		instead be from unsigned to signed. */

		mse::s_type_test1();

		{
			size_t number_of_security_credits = 0;
			number_of_security_credits += 5;
			int minimum_number_of_security_credits_required_for_access = 7;
			bool access_granted = false;
			if (number_of_security_credits - minimum_number_of_security_credits_required_for_access >= 0) {
				/* You may not even get a compiler warning about the implicit conversion from (signed) int to (unsigned) size_t. */
				access_granted = true; /*oops*/
			}
			else {
				access_granted = false;
				assert(false);
			}
		}

		{
			mse::CSize_t number_of_security_credits = 0;
			number_of_security_credits += 5;
			int minimum_number_of_security_credits_required_for_access = 7;
			bool access_granted = false;
			if (number_of_security_credits - minimum_number_of_security_credits_required_for_access >= 0) {
				access_granted = true;
				assert(false);
			}
			else {
				access_granted = false; /* that's better */
			}
		}

		{
			size_t number_of_security_credits = 0;
			number_of_security_credits += 5;
			mse::CInt minimum_number_of_security_credits_required_for_access = 7;
			mse::CBool access_granted = false;
			if (number_of_security_credits - minimum_number_of_security_credits_required_for_access >= 0) {
				access_granted = true;
				assert(false);
			}
			else {
				access_granted = false; /* this works too */
			}
		}

		mse::CSize_t mse_szt1 = 0;
		/* size_t szt2 = mse_szt1; */ /* This wouldn't compile. */
#ifdef MSVC2010_COMPATIBILE
		size_t szt1 = mse::as_a_size_t(mse_szt1); /* We exclude automatic conversion from mse::CSize_t to size_t because we
													 consider size_t an intrinsically error prone type. */
#else /*MSVC2010_COMPATIBILE*/
		size_t szt1 = static_cast<size_t>(mse_szt1); /* We exclude automatic conversion from mse::CSize_t to size_t because we
													 consider size_t an intrinsically error prone type. */
#endif /*MSVC2010_COMPATIBILE*/

		try {
			mse::CSize_t mse_szt2 = 0;
			mse_szt2 = -3;
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by an "out of range" assignment to an mse::CSize_t. */
		}

		try {
			mse::CSize_t mse_szt3 = 3;
			mse_szt3 -= 1; /* this is fine */
			mse_szt3 -= 4; /* this is gonna throw an exception */
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by an attempt to set an mse::CSize_t to an "out of range" value. */
		}
	}

	{
		/**************************/
		/*   TRegisteredPointer   */
		/**************************/

		/* For safety reasons we want to avoid using native pointers. "Managed" pointers like std:shared_ptr are an alternative, but
		sometimes you don't want a pointer that takes ownership (of the object's lifespan). So we provide mse::TRegisteredPointer.
		Because it doesn't take ownership, it can be used with objects allocated on the stack and is compatible with raii
		techniques. Also, in most cases, it can be used as a compatible, direct substitute for native pointers, making it
		straightforward to update legacy code. Proper "const", "not null" and "fixed" (non-retargetable) versions are provided as
		well.*/

		class A {
		public:
			virtual ~A() {}
			int b = 3;
		};
		class B {
		public:
			static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
			static int foo2(mse::TRegisteredPointer<A> A_registered_ptr) { return A_registered_ptr->b; }
			/* mse::TRegisteredFixedConstPointer<A> is recommended where you might have used "const A&".*/
			static int foo3(mse::TRegisteredFixedConstPointer<A> A_registered_fc_ptr) { return A_registered_fc_ptr->b; }
		protected:
			~B() {}
		};

		A* A_native_ptr = nullptr;
		/* mse::TRegisteredPointer<> is basically a "safe" version of the native pointer. */
		mse::TRegisteredPointer<A> A_registered_ptr1;

		{
			A a;
			mse::TRegisteredObj<A> registered_a;
			/* mse::TRegisteredObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
			in almost all cases. */

			assert(a.b == registered_a.b);
			A_native_ptr = &a;
			A_registered_ptr1 = &registered_a;
			assert(A_native_ptr->b == A_registered_ptr1->b);

			mse::TRegisteredPointer<A> A_registered_ptr2 = &registered_a;
			A_registered_ptr2 = nullptr;
			try {
				int i = A_registered_ptr2->b; /* this is gonna throw an exception */
			}
			catch (...) {
				std::cerr << "expected exception" << std::endl;
				/* The exception is triggered by an attempt to dereference a null "registered pointer". */
			}

			/* mse::TRegisteredPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
			B::foo1((A*)A_registered_ptr1);

			B::foo3(&registered_a);
			/* mse::TRegisteredPointers don't convert directly into mse::TRegisteredFixedConstPointers because
			mse::TRegisteredFixedConstPointers are never supposed to be null, where mse::TRegisteredPointers can be. But you
			can easily obtain an mse::TRegisteredFixedPointer which does convert to an mse::TRegisteredFixedConstPointer. */
			B::foo3(&*A_registered_ptr1);

			/* Functions can be templated to allow the caller to use the (smart/safe) pointer of their choice. */
			H::foo4<mse::TRegisteredFixedConstPointer<A>>(&*A_registered_ptr1);
			/* You don't actually need to explicitly specify the template type. */
			H::foo4(&*A_registered_ptr1);

			if (A_registered_ptr2) {
				assert(false);
			}
			else if (A_registered_ptr2 != A_registered_ptr1) {
				A_registered_ptr2 = A_registered_ptr1;
				assert(A_registered_ptr2 == A_registered_ptr1);
			}
		}

		try {
			/* A_registered_ptr1 "knows" that the (registered) object it was pointing to has now been deallocated. */
			int i = A_registered_ptr1->b; /* So this is gonna throw an exception */
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
		}

		{
			/* For heap allocations mse::registered_new is kind of analagous to std::make_shared, but again,
			mse::TRegisteredPointers don't take ownership so you are responsible for deallocation. */
			auto A_registered_ptr3 = mse::registered_new<A>();
			assert(3 == A_registered_ptr3->b);
			mse::registered_delete<A>(A_registered_ptr3);
			try {
				/* A_registered_ptr3 "knows" that the (registered) object it was pointing to has now been deallocated. */
				int i = A_registered_ptr3->b; /* So this is gonna throw an exception */
			}
			catch (...) {
				std::cerr << "expected exception" << std::endl;
			}
		}

		{
			/* Remember that registered pointers can only point to registered objects. So, for example, if you want
			a registered pointer to an object's base class object, that base class object has to be a registered
			object. */
			class DA : public mse::TRegisteredObj<A> {};
			mse::TRegisteredObj<DA> registered_da;
			mse::TRegisteredPointer<DA> DA_registered_ptr1 = &registered_da;
			mse::TRegisteredPointer<A> A_registered_ptr4 = DA_registered_ptr1;
			A_registered_ptr4 = &registered_da;

			class D : public A {};
			mse::TRegisteredObj<D> registered_d;
			mse::TRegisteredPointer<D> D_registered_ptr1 = &registered_d;
			/* The next commented out line of code is not going to work because D's base class object is not a
			registered object. */
			//mse::TRegisteredPointer<A> A_registered_ptr5 = D_registered_ptr1;

			/* Note that unlike registered pointers, relaxed registered pointers can point to base class objects
			that are not relaxed registered objects. */
		}

		{
			/***********************************/
			/*   TRelaxedRegisteredPointer   */
			/***********************************/

			/* mse::TRelaxedRegisteredPointer<> behaves very similar to mse::TRegisteredPointer<> but is even more "compatible"
			with native pointers (i.e. less explicit casting is required when interacting with native pointers and native pointer
			interfaces). So if you're updating existing or legacy code to be safer, replacing native pointers with
			mse::TRelaxedRegisteredPointer<> may be more convenient than mse::TRegisteredPointer<>.
			One case where you may need to use mse::TRelaxedRegisteredPointer<> even when not dealing with legacy code is when
			you need a reference to a class before it is fully defined. For example, when you have two classes that mutually
			reference each other. mse::TRegisteredPointer<> does not support this.
			*/

			class C;

			class D {
			public:
				virtual ~D() {}
				mse::TRelaxedRegisteredPointer<C> m_c_ptr;
			};

			class C {
			public:
				mse::TRelaxedRegisteredPointer<D> m_d_ptr;
			};

			mse::TRelaxedRegisteredObj<C> regobjfl_c;
			mse::TRelaxedRegisteredPointer<D> d_ptr = mse::relaxed_registered_new<D>();

			regobjfl_c.m_d_ptr = d_ptr;
			d_ptr->m_c_ptr = &regobjfl_c;

			mse::relaxed_registered_delete<D>(d_ptr);

			{
				/* Polymorphic conversions. */
				class FD : public mse::TRelaxedRegisteredObj<D> {};
				mse::TRelaxedRegisteredObj<FD> relaxedregistered_fd;
				mse::TRelaxedRegisteredPointer<FD> FD_relaxedregistered_ptr1 = &relaxedregistered_fd;
				mse::TRelaxedRegisteredPointer<D> D_relaxedregistered_ptr4 = FD_relaxedregistered_ptr1;
				D_relaxedregistered_ptr4 = &relaxedregistered_fd;
				mse::TRelaxedRegisteredFixedPointer<D> D_relaxedregistered_fptr1 = &relaxedregistered_fd;
				mse::TRelaxedRegisteredFixedConstPointer<D> D_relaxedregistered_fcptr1 = &relaxedregistered_fd;

				/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
				class GD : public D {};
				mse::TRelaxedRegisteredObj<GD> relaxedregistered_gd;
				mse::TRelaxedRegisteredPointer<GD> GD_relaxedregistered_ptr1 = &relaxedregistered_gd;
				mse::TRelaxedRegisteredPointer<D> D_relaxedregistered_ptr5 = GD_relaxedregistered_ptr1;
				D_relaxedregistered_ptr5 = GD_relaxedregistered_ptr1;
				mse::TRelaxedRegisteredFixedPointer<GD> GD_relaxedregistered_fptr1 = &relaxedregistered_gd;
				D_relaxedregistered_ptr5 = &relaxedregistered_gd;
				mse::TRelaxedRegisteredFixedPointer<D> D_relaxedregistered_fptr2 = &relaxedregistered_gd;
				mse::TRelaxedRegisteredFixedConstPointer<D> D_relaxedregistered_fcptr2 = &relaxedregistered_gd;
			}
		}

		mse::s_regptr_test1();
		mse::s_relaxedregptr_test1();

		{
			/*************************/
			/*   Simple Benchmarks   */
			/*************************/

			/* Just some simple speed tests. */
			class CE {
			public:
				CE() {}
				CE(int& count_ref) : m_count_ptr(&count_ref) { (*m_count_ptr) += 1; }
				virtual ~CE() { (*m_count_ptr) -= 1; }
				int m_x;
				int *m_count_ptr;
			};
#ifndef NDEBUG
			static const int number_of_loops = 10/*arbitrary*/;
#else // !NDEBUG
			static const int number_of_loops = 1000000/*arbitrary*/;
#endif // !NDEBUG
			std::cout << std::endl;
			std::cout << "Some simple benchmarks: \n";
			std::cout << "number of loops: " << number_of_loops << " \n" << " \n";
			{
				int count = 0;
				auto item_ptr2 = new CE(count);
				delete item_ptr2; item_ptr2 = nullptr;
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto item_ptr = new CE(count);
					item_ptr2 = item_ptr;
					delete item_ptr;
					item_ptr = nullptr;
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "native pointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				mse::TRegisteredPointer<CE> item_ptr2 = mse::registered_new<CE>(count);
				mse::registered_delete<CE>(item_ptr2);
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					mse::TRegisteredPointer<CE> item_ptr = mse::registered_new<CE>(count);
					item_ptr2 = item_ptr;
					mse::registered_delete<CE>(item_ptr);
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRegisteredPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				mse::TRelaxedRegisteredPointer<CE> item_ptr2 = mse::relaxed_registered_new<CE>(count);
				mse::relaxed_registered_delete<CE>(item_ptr2);
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					mse::TRelaxedRegisteredPointer<CE> item_ptr = mse::relaxed_registered_new<CE>(count);
					item_ptr2 = item_ptr;
					mse::relaxed_registered_delete<CE>(item_ptr);
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRelaxedRegisteredPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				auto item_ptr2 = std::make_shared<CE>(count);
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					auto item_ptr = std::make_shared<CE>(count);
					item_ptr2 = item_ptr;
					item_ptr = nullptr;
				}
				item_ptr2 = nullptr;

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "std::shared_ptr: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				mse::TRegisteredPointer<CE> item_ptr2 = &(mse::TRegisteredObj<CE>(count));
				auto t1 = std::chrono::high_resolution_clock::now();
				{
					for (int i = 0; i < number_of_loops; i += 1) {
						mse::TRegisteredObj<CE> object(count);
						mse::TRegisteredPointer<CE> item_ptr = &object;
						item_ptr2 = item_ptr;
					}
				}

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRegisteredPointer targeting the stack: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}
			{
				int count = 0;
				mse::TRefCountingPointer<CE> item_ptr2 = mse::make_refcounting<CE>(count);
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < number_of_loops; i += 1) {
					mse::TRefCountingPointer<CE> item_ptr = mse::make_refcounting<CE>(count);
					item_ptr2 = item_ptr;
					item_ptr = nullptr;
				}
				item_ptr2 = nullptr;

				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRefCountingPointer: " << time_span.count() << " seconds.";
				if (0 != count) {
					std::cout << " destructions pending: " << count << "."; /* Using the count variable for (potential) output should prevent the optimizer from discarding it. */
				}
				std::cout << std::endl;
			}

			std::cout << std::endl;
			static const int number_of_loops2 = (10/*arbitrary*/)*number_of_loops;
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					CF* m_next_item_ptr;
					int m_a = 3;
				};
				CF item1(1);
				CF item2(2);
				CF item3(3);
				item1.m_next_item_ptr = &item2;
				item2.m_next_item_ptr = &item3;
				item3.m_next_item_ptr = &item1;
				auto t1 = std::chrono::high_resolution_clock::now();
				CF* cf_ptr = item1.m_next_item_ptr;
				for (int i = 0; i < number_of_loops2; i += 1) {
					cf_ptr = cf_ptr->m_next_item_ptr;
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "native pointer dereferencing: " << time_span.count() << " seconds.";
				if (3 == cf_ptr->m_a) {
					std::cout << " "; /* Using cf_ptr->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					mse::TRelaxedRegisteredPointer<CF> m_next_item_ptr;
					int m_a = 3;
				};
				mse::TRelaxedRegisteredObj<CF> item1(1);
				mse::TRelaxedRegisteredObj<CF> item2(2);
				mse::TRelaxedRegisteredObj<CF> item3(3);
				item1.m_next_item_ptr = &item2;
				item2.m_next_item_ptr = &item3;
				item3.m_next_item_ptr = &item1;
				auto t1 = std::chrono::high_resolution_clock::now();
				mse::TRelaxedRegisteredPointer<CF>* rpfl_ptr = std::addressof(item1.m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					rpfl_ptr = std::addressof((*rpfl_ptr)->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRelaxedRegisteredPointer (checked) dereferencing: " << time_span.count() << " seconds.";
				if (3 == (*rpfl_ptr)->m_a) {
					std::cout << " "; /* Using rpfl_ref->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					mse::TRelaxedRegisteredPointer<CF> m_next_item_ptr;
					int m_a = 3;
				};
				mse::TRelaxedRegisteredObj<CF> item1(1);
				mse::TRelaxedRegisteredObj<CF> item2(2);
				mse::TRelaxedRegisteredObj<CF> item3(3);
				item1.m_next_item_ptr = &item2;
				item2.m_next_item_ptr = &item3;
				item3.m_next_item_ptr = &item1;
				auto t1 = std::chrono::high_resolution_clock::now();
				CF* cf_ptr = (item1.m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					cf_ptr = (cf_ptr->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRelaxedRegisteredPointer unchecked dereferencing: " << time_span.count() << " seconds.";
				if (3 == cf_ptr->m_a) {
					std::cout << " "; /* Using rpfl_ref->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					std::weak_ptr<CF> m_next_item_ptr;
					int m_a = 3;
				};
				auto item1_ptr = std::make_shared<CF>(1);
				auto item2_ptr = std::make_shared<CF>(2);
				auto item3_ptr = std::make_shared<CF>(3);
				item1_ptr->m_next_item_ptr = item2_ptr;
				item2_ptr->m_next_item_ptr = item3_ptr;
				item3_ptr->m_next_item_ptr = item1_ptr;
				auto t1 = std::chrono::high_resolution_clock::now();
				std::weak_ptr<CF>* wp_ptr = &(item1_ptr->m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					wp_ptr = &((*wp_ptr).lock()->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "std::weak_ptr dereferencing: " << time_span.count() << " seconds.";
				if (3 == (*wp_ptr).lock()->m_a) {
					std::cout << " "; /* Using wp_ref.lock()->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
			{
				class CF {
				public:
					CF(int a = 0) : m_a(a) {}
					mse::TRefCountingPointer<CF> m_next_item_ptr;
					int m_a = 3;
				};
				auto item1_ptr = mse::make_refcounting<CF>(1);
				auto item2_ptr = mse::make_refcounting<CF>(2);
				auto item3_ptr = mse::make_refcounting<CF>(3);
				item1_ptr->m_next_item_ptr = item2_ptr;
				item2_ptr->m_next_item_ptr = item3_ptr;
				item3_ptr->m_next_item_ptr = item1_ptr;
				auto t1 = std::chrono::high_resolution_clock::now();
				mse::TRefCountingPointer<CF>* refc_ptr = &(item1_ptr->m_next_item_ptr);
				for (int i = 0; i < number_of_loops2; i += 1) {
					refc_ptr = &((*refc_ptr)->m_next_item_ptr);
				}
				auto t2 = std::chrono::high_resolution_clock::now();
				auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
				std::cout << "mse::TRefCountingPointer (checked) dereferencing: " << time_span.count() << " seconds.";
				item1_ptr->m_next_item_ptr = nullptr; /* to break the reference cycle */
				if (3 == (*refc_ptr)->m_a) {
					std::cout << " "; /* Using refc_ref->m_a for (potential) output should prevent the optimizer from discarding too much. */
				}
				std::cout << std::endl;
			}
		}
	}

#ifdef MSEREGISTEREDREFWRAPPER
	{
		/*****************************/
		/*   TRegisteredRefWrapper   */
		/*****************************/

		/* Stl provides a copyable, assignable wrapper for C++ references called std::reference_wrapper. std::reference_wrappers,
		like native C++ references, are not completely safe in the sense that the object they refer to can be deallocated while
		a reference to it is still available. So we provide mse::TRegisteredRefWrapper, a safe implementation of
		std::reference_wrapper that "knows" when the object being referenced has been deallocated and will throw an exception
		on any attempt to access the object after it has been destroyed.
		In most cases it is probably preferable to just use mse::TRegisteredFixedPointer instead of mse::TRegisteredRefWrapper. 
		*/
		{
			/* This example originally comes from http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper. */
			std::list<mse::TRegisteredObj<mse::CInt>> l(10);
			std::iota(l.begin(), l.end(), -4);

			std::vector<mse::TRegisteredRefWrapper<mse::CInt>> v(l.begin(), l.end());
			// can't use shuffle on a list (requires random access), but can use it on a vector
			std::shuffle(v.begin(), v.end(), std::mt19937{ std::random_device{}() });

			std::cout << '\n';
			std::cout << "TRegisteredRefWrapper test output: \n";
			std::cout << "Contents of the list: ";
			for (auto n : l) { std::cout << n << ' '; } std::cout << '\n';

			std::cout << "Contents of the list, as seen through a shuffled vector: ";
			for (auto i : v) { std::cout << (mse::CInt&)i << ' '; } std::cout << '\n';

			std::cout << "Doubling the values in the initial list...\n";
			for (auto& i : l) {
				i *= 2;
			}

			std::cout << "Contents of the list, as seen through a shuffled vector: ";
			for (auto i : v) { std::cout << (mse::CInt&)i << ' '; } std::cout << '\n';
			std::cout << '\n';
		}
		{
			/* This example originally comes from http://www.cplusplus.com/reference/functional/reference_wrapper/. */
			mse::TRegisteredObj<mse::CInt> a(10), b(20), c(30);
			// an array of "references":
			mse::TRegisteredRefWrapper<mse::CInt> refs[] = { a,b,c };
			std::cout << "refs:";
			for (mse::CInt& x : refs) std::cout << ' ' << x;
			std::cout << '\n';

			mse::TRegisteredObj<mse::CInt> foo(10);
			auto bar = mse::registered_ref(foo);
			//++bar;
			//++(mse::TRegisteredObj<mse::CInt>&)bar;
			++(mse::CInt&)bar;
			std::cout << foo << '\n';
		}
	}
#endif // MSEREGISTEREDREFWRAPPER

	{
		/*****************************/
		/*    TRefCountingPointer    */
		/*****************************/

		/* TRefCountingPointer behaves similar to std::shared_ptr. Some differences being that it foregoes any thread safety
		mechanisms, it does not accept raw pointer assignment or construction (use make_refcounting<>() instead), and it will throw
		an exception on attempted nullptr dereference. And it's faster. And like TRegisteredPointer, proper "const", "not null"
		and "fixed" (non-retargetable) versions are provided as well. */

		class A {
		public:
			A() {}
			A(const A& _X) : b(_X.b) {}
			virtual ~A() {
				int q = 3; /* just so you can place a breakpoint if you want */
			}
			A& operator=(const A& _X) { b = _X.b; return (*this); }

			int b = 3;
			std::string s = "some text ";
		};
		typedef std::vector<mse::TRefCountingFixedPointer<A>> CRCFPVector;
		class B {
		public:
			static int foo1(mse::TRefCountingPointer<A> A_refcounting_ptr, CRCFPVector& rcfpvector_ref) {
				rcfpvector_ref.clear();
				int retval = A_refcounting_ptr->b;
				A_refcounting_ptr = nullptr; /* Target object is destroyed here. */
				return retval;
			}
			static std::string foo2(mse::TStrongFixedPointer<std::string, mse::TRefCountingFixedPointer<A>> strong_string_ptr, CRCFPVector& rcfpvector_ref) {
				rcfpvector_ref.clear();
				std::string retval = (*strong_string_ptr);
				return retval;
			}
		protected:
			~B() {}
		};

		{
			CRCFPVector rcfpvector;
			{
				mse::TRefCountingFixedPointer<A> A_refcountingfixed_ptr1 = mse::make_refcounting<A>();
				rcfpvector.push_back(A_refcountingfixed_ptr1);

				/* Just to demonstrate conversion between refcounting pointer types. */
				mse::TRefCountingConstPointer<A> A_refcountingconst_ptr1 = A_refcountingfixed_ptr1;
			}
			B::foo1(rcfpvector.front(), rcfpvector);
		}
		{
			CRCFPVector rcfpvector;
			{
				mse::TRefCountingFixedPointer<A> A_refcountingfixed_ptr1 = mse::make_refcounting<A>();
				rcfpvector.push_back(A_refcountingfixed_ptr1);
			}

			/* strong_string_ptr1 here is essentially a pointer to "A.s" (the string member of class A) welded
			to a refcounting pointer to A to make sure that the object is not deallocated while strong_string_ptr1
			is still around. */
			auto strong_string_ptr1 = mse::make_strong(rcfpvector.front()->s, rcfpvector.front());
			B::foo2(strong_string_ptr1, rcfpvector);

			/* In practice, rather than declaring a specific mse::TStrongFixedPointer parameter, we expect
			functions to be "templatized" so that they can accept any type of pointer. */
			std::string res1 = H::foo6(strong_string_ptr1, strong_string_ptr1);
		}

		mse::TRefCountingPointer_test TRefCountingPointer_test1;
		bool TRefCountingPointer_test1_res = TRefCountingPointer_test1.testBehaviour();
		TRefCountingPointer_test1_res &= TRefCountingPointer_test1.testLinked();
		TRefCountingPointer_test1.test1();
	}

	{
		/*************************************/
		/*  TRefCountingOfRegisteredPointer  */
		/*************************************/

		/* TRefCountingOfRegisteredPointer is simply an alias for TRefCountingPointer<TRegisteredObj<_Ty>>. TRegisteredObj<_Ty> is
		meant to behave much like, and be compatible with a _Ty. The reason why we might want to use it is because the &
		("address of") operator of TRegisteredObj<_Ty> returns a TRegisteredFixedPointer<_Ty> rather than a raw pointer, and
		TRegisteredPointers can serve as safe "weak pointers".
		*/

		class A {
		public:
			A() {}
			A(const A& _X) : b(_X.b) {}
			virtual ~A() {
				int q = 3; /* just so you can place a breakpoint if you want */
			}
			A& operator=(const A& _X) { b = _X.b; return (*this); }

			int b = 3;
		};
		typedef std::vector<mse::TRefCountingOfRegisteredFixedPointer<A>> CRCRFPVector;

		{
			CRCRFPVector rcrfpvector;
			{
				mse::TRefCountingOfRegisteredFixedPointer<A> A_refcountingofregisteredfixed_ptr1 = mse::make_refcountingofregistered<A>();
				rcrfpvector.push_back(A_refcountingofregisteredfixed_ptr1);

				/* Just to demonstrate conversion between refcountingofregistered pointer types. */
				mse::TRefCountingOfRegisteredConstPointer<A> A_refcountingofregisteredconst_ptr1 = A_refcountingofregisteredfixed_ptr1;
			}
			int res1 = H::foo5(rcrfpvector.front(), rcrfpvector);
			assert(3 == res1);

#if !defined(MSE_REGISTEREDPOINTER_DISABLED)

			rcrfpvector.push_back(mse::make_refcountingofregistered<A>());
			/* The first parameter in this case will be a TRegisteredFixedPointer<A>. */
			int res2 = H::foo5(&(*rcrfpvector.front()), rcrfpvector);
			assert(-1 == res2);

#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
		}

		mse::TRefCountingOfRegisteredPointer_test TRefCountingOfRegisteredPointer_test1;
		bool TRefCountingOfRegisteredPointer_test1_res = TRefCountingOfRegisteredPointer_test1.testBehaviour();
		TRefCountingOfRegisteredPointer_test1_res &= TRefCountingOfRegisteredPointer_test1.testLinked();
		TRefCountingOfRegisteredPointer_test1.test1();
	}

	{
		/********************************************/
		/*  TRefCountingOfRelaxedRegisteredPointer  */
		/********************************************/

		/* TRefCountingOfRelaxedRegisteredPointer is simply an alias for TRefCountingPointer<TRelaxedRegisteredObj<_Ty>>. TRelaxedRegisteredObj<_Ty> is
		meant to behave much like, and be compatible with a _Ty. The reason why we might want to use it is because the &
		("address of") operator of TRelaxedRegisteredObj<_Ty> returns a TRelaxedRegisteredFixedPointer<_Ty> rather than a raw pointer, and
		TRelaxedRegisteredPointers can serve as safe "weak pointers".
		*/

		/* Here we demonstrate using TRelaxedRegisteredFixedPointer<> as a safe "weak_ptr" to prevent "cyclic references" from
		becoming memory leaks. */
		class CRCNode {
		public:
			CRCNode(mse::TRegisteredFixedPointer<mse::CInt> node_count_ptr
				, mse::TRelaxedRegisteredPointer<CRCNode> root_ptr) : m_node_count_ptr(node_count_ptr), m_root_ptr(root_ptr) {
				(*node_count_ptr) += 1;
			}
			CRCNode(mse::TRegisteredFixedPointer<mse::CInt> node_count_ptr) : m_node_count_ptr(node_count_ptr) {
				(*node_count_ptr) += 1;
			}
			virtual ~CRCNode() {
				(*m_node_count_ptr) -= 1;
			}
			static mse::TRefCountingOfRelaxedRegisteredFixedPointer<CRCNode> MakeRoot(mse::TRegisteredFixedPointer<mse::CInt> node_count_ptr) {
				auto retval = mse::make_refcountingofrelaxedregistered<CRCNode>(node_count_ptr);
				(*retval).m_root_ptr = &(*retval);
				return retval;
			}
			mse::TRefCountingOfRelaxedRegisteredPointer<CRCNode> ChildPtr() const { return m_child_ptr; }
			mse::TRefCountingOfRelaxedRegisteredFixedPointer<CRCNode> MakeChild() {
				auto retval = mse::make_refcountingofrelaxedregistered<CRCNode>(m_node_count_ptr, m_root_ptr);
				m_child_ptr = retval;
				return retval;
			}
			void DisposeOfChild() {
				m_child_ptr = nullptr;
			}

		private:
			mse::TRegisteredFixedPointer<mse::CInt> m_node_count_ptr;
			mse::TRefCountingOfRelaxedRegisteredPointer<CRCNode> m_child_ptr;
			mse::TRelaxedRegisteredPointer<CRCNode> m_root_ptr;
		};

		mse::TRegisteredObj<mse::CInt> node_counter = 0;
		{
			mse::TRefCountingOfRelaxedRegisteredPointer<CRCNode> root_ptr = CRCNode::MakeRoot(&node_counter);
			auto kid1 = root_ptr->MakeChild();
			{
				auto kid2 = kid1->MakeChild();
				auto kid3 = kid2->MakeChild();
			}
			assert(4 == node_counter);
			kid1->DisposeOfChild();
			assert(2 == node_counter);
		}
		assert(0 == node_counter);

		mse::TRefCountingOfRelaxedRegisteredPointer_test TRefCountingOfRelaxedRegisteredPointer_test1;
		bool TRefCountingOfRelaxedRegisteredPointer_test1_res = TRefCountingOfRelaxedRegisteredPointer_test1.testBehaviour();
		TRefCountingOfRelaxedRegisteredPointer_test1_res &= TRefCountingOfRelaxedRegisteredPointer_test1.testLinked();
		TRefCountingOfRelaxedRegisteredPointer_test1.test1();
	}

	{
		/************************/
		/*  TScopeFixedPointer  */
		/************************/

		/* The "scope" templates basically just allow the programmer to indicate that the target object has "scope
		lifetime". That is, the object is either allocated on the stack, or it's "owner" pointer is allocated on
		the stack. Unfortunately there's really no way to enforce this, which makes this data type less intrinsically
		safe than say, "reference counting" pointers. Because of this, "scope" pointers can optionally use relaxed
		registered pointers as their base class, thereby inheriting their safety features. */

		class A {
		public:
			A(int x) : b(x) {}
			A(const A& _X) : b(_X.b) {}
			virtual ~A() {}
			A& operator=(const A& _X) { b = _X.b; return (*this); }

			int b = 3;
		};
		class B {
		public:
			static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
			static int foo2(mse::TScopeFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
			static int foo3(mse::TScopeFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
		protected:
			~B() {}
		};

		mse::TScopeObj<A> a_scpobj(5);
		int res1 = (&a_scpobj)->b;
		int res2 = B::foo2(&a_scpobj);
		int res3 = B::foo3(&a_scpobj);
		mse::TScopeOwnerPointer<A> a_scpoptr(7);
		int res4 = B::foo2(&(*a_scpoptr));

		mse::s_scpptr_test1();
	}

	return 0;
}


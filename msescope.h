
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSESCOPE_H_
#define MSESCOPE_H_

#include "mseprimitives.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

/* Defining MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED will cause relaxed registered pointers to be used to help catch
misuse of scope pointers in debug mode. Additionally defining MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED will cause
mse::TRelaxedRegisteredObj to be used in non-debug modes as well. */
#ifdef MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED
#include "mserelaxedregistered.h"
#endif // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED


#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_SCOPEPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

#ifdef NDEBUG
#ifndef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#define MSE_SCOPEPOINTER_DISABLED
#endif // !MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#endif // NDEBUG


namespace mse {

#ifdef MSE_SCOPEPOINTER_DISABLED
	template<typename _Ty> using TXScopePointer = _Ty*;
	template<typename _Ty> using TXScopeConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeNotNullPointer = _Ty*;
	template<typename _Ty> using TXScopeNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeFixedPointer = _Ty*;
	template<typename _Ty> using TXScopeFixedConstPointer = const _Ty*;
	template<typename _TROy> using TXScopeObj = _TROy;

#else /*MSE_SCOPEPOINTER_DISABLED*/

	/* This macro roughly simulates constructor inheritance. Originally it was used when some compilers didn't support
	constructor inheritance, but now we use it because of it's differences with standard constructor inheritance. */
#define MSE_SCOPE_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if<std::is_constructible<Base, Args...>::value>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

#ifdef MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

	template<typename _Ty> using TXScopePointerBase = mse::TRelaxedRegisteredPointer<_Ty>;
	template<typename _Ty> using TXScopeConstPointerBase = mse::TRelaxedRegisteredConstPointer<_Ty>;
	template<typename _TROz> using TXScopeObjBase = mse::TRelaxedRegisteredObj<_TROz>;

#else // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

	template<typename _Ty> using TXScopePointerBase = TSaferPtrForLegacy<_Ty>;
	template<typename _Ty> using TXScopeConstPointerBase = TSaferPtrForLegacy<const _Ty>;

	template<typename _TROz>
	class TXScopeObjBase : public _TROz {
	public:
		MSE_SCOPE_USING(TXScopeObjBase, _TROz);
	};

#endif // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

	template<typename _Ty> class TXScopeObj;
	template<typename _Ty> class TXScopeNotNullPointer;
	template<typename _Ty> class TXScopeNotNullConstPointer;
	template<typename _Ty> class TXScopeFixedPointer;
	template<typename _Ty> class TXScopeFixedConstPointer;
	template<typename _Ty> class TXScopeOwnerPointer;

	/* Use TXScopeFixedPointer instead. */
	template<typename _Ty>
	class TXScopePointer : public TXScopePointerBase<_Ty> {
	public:
	private:
		TXScopePointer() : TXScopePointerBase<_Ty>() {}
		TXScopePointer(TXScopeObj<_Ty>* ptr) : TXScopePointerBase<_Ty>(ptr) {}
		TXScopePointer(const TXScopePointer& src_cref) : TXScopePointerBase<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePointer(const TXScopePointer<_Ty2>& src_cref) : TXScopePointerBase<_Ty>(src_cref) {}
		virtual ~TXScopePointer() {}
		TXScopePointer<_Ty>& operator=(TXScopeObj<_Ty>* ptr) {
			return TXScopePointerBase<_Ty>::operator=(ptr);
		}
		TXScopePointer<_Ty>& operator=(const TXScopePointer<_Ty>& _Right_cref) {
			return TXScopePointerBase<_Ty>::operator=(_Right_cref);
		}
		operator bool() const {
			bool retval = ((*static_cast<const TXScopePointerBase<_Ty>*>(this)) != nullptr);
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const {
			_Ty* retval = (*static_cast<const TXScopePointerBase<_Ty>*>(this));
			return retval;
		}
		explicit operator TXScopeObj<_Ty>*() const {
			TXScopeObj<_Ty>* retval = (*static_cast<const TXScopePointerBase<_Ty>*>(this));
			return retval;
		}

		TXScopePointer<_Ty>* operator&() { return this; }
		const TXScopePointer<_Ty>* operator&() const { return this; }

		friend class TXScopeNotNullPointer<_Ty>;
	};

	/* Use TXScopeFixedConstPointer instead. */
	template<typename _Ty>
	class TXScopeConstPointer : public TXScopeConstPointerBase<const _Ty> {
	public:
	private:
		TXScopeConstPointer() : TXScopeConstPointerBase<const _Ty>() {}
		TXScopeConstPointer(const TXScopeObj<_Ty>* ptr) : TXScopeConstPointerBase<const _Ty>(ptr) {}
		TXScopeConstPointer(const TXScopeConstPointer& src_cref) : TXScopeConstPointerBase<const _Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeConstPointer(const TXScopeConstPointer<_Ty2>& src_cref) : TXScopeConstPointerBase<const _Ty>(src_cref) {}
		TXScopeConstPointer(const TXScopePointer<_Ty>& src_cref) : TXScopeConstPointerBase<const _Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeConstPointer(const TXScopePointer<_Ty2>& src_cref) : TXScopeConstPointerBase<const _Ty>(src_cref) {}
		virtual ~TXScopeConstPointer() {}
		TXScopeConstPointer<_Ty>& operator=(const TXScopeObj<_Ty>* ptr) {
			return TXScopeConstPointerBase<_Ty>::operator=(ptr);
		}
		TXScopeConstPointer<_Ty>& operator=(const TXScopeConstPointer<_Ty>& _Right_cref) {
			return TXScopeConstPointerBase<_Ty>::operator=(_Right_cref);
		}
		TXScopeConstPointer<_Ty>& operator=(const TXScopePointer<_Ty>& _Right_cref) { return (*this).operator=(TXScopeConstPointer(_Right_cref)); }
		operator bool() const {
			bool retval = (*static_cast<const TXScopeConstPointerBase<_Ty>*>(this));
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const {
			const _Ty* retval = (*static_cast<const TXScopeConstPointerBase<const _Ty>*>(this));
			return retval;
		}
		explicit operator const TXScopeObj<_Ty>*() const {
			const TXScopeObj<_Ty>* retval = (*static_cast<const TXScopeConstPointerBase<const _Ty>*>(this));
			return retval;
		}

		TXScopeConstPointer<_Ty>* operator&() { return this; }
		const TXScopeConstPointer<_Ty>* operator&() const { return this; }

		friend class TXScopeNotNullConstPointer<_Ty>;
	};

	/* Use TXScopeFixedPointer instead. */
	template<typename _Ty>
	class TXScopeNotNullPointer : public TXScopePointer<_Ty> {
	public:
	private:
		TXScopeNotNullPointer(TXScopeObj<_Ty>* ptr) : TXScopePointer<_Ty>(ptr) {}
		TXScopeNotNullPointer(const TXScopeNotNullPointer& src_cref) : TXScopePointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeNotNullPointer(const TXScopeNotNullPointer<_Ty2>& src_cref) : TXScopePointer<_Ty>(src_cref) {}
		virtual ~TXScopeNotNullPointer() {}
		TXScopeNotNullPointer<_Ty>& operator=(const TXScopePointer<_Ty>& _Right_cref) {
			TXScopePointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		operator bool() const { return (*static_cast<const TXScopePointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TXScopePointer<_Ty>::operator _Ty*(); }
		explicit operator TXScopeObj<_Ty>*() const { return TXScopePointer<_Ty>::operator TXScopeObj<_Ty>*(); }

		TXScopeNotNullPointer<_Ty>* operator&() { return this; }
		const TXScopeNotNullPointer<_Ty>* operator&() const { return this; }

		friend class TXScopeFixedPointer<_Ty>;
	};

	/* Use TXScopeFixedConstPointer instead. */
	template<typename _Ty>
	class TXScopeNotNullConstPointer : public TXScopeConstPointer<_Ty> {
	public:
	private:
		TXScopeNotNullConstPointer(const TXScopeNotNullConstPointer<_Ty>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeNotNullConstPointer(const TXScopeNotNullConstPointer<_Ty2>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		TXScopeNotNullConstPointer(const TXScopeNotNullPointer<_Ty>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeNotNullConstPointer(const TXScopeNotNullPointer<_Ty2>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		virtual ~TXScopeNotNullConstPointer() {}
		operator bool() const { return (*static_cast<const TXScopeConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TXScopeConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TXScopeObj<_Ty>*() const { return TXScopeConstPointer<_Ty>::operator const TXScopeObj<_Ty>*(); }
		TXScopeNotNullConstPointer(const TXScopeObj<_Ty>* ptr) : TXScopeConstPointer<_Ty>(ptr) {}

		TXScopeNotNullConstPointer<_Ty>* operator&() { return this; }
		const TXScopeNotNullConstPointer<_Ty>* operator&() const { return this; }

		friend class TXScopeFixedConstPointer<_Ty>;
	};

	/* A TXScopeFixedPointer points to a TXScopeObj. Its intended for very limited use. Basically just to pass a TXScopeObj
	by reference as a function parameter. TXScopeFixedPointers can be obtained from TXScopeObj's "&" (address of) operator. */
	template<typename _Ty>
	class TXScopeFixedPointer : public TXScopeNotNullPointer<_Ty> {
	public:
		TXScopeFixedPointer(const TXScopeFixedPointer& src_cref) : TXScopeNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopeNotNullPointer<_Ty>(src_cref) {}
		virtual ~TXScopeFixedPointer() {}
		operator bool() const { return (*static_cast<const TXScopeNotNullPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TXScopeNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TXScopeObj<_Ty>*() const { return TXScopeNotNullPointer<_Ty>::operator TXScopeObj<_Ty>*(); }

	private:
		TXScopeFixedPointer(TXScopeObj<_Ty>* ptr) : TXScopeNotNullPointer<_Ty>(ptr) {}
		TXScopeFixedPointer<_Ty>& operator=(const TXScopeFixedPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeFixedPointer<_Ty>* operator&() { return this; }
		const TXScopeFixedPointer<_Ty>* operator&() const { return this; }

		friend class TXScopeObj<_Ty>;
	};

	template<typename _Ty>
	class TXScopeFixedConstPointer : public TXScopeNotNullConstPointer<_Ty> {
	public:
		TXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		virtual ~TXScopeFixedConstPointer() {}
		operator bool() const { return (*static_cast<const TXScopeNotNullConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TXScopeNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TXScopeObj<_Ty>*() const { return TXScopeNotNullConstPointer<_Ty>::operator const TXScopeObj<_Ty>*(); }

	private:
		TXScopeFixedConstPointer(const TXScopeObj<_Ty>* ptr) : TXScopeNotNullConstPointer<_Ty>(ptr) {}
		TXScopeFixedConstPointer<_Ty>& operator=(const TXScopeFixedConstPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }


		TXScopeFixedConstPointer<_Ty>* operator&() { return this; }
		const TXScopeFixedConstPointer<_Ty>* operator&() const { return this; }

		friend class TXScopeObj<_Ty>;
	};

	/* TXScopeObj is intended as a transparent wrapper for other classes/objects with "scope lifespans". That is, objects
	that are either allocated on the stack, or whose "owning" pointer is allocated on the stack. Unfortunately it's not
	really possible to prevent misuse. For example, std::list<TXScopeObj<mse::CInt>> is an improper, and dangerous, use
	of TXScopeObj<>. So we provide the option of using an mse::TRelaxedRegisteredObj as TXScopeObj's base class for
	enhanced safety and to help catch misuse. Defining MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED will cause
	mse::TRelaxedRegisteredObj to be used in debug mode. Additionally defining MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	will cause mse::TRelaxedRegisteredObj to be used in non-debug modes as well. */
	template<typename _TROy>
	class TXScopeObj : public TXScopeObjBase<_TROy> {
	public:
		MSE_SCOPE_USING(TXScopeObj, TXScopeObjBase<_TROy>);
		TXScopeObj(const TXScopeObj& _X) : TXScopeObjBase<_TROy>(_X) {}
		virtual ~TXScopeObj() {}
		using _TROy::operator=;
		TXScopeObj& operator=(const TXScopeObj& _X) { TXScopeObjBase<_TROy>::operator=(_X); return (*this); }
		TXScopeFixedPointer<_TROy> operator&() {
			return this;
		}
		TXScopeFixedConstPointer<_TROy> operator&() const {
			return this;
		}

	private:
		TXScopeObj(TXScopeObj&& _X) = delete;
		TXScopeObj& operator=(TXScopeObj&& _X) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		friend class TXScopeOwnerPointer<_TROy>;
	};

#endif /*MSE_SCOPEPOINTER_DISABLED*/

	/* TXScopeOwnerPointer is meant to be much like boost::scoped_ptr<>. Instead of taking a native pointer,
	TXScopeOwnerPointer just forwards it's constructor arguments to the constructor of the TXScopeObj<_Ty>.
	TXScopeOwnerPointers are meant to be allocated on the stack only. Unfortunately there's really no way to
	enforce this, which makes this data type less intrinsically safe than say, "reference counting" pointers.
	*/
	template<typename _Ty>
	class TXScopeOwnerPointer {
	public:
		template <class... Args>
		TXScopeOwnerPointer(Args&&... args) {
			TXScopeObj<_Ty>* new_ptr = new TXScopeObj<_Ty>(std::forward<Args>(args)...);
			m_ptr = new_ptr;
		}
		virtual ~TXScopeOwnerPointer() {
			assert(m_ptr);
			delete m_ptr;
		}

		TXScopeObj<_Ty>& operator*() const {
			return (*m_ptr);
		}
		TXScopeObj<_Ty>* operator->() const {
			return m_ptr;
		}

	private:
		TXScopeOwnerPointer(TXScopeOwnerPointer<_Ty>& src_cref) = delete;
		TXScopeOwnerPointer<_Ty>& operator=(const TXScopeOwnerPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeObj<_Ty>* m_ptr = nullptr;
	};

	template <class _TTargetType, class _TXScopePointerType = void*> class TXScopeWeakFixedConstPointer;

	/* If, for example, you want a safe pointer to a member of a registered pointer target, you can use a
	TXScopeWeakFixedPointer to store a copy of the registered pointer along with the pointer targeting the
	member. */
	template <class _TTargetType, class _TXScopePointerType = void*>
	class TXScopeWeakFixedPointer {
	public:
		_TTargetType& operator*() const {
			return (*m_target_pointer);
		}
		_TTargetType* operator->() const {
			return m_target_pointer;
		}

		template <class _TTargetType2, class _TXScopePointerType2>
		static TXScopeWeakFixedPointer make_xscopeweak(_TTargetType2& target, const _TXScopePointerType2& /*xscope_pointer*/) {
			return TXScopeWeakFixedPointer(target/*, xscope_pointer*/);
		}
		template <class _TTargetType2>
		static TXScopeWeakFixedPointer make_xscopeweak(_TTargetType2& target) {
			return TXScopeWeakFixedPointer(target/*, xscope_pointer*/);
		}

	private:
		TXScopeWeakFixedPointer(_TTargetType& target/* often a struct member */, _TXScopePointerType xscope_pointer/* an xscope pointer */)
			: m_target_pointer(&target) {}
		TXScopeWeakFixedPointer(_TTargetType& target/* often a struct member */) : m_target_pointer(&target) {}
		TXScopeWeakFixedPointer& operator=(const TXScopeWeakFixedPointer& _Right_cref) = delete;

		_TTargetType* m_target_pointer;
		friend class TXScopeWeakFixedConstPointer<_TTargetType, _TXScopePointerType>;
	};

	template <class _TTargetType, class _TXScopePointerType>
	TXScopeWeakFixedPointer<_TTargetType, _TXScopePointerType> make_xscopeweak(_TTargetType& target, const _TXScopePointerType& /*xscope_pointer*/) {
		return TXScopeWeakFixedPointer<_TTargetType, _TXScopePointerType>::make_xscopeweak(target/*, xscope_pointer*/);
	}
	template <class _TTargetType>
	TXScopeWeakFixedPointer<_TTargetType> make_xscopeweak(_TTargetType& target) {
		return TXScopeWeakFixedPointer<_TTargetType>::make_xscopeweak(target/*, xscope_pointer*/);
	}

	template <class _TTargetType, class _TXScopePointerType>
	class TXScopeWeakFixedConstPointer {
	public:
		TXScopeWeakFixedConstPointer(const TXScopeWeakFixedConstPointer&) = default;
		TXScopeWeakFixedConstPointer(const TXScopeWeakFixedPointer<_TTargetType, _TXScopePointerType>&src) : m_target_pointer(src.m_target_pointer) {}
		const _TTargetType& operator*() const {
			return (*m_target_pointer);
		}
		const _TTargetType* operator->() const {
			return m_target_pointer;
		}

	private:
		TXScopeWeakFixedConstPointer(const _TTargetType& target/* often a struct member */, _TXScopePointerType xscope_pointer/* an xscope pointer */)
			: m_target_pointer(&target) {}
		TXScopeWeakFixedConstPointer(const _TTargetType& target/* often a struct member */) : m_target_pointer(&target) {}
		TXScopeWeakFixedConstPointer& operator=(const TXScopeWeakFixedConstPointer& _Right_cref) = delete;

		const _TTargetType* m_target_pointer;
	};

	/* shorter aliases */
	//template<typename _Ty> using scpp = TXScopePointer<_Ty>;
	//template<typename _Ty> using scpcp = TXScopeConstPointer<_Ty>;
	//template<typename _Ty> using scpnnp = TXScopeNotNullPointer<_Ty>;
	//template<typename _Ty> using scpnncp = TXScopeNotNullConstPointer<_Ty>;
	template<typename _Ty> using scpfp = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using scpfcp = TXScopeFixedConstPointer<_Ty>;
	template<typename _TROy> using scpo = TXScopeObj<_TROy>;
	template<class _TTargetType, class _TXScopePointerType> using scpwkfp = TSyncWeakFixedPointer<_TTargetType, _TXScopePointerType>;
	template<class _TTargetType, class _TXScopePointerType> using scpwkfcp = TSyncWeakFixedConstPointer<_TTargetType, _TXScopePointerType>;

	/* deprecated aliases */
	template<typename _Ty> using TScopeFixedPointer = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using TScopeFixedConstPointer = TXScopeFixedConstPointer<_Ty>;
	template<typename _TROy> using TScopeObj = TXScopeObj<_TROy>;
	template<typename _Ty> using TScopeOwnerPointer = TXScopeOwnerPointer<_Ty>;


	static void s_scpptr_test1() {
		class A {
		public:
			A(int x) : b(x) {}
			A(const A& _X) : b(_X.b) {}
			A(A&& _X) : b(std::move(_X.b)) {}
			virtual ~A() {}
			A& operator=(A&& _X) { b = std::move(_X.b); return (*this); }
			A& operator=(const A& _X) { b = _X.b; return (*this); }

			int b = 3;
		};
		class B {
		public:
			static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
			static int foo2(mse::TXScopeFixedPointer<A> A_scope_ptr) { return A_scope_ptr->b; }
		protected:
			~B() {}
		};

		A* A_native_ptr = nullptr;

		{
			A a(7);
			mse::TXScopeObj<A> scope_a(7);
			/* mse::TXScopeObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
			in almost all cases. */

			assert(a.b == scope_a.b);
			A_native_ptr = &a;
			mse::TXScopeFixedPointer<A> A_scope_ptr1 = &scope_a;
			assert(A_native_ptr->b == A_scope_ptr1->b);

			mse::TXScopeFixedPointer<A> A_scope_ptr2 = &scope_a;

			/* mse::TXScopeFixedPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
			B::foo1((A*)A_scope_ptr1);

			if (A_scope_ptr2) {
			}
			else if (A_scope_ptr2 != A_scope_ptr1) {
				int q = B::foo2(A_scope_ptr2);
			}

			A a2 = a;
			mse::TXScopeObj<A> scope_a2 = scope_a;
			scope_a2 = a;
			scope_a2 = scope_a;

			mse::TXScopeFixedConstPointer<A> rcp = A_scope_ptr1;
			mse::TXScopeFixedConstPointer<A> rcp2 = rcp;
			const mse::TXScopeObj<A> cscope_a(11);
			mse::TXScopeFixedConstPointer<A> rfcp = &cscope_a;

			mse::TXScopeOwnerPointer<A> A_scpoptr(11);
			B::foo2(&*A_scpoptr);
			if (A_scpoptr->b == (&*A_scpoptr)->b) {
			}
		}

		{
			/* Polymorphic conversions. */
			class E {
			public:
				int m_b = 5;
			};
			class FE : public mse::TXScopeObj<E> {};
			mse::TXScopeObj<FE> scope_fd;
			mse::TXScopeFixedPointer<FE> FE_scope_fptr1 = &scope_fd;
			mse::TXScopeFixedPointer<E> E_scope_ptr4 = FE_scope_fptr1;
			mse::TXScopeFixedPointer<E> E_scope_fptr1 = &scope_fd;
			mse::TXScopeFixedConstPointer<E> E_scope_fcptr1 = &scope_fd;

			/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
			class GE : public E {};
			mse::TXScopeObj<GE> scope_gd;
			mse::TXScopeFixedPointer<GE> GE_scope_fptr1 = &scope_gd;
			mse::TXScopeFixedPointer<E> E_scope_ptr5 = GE_scope_fptr1;
			mse::TXScopeFixedPointer<E> E_scope_fptr2 = &scope_gd;
			mse::TXScopeFixedConstPointer<E> E_scope_fcptr2 = &scope_gd;
		}
	}
}

#endif // MSESCOPE_H_
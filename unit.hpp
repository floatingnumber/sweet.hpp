/* unit.hpp - a simple unit test framework for C++ based on QUnit and Testdog
Author: Robert "burner" Schadek rburners@gmail.com License: LGPL 3 or higher

Example:
#include <fstream>
#include "unit.hpp"
	
UNITTEST(fancyname) {
	// std::ofstream unittestlog("testlog.tst"); 	optional
	// this->setOutputStream(&unittestlog); 		optional
	AS_T(true);
	AS_F(4!=4);
	AS_EQ(42, 42);
	AS_NEQ(42, 43);

	if(AS_T(true)) {
		// do a test that is only possible if the previous test passed
	}
}

int main() {
	if(!Unit::runTests()) {
		return 1;
	}
}
*/

#ifndef SWEET_UNIT_HPP
#define SWEET_UNIT_HPP
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <type_traits>
#include <limits>

#define UNITTEST(test_name) \
class test_name##_test_class : public Unit::Unittest { void run_impl(); \
public: \
test_name##_test_class() : Unit::Unittest(#test_name,__FILE__,__LINE__) {} \
} test_name##_test_class_impl; \
void test_name##_test_class::run_impl()

#define AS_EQ(e1,e2)		UNIT_COMPARE(true,true,e1,e2)
#define AS_NEQ(e1,e2)		UNIT_COMPARE(true,false,e1,e2)
#define AS_T(e)				UNIT_COMPARE(false,true,e,true)
#define AS_F(e)				UNIT_COMPARE(false,true,e,false)
#define ASSERT_EQ(e1,e2)	UNIT_COMPARED(true,true,e1,e2)
#define ASSERT_NEQ(e1,e2)	UNIT_COMPARED(true,false,e1,e2)
#define ASSERT_T(e)			UNIT_COMPARED(false,true,e,true)
#define ASSERT_F(e)			UNIT_COMPARED(false,true,e,false)

#define UNIT_COMPARE(compare,result,e1,e2) evaluate(compare,			  \
result, e1, e2, #e1, #e2,Unit::sname(__FILE__), __LINE__)
#define UNIT_COMPARED(compare,result,e1,e2) Unit::Unittest::evaluates(compare,  \
result, e1, e2, #e1, #e2,Unit::sname(__FILE__), __LINE__, &std::cout, "", true)

namespace Unit {
	using namespace std;
	static inline string sname(const string& str) {
		size_t idx(str.rfind('/'));
		if(idx != string::npos) {
			string ret(str);
			ret.erase(0, idx+1);	
			return ret;
		} else
			return string(str);
	}

	template<bool>
	struct comp_sel {
		template<typename T, typename S>
		static bool comp(const T t, const S s) {
			return t == s;
		}
	};

	template<>
	struct comp_sel<true> {
		template<typename T, typename S>
		static bool comp(const T t, const S s) {
			if(std::numeric_limits<T>::infinity() == t &&
					std::numeric_limits<S>::infinity() == s) {
				return true;
			}
			return fabs(t - s) <= 0.0001;
		}
	};
	
	class Unittest;

	inline vector<Unittest*>& getTests() {
		static vector<Unittest*> tests;
		return tests;
	}

	class Unittest {
	public:
		Unittest(const string& name, std::string f, int l) : file(sname(f)),
				line(l), name_(name), errors_(0), out_(&cerr) {
			getTests().push_back(this);
		}

		template<typename E1, typename E2> static bool evaluates(bool compare, 
				bool result, const E1& e1, const E2& e2, const string& str1, 
				const string& str2, const string& file, int line,
				ostream* out, const string& name, bool die) {

			//if(result ? (e1 == e2) : (e1 != e2)) return true;
			if(result ? 
					(comp_sel<is_floating_point<E1>::value>::comp(e1, e2)) :
					(!comp_sel<is_floating_point<E1>::value>::comp(e1, e2))) {
				return true;
			}

			if(name != "") {
				*out<<sname(file)<< ":"<<line<<" in "<<"Unittest("<<name<<
					") Assert Failed: ";
			} else {
				*out<<sname(file)<< ":"<<line<<" Assert Failed: ";
			}
			stringstream s2;
			s2<<boolalpha<<(e2);

			if(compare) {
				stringstream s1;
				s1<<boolalpha<<(e1);
  
				const string cmp(result ? "==" : "!=");
				*out<<"compare {"<<str1<<"} "<< cmp<<" {"<<str2<<"} "<<"got {\""<<
					s1.str()<<"\"} "<<cmp<<" {\""<<s2.str()<< "\"}";
			} else {
				*out<<"evalute {"<<str1<<"} == "<<s2.str();
			}
			*out<<endl;
			if(die) {
				exit(1);
			}
			return false;
		}

		template<typename E1, typename E2> bool evaluate(bool compare, 
				bool result, const E1& e1, const E2& e2, const string& str1, 
				const string& str2, const string& file, int line) {
			bool rlst = Unittest::evaluates<E1,E2>(compare, result, e1, e2,
					str1, str2, file, line, out_, name_, false);
			if(!rlst) {
				++errors_;
			}

			return rlst;
		}

		virtual void run_impl() = 0;

		bool run() {
			run_impl();
			return errors_;
		}

		void setOutputStream(ostream* o) {
			out_ = o;
		}

		std::string file;
		int line;
		string name_;

	private:
		int errors_;
		ostream* out_;
	};

	inline bool runTests() {
		bool rs(true);
		for(vector<Unittest*>::iterator it = getTests().begin(); it !=
				getTests().end(); ++it) {
			try {
				bool tmp = (*it)->run();
				rs &= !tmp;
			} catch(std::exception& e) {
				std::cerr<<(*it)->file<<":"<<(*it)->line<<" Unittest"<<
					(*it)->name_<<" has thrown an "<< "uncaught exception "<<
					" with message "<<e.what()<<std::endl;
				rs &= false;
			} catch(...) {
				std::cerr<<(*it)->file<<":"<<(*it)->line<<" Unittest"<<
					(*it)->name_<<" has thrown an "<< "uncaught exception "
					<<std::endl;
				rs &= false;
			}
		}
		return rs;
	}
}
#endif

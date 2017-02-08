/**************************************************************************
vtest.h

a simple testing framework

...
**************************************************************************/
#ifndef __V_TEST_H__
#define __V_TEST_H__

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <vector>
#include <string>
#include <map>

#define VTEST_VERSION "2018"

#ifdef _MSC_VER
#include <Windows.h>
#ifndef snprintf
#define snprintf _snprintf_s
#endif
#include <unordered_map>
#define UT_HASH_MAP std::unordered_map
#else
// NOT C++11
#include <tr1/unordered_map>
#define UT_HASH_MAP std::tr1::unordered_map
#endif

namespace vtest
{
    class console
    {
    public:
        static console& instance()
        {
            static console obj_;
            return obj_;
        }
#ifdef _MSC_VER
        static void reset_color_mode()
        {
            SetConsoleTextAttribute(get_console(), 7);
            fflush(stdout);
        }
        static void set_color_mode_passed()
        {
            SetConsoleTextAttribute(get_console(), 10);
        }
        static void set_color_mode_failed()
        {
            SetConsoleTextAttribute(get_console(), 12);
        }
        static void set_color_mode_tip()
        {
            SetConsoleTextAttribute(get_console(), 11);
        }
        static HANDLE get_console()
        {
            static HANDLE h_cons = NULL;
            if (h_cons == NULL) {
                h_cons = GetStdHandle(STD_OUTPUT_HANDLE);
            }
            return h_cons;
        }
#else
        static void reset_color_mode()
        {
            printf("%s", "\033[m");
            fflush(stdout);
        }
        static void set_color_mode_passed()
        {
            printf("%s", "\033[1;32;32m");
        }
        static void set_color_mode_failed()
        {
            printf("%s", "\033[1;31m");
        }
        static void set_color_mode_tip()
        {
            printf("%s", "\033[36m");
        }
#endif
    private:
        console() {}
    };
    static console& ut_cons = console::instance();

    class unit_test
    {
    public:
        void set_exit_on_failed(bool value) { exit_on_failed_ = value; }
        void set_pause_on_exit(bool value) { pause_on_exit_ = value; }
        void set_report_detail(bool value) { report_detail_ = value; }
        int run_all()
        {
            typedef void(*UNITTEST_PROC)(void);
            printf("--------------------------------------------------\n");
            printf("Unit test start with vTest %s...\n", VTEST_VERSION);
            printf("--------------------------------------------------\n");
            std::vector<func_info>::iterator it;
            while (!funcs_.empty())
            {
                std::vector<func_info> funcs;
                funcs.swap(funcs_);
                for (it = funcs.begin(); it != funcs.end(); it++)
                {
                    if (level_filter_)
                    {
                        map_it_ = map_run_level_.find(it->level);
                        if (level_check_) {
                            if (map_it_ != map_run_level_.end()
                                && map_it_->second == 1)
                            {
                                printf("\n[run] %s\n", it->func.c_str());
                                run_++;
                                (UNITTEST_PROC(it->ptr))();
                            }
                        }
                        else if (map_it_ == map_run_level_.end()
                            || map_it_->second == 1)
                        {
                            printf("\n[Run] %s\n", it->func.c_str());
                            run_++;
                            (UNITTEST_PROC(it->ptr))();
                        }
                    }
                    else {
                        printf("\n[Run] %s\n", it->func.c_str());
                        run_++;
                        (UNITTEST_PROC(it->ptr))();
                    }
                }
            }
            printf("--------------------------------------------------\n");
            printf("Unit test end.\n");
            printf("--------------------------------------------------\n");
            show_result();
            if (pause_on_exit_) {
                printf("Press any key to exit...\n");
                getchar();
            }
            return count_ - pass_;
        }
    public:
        void check_eq(bool eq, const char* fn, int ln, const char* fp, int row)
        {
            count_++;
            if (eq == true) {
                pass_++;
                ut_cons.set_color_mode_passed();
                printf("PASS %s, line %d, case %d\n", fn, ln, row);
                ut_cons.reset_color_mode();
            }
            else {
                ut_cons.set_color_mode_failed();
                char buf[128];
                snprintf(buf, 128, "ERROR %s, line %d, case %d, %s\n",
                    fn, ln, row, get_file_name(fp));
                printf(buf);
                ut_cons.reset_color_mode();
                errs_.push_back(std::string(buf));
                if (exit_on_failed_) {
                    show_result();
                    if (pause_on_exit_) {
                        printf("Press any key to exit...\n");
                        getchar();
                    }
                    exit(1);
                }
            }
        }
        void show_result()
        {
            ut_cons.set_color_mode_passed();
            printf("--------------------------------------------------\n");
            printf("Run %d, Test %d, Pass %d, ", run_, count_, pass_);
            if (count_ != pass_) {
                ut_cons.set_color_mode_failed();
            }
            printf("Failed %d\n", count_ - pass_);
            printf("--------------------------------------------------\n");
            if (report_detail_ && !errs_.empty()) {
                for (size_t i = 0; i < errs_.size(); i++) {
                    printf(errs_[i].c_str());
                }
            }
            ut_cons.reset_color_mode();
            printf("--------------------------------------------------\n");
        }
        void add_func(void* ptr, const char* func, bool first = false)
        {
            if (first) {
                funcs_.insert(funcs_.begin(), func_info(ptr, func, level_));
            }
            else {
                funcs_.push_back(func_info(ptr, func, level_));
            }
        }
        void set_level(const char* v)
        {
            level_ = v;
        }
        void allow_run_level(const char* v)
        {
            if (v) {
                level_filter_ = true;
                map_run_level_[v] = 1;
            }
        }
        void disable_run_level(const char* v)
        {
            if (v) {
                level_filter_ = true;
                map_run_level_[v] = 0;
            }
        }
        void disable_all_level()
        {
            level_check_ = true;
        }
        void init(int argc, char* argv[])
        {
            std::string s;
            for (int i = 1; i < argc; i++) {
                s = argv[i];
                if (s.find("-rx") == 0) {
                    level_check_ = true;
                }
                else if (s.find("-rd=") == 0) {
                    extract_levels(s.substr(4, s.length()), 0);
                }
                else if (s.find("-ra=") == 0) {
                    extract_levels(s.substr(4, s.length()), 1);
                }
                else if (s.find("-p") == 0) {
                    pause_on_exit_ = true;
                }
                else if (s.find("-e") == 0) {
                    exit_on_failed_ = true;
                }
            }
            if (!map_run_level_.empty()) {
                level_filter_ = true;
            }
        }
        const char* get_file_name(const char* fname)
        {
            const char* rs = strrchr(fname, '\\');
            if (rs == NULL) {
                rs = strrchr(fname, '/');
            }
            return rs ? rs + 1 : fname;
        }
        static unit_test& instance()
        {
            static unit_test obj_;
            return obj_;
        }
    private:
        unit_test()
        {
            run_ = 0;
            count_ = 0;
            pass_ = 0;
            exit_on_failed_ = false;
            pause_on_exit_ = false;
            report_detail_ = true;
            level_filter_ = false;
            level_check_ = false;
            level_ = "__root__";
        }
        struct func_info {
            void* ptr;
            std::string func;
            std::string level;
            func_info(void* p, std::string c, std::string l)
                : ptr(p), func(c), level(l)
            {}
        };
        void extract_levels(const std::string& str, int state)
        {
            std::string v;
            for (size_t i = 0; i < str.length(); i++) {
                if (str.at(i) == ',') {
                    if (v.size() > 0) {
                        map_run_level_[v] = state;
                    }
                    v = "";
                }
                else {
                    if (str.at(i) == 0) return;
                    v += str.at(i);
                }
            }
            if (v.size() > 0) {
                map_run_level_[v] = state;
            }
        }
    private:
        int run_;
        int count_;
        int pass_;
        bool exit_on_failed_;
        bool pause_on_exit_;
        bool report_detail_;
        bool level_filter_;
        bool level_check_;
        std::string level_;
        std::vector<func_info> funcs_;
        std::vector<std::string> errs_;
        std::map<std::string, int> map_run_level_;
        std::map<std::string, int>::iterator map_it_;
    };
    static unit_test& ut_test = unit_test::instance();

    class ut_func_holder
    {
    public:
        ut_func_holder(void* ptr, const char* func, bool first = false)
        {
            ut_test.add_func(ptr, func, first);
        }
    };

    class ut_level_holder
    {
    public:
        ut_level_holder(const char* level)
        {
            ut_test.set_level(level);
        }
    };

#ifdef _MSC_VER
    typedef __int8            ut_i8;
    typedef __int16           ut_i16;
    typedef __int32           ut_i32;
    typedef __int64           ut_i64;
    typedef unsigned __int8   ut_u8;
    typedef unsigned __int16  ut_u16;
    typedef unsigned __int32  ut_u32;
    typedef unsigned __int64  ut_u64;
#else
    typedef signed char    ut_i8;
    typedef short          ut_i16;
    typedef int            ut_i32;
    typedef long           ut_i64;
    typedef unsigned char  ut_u8;
    typedef unsigned short ut_u16;
    typedef unsigned int   ut_u32;
    typedef unsigned long  ut_u64;
#endif

    class ut_var
    {
    public:
        ut_var(const bool v) { init(); t_ = UTD_BOOL; v_.b = v; }
        ut_var(const ut_i8 v) { init(); t_ = UTD_CHAR; v_.c = v; }
        ut_var(const ut_i16 v) { init(); t_ = UTD_SHORT; v_.s = v; }
        ut_var(const ut_i32 v) { init(); t_ = UTD_INT; v_.i = v; }
        ut_var(const ut_i64 v) { init(); t_ = UTD_INT64; v_.l = v; }
        ut_var(const ut_u8 v) { init(); t_ = UTD_UCHAR; v_.c = v; }
        ut_var(const ut_u16 v) { init(); t_ = UTD_USHORT; v_.s = v; }
        ut_var(const ut_u32 v) { init(); t_ = UTD_UINT; v_.i = v; }
        ut_var(const ut_u64 v) { init(); t_ = UTD_UINT64; v_.l = v; }
        ut_var(const float v) { init(); t_ = UTD_FLOAT; v_.f = v; }
        ut_var(const double v) { init(); t_ = UTD_DOUBLE; v_.r = v; }
        ut_var(const char* v) { init(); t_ = UTD_STR; s_ = std::string(v); }
        ut_var(const wchar_t* v) { init(); t_ = UTD_WSTR; ws_ = std::wstring(v); }
        ut_var(const std::string& v) { init(); t_ = UTD_STR; s_ = v; }
        ut_var(const std::wstring& v) { init();t_ = UTD_WSTR;  ws_ = v; }
        ut_var() { init(); }
        ut_var(const ut_var& other)
        {
            this->t_ = other.t_;
            this->s_ = other.s_;
            this->ws_ = other.ws_;
            memcpy(&this->v_, &other.v_, 8);
        }
        ut_var& operator=(const ut_var& other)
        {
            this->t_ = other.t_;
            this->s_ = other.s_;
            this->ws_ = other.ws_;
            memcpy(&this->v_, &other.v_, 8);
            return (*this);
        }
        operator const char*() const { return s_.c_str(); }
        operator const wchar_t*() const { return ws_.c_str(); }
        operator const std::string() const { return s_; }
        operator const std::wstring() const { return ws_; }
        operator const double() const { return v_.r; }
        operator const float() const { return v_.f; }
        operator const ut_i64() const { return v_.l; }
        operator const ut_i32() const { return v_.i; }
        operator const ut_i16() const { return v_.s; }
        operator const ut_i8() const { return v_.c; }
        operator const ut_u64() const { return v_.l; }
        operator const ut_u32() const { return v_.i; }
        operator const ut_u16() const { return v_.s; }
        operator const ut_u8() const { return v_.c; }
        operator const bool() const { return v_.b != 0; }
        operator std::string&() { return s_; }
        operator std::wstring&() { return ws_; }
        operator double&() { return v_.r; }
        operator float&() { return v_.f; }
        operator ut_i64&() { return v_.l; }
        operator ut_i32&() { return v_.i; }
        operator ut_i16&() { return v_.s; }
        operator ut_i8&() { return v_.c; }
        operator ut_u64&() { return *(ut_u64*)&v_.l; }
        operator ut_u32&() { return *(ut_u32*)&v_.i; }
        operator ut_u16&() { return *(ut_u16*)&v_.s; }
        operator ut_u8&() { return *(ut_u8*)&v_.c; }
        operator bool&() { return v_.b; }
        bool operator== (const ut_var& rh) const
        {
            return is_equal(rh);
        }
        bool operator== (const bool v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const ut_i8 v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const ut_i16 v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const ut_i32 v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const ut_i64 v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const ut_u8 v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const ut_u16 v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const ut_u32 v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const ut_u64 v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const float v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const double v) const
        {
            return is_equal(ut_var(v));
        }
        bool operator== (const std::wstring& v) const
        {
            return ws_ == v;
        }
        bool operator== (const std::string& v) const
        {
            return s_ == v;
        }
        bool operator== (const wchar_t* v) const
        {
            return ws_ == v;
        }
        bool operator== (const char* v) const
        {
            return s_ == v;
        }
        bool operator!= (const ut_var& rh) const
        {
            return !is_equal(rh);
        }
        bool operator!= (const bool v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const ut_i8 v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const ut_i16 v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const ut_i32 v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const ut_i64 v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const ut_u8 v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const ut_u16 v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const ut_u32 v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const ut_u64 v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const float v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const double v) const
        {
            return !is_equal(ut_var(v));
        }
        bool operator!= (const std::wstring& v) const
        {
            return ws_ != v;
        }
        bool operator!= (const std::string& v) const
        {
            return s_ != v;
        }
        bool operator!= (const wchar_t* v) const
        {
            return ws_ != v;
        }
        bool operator!= (const char* v) const
        {
            return s_ != v;
        }
        std::string to_str()
        {
            char buf[64];
            buf[0] = 0;
            switch (t_)
            {
            case UTD_BOOL: snprintf(buf, 64, v_.b ? "true" : "false"); break;
            case UTD_UCHAR: snprintf(buf, 64, "%u", v_.c); break;
            case UTD_CHAR: snprintf(buf, 64, "%c", v_.c); break;
            case UTD_USHORT: snprintf(buf, 64, "%u", v_.s); break;
            case UTD_SHORT: snprintf(buf, 64, "%d", v_.s); break;
            case UTD_UINT: snprintf(buf, 64, "%u", v_.i); break;
            case UTD_INT: snprintf(buf, 64, "%d", v_.i); break;
#ifdef _MSC_VER
            case UTD_UINT64: snprintf(buf, 64, "%llu", v_.l); break;
            case UTD_INT64: snprintf(buf, 64, "%lld", v_.l); break;
#else
            case UTD_UINT64: snprintf(buf, 64, "%lu", v_.l); break;
            case UTD_INT64: snprintf(buf, 64, "%li", v_.l); break;
#endif
            case UTD_FLOAT: snprintf(buf, 64, "%f", v_.f); break;
            case UTD_DOUBLE: snprintf(buf, 64, "%f", v_.r); break;
            case UTD_STR: return s_.c_str();
                //case UTD_WSTR: return ws_.c_str();
            default:
                break;
            }
            return std::string(buf);
        }
    private:
        inline void init()
        {
            t_ = UTD_BOOL;
            memset(&v_, 0, sizeof(number));
        }
        bool is_equal(const ut_var& rh) const
        {
            if (t_ == UTD_BOOL)
            {
                if (rh.t_ == UTD_BOOL) {
                    return v_.b == rh.v_.b;
                }
                else if (rh.t_ == UTD_FLOAT || rh.t_ == UTD_DOUBLE) {
                    return v_.b == (abs(rh.to_double()) > 0.00000000001);
                }
                else if (rh.t_ >= UTD_NUM_START && rh.t_ <= UTD_NUM_END) {
                    return v_.b == (rh.to_u64() > 0);
                }
                return false;
            }
            else if (t_ == UTD_FLOAT || t_ == UTD_DOUBLE)
            {
                if (rh.t_ == UTD_BOOL) {
                    return rh.v_.b == (abs(to_double()) > 0.00000000001);
                }
                else if (rh.t_ == UTD_FLOAT || rh.t_ == UTD_DOUBLE) {
                    return abs(to_double() - rh.to_double()) < 0.00000000001;
                }
                else if (rh.t_ >= UTD_NUM_START && rh.t_ <= UTD_NUM_END) {
                    return abs(to_double() - rh.to_u64()) < 0.00000000001;
                }
                return false;
            }
            else if (t_ >= UTD_NUM_START && t_ <= UTD_NUM_END)
            {
                if (rh.t_ == UTD_BOOL) {
                    return rh.v_.b == (to_u64() > 0);
                }
                else if (rh.t_ == UTD_FLOAT || rh.t_ == UTD_DOUBLE) {
                    return abs(to_u64() - rh.to_double()) < 0.00000000001;
                }
                else if (rh.t_ >= UTD_NUM_START && rh.t_ <= UTD_NUM_END) {
                    return to_u64() == rh.to_u64();
                }
                return false;
            }
            else if (t_ == UTD_STR)
            {
                if (rh.t_ == UTD_STR) return s_ == rh.s_;
                return false;
            }
            else if (t_ == UTD_WSTR)
            {
                if (rh.t_ == UTD_WSTR) return ws_ == rh.ws_;
                return false;
            }
            return false;
        }
        double to_double() const
        {
            switch (t_)
            {
            case UTD_FLOAT: return v_.f;
            case UTD_DOUBLE: return v_.r;
            default:
                break;
            }
            return 0.F;
        }
        ut_u64 to_u64() const
        {
            switch (t_)
            {
            case UTD_CHAR:
            case UTD_UCHAR: return v_.c;
            case UTD_SHORT:
            case UTD_USHORT: return v_.s;
            case UTD_INT:
            case UTD_UINT: return v_.i;
            case UTD_INT64:
            case UTD_UINT64: return v_.l;
            default:
                break;
            }
            return 0;
        }
    private:
        enum UT_DATA_TYPE
        {
            UTD_BOOL = 1,
            UTD_CHAR, UTD_SHORT, UTD_INT, UTD_INT64,
            UTD_UCHAR, UTD_USHORT, UTD_UINT, UTD_UINT64,
            UTD_FLOAT, UTD_DOUBLE, UTD_STR, UTD_WSTR,
            UTD_NUM_START = UTD_CHAR,
            UTD_NUM_END = UTD_UINT64
        };
        union number
        {
            bool b;
            ut_i8 c;
            ut_i16 s;
            ut_i32 i;
            ut_i64 l;
            float f;
            double r;
        } v_;
        UT_DATA_TYPE t_;
        std::string s_;
        std::wstring ws_;
    };
    typedef std::vector<std::vector<ut_var> > ut_vars;

    class kv_cache
    {
    public:
        void set(const std::string& key, ut_var v)
        {
            kv_[key] = v;
        }
        ut_var get(const std::string& key)
        {
            if (kv_.find(key) != kv_.end()) {
                return kv_[key];
            }
            else {
                return ut_var();
            }
        }
        static kv_cache& instance()
        {
            static kv_cache obj_;
            return obj_;
        }
    private:
        UT_HASH_MAP<std::string, ut_var> kv_;
    };
    static kv_cache& ut_kv = kv_cache::instance();

#define TIPC(x,...)\
{\
    ut_cons.set_color_mode_tip();\
    printf(x,##__VA_ARGS__);\
    ut_cons.reset_color_mode();\
}
#define TIP(x,...)\
{\
    ut_cons.set_color_mode_tip();\
    printf(x,##__VA_ARGS__);\
    printf("\n");\
    ut_cons.reset_color_mode();\
}
#define EXPECT(x)\
{\
    ut_test.check_eq((x), __FUNCTION__, __LINE__, __FILE__, 0);\
}
#define VEXPECT(t,x)\
{\
    TIP(t);\
    ut_test.check_eq((x), __FUNCTION__, __LINE__, __FILE__, 0);\
}
#define EXPECT_EQ(a,b)\
{\
    ut_var v1 = (a), v2 = (b);\
    bool eq = (v1 == v2);\
    if (eq == false) {\
        TIP("Expect: %s, Return Value: %s",\
        v1.to_str().c_str(), v2.to_str().c_str());\
    }\
    ut_test.check_eq(eq, __FUNCTION__, __LINE__, __FILE__, 0);\
}
#define VEXPECT_EQ(t,a,b)\
{\
    TIP(t);\
    EXPECT_EQ(a,b);\
}

#define BAT_CHECK_EQ(i,a,b)\
{\
    ut_var v1 = (a), v2 = (b);\
    bool eq = (v1 == v2);\
    if (eq == false) {\
        TIP("Expect: %s, Return Value: %s",\
        v1.to_str().c_str(), v2.to_str().c_str());\
    }\
    ut_test.check_eq(eq, __FUNCTION__, __LINE__, __FILE__, i);\
}
#define BAT_CHECK_1(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, true,x(v[i][0]));\
}
#define BAT_CHECK_2(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, true,x(v[i][0], v[i][1]));\
}
#define BAT_CHECK_3(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, true,x(v[i][0], v[i][1], v[i][2]));\
}
#define BAT_CHECK_4(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, true,x(v[i][0], v[i][1], v[i][2], v[i][3]));\
}
#define BAT_CHECK_5(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, true,x(v[i][0], v[i][1], v[i][2], v[i][3], v[i][4]));\
}
#define BAT_CHECK_6(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, true,x(v[i][0], v[i][1], v[i][2], v[i][3], v[i][4]\
            , v[i][5]));\
}
#define BAT_CHECK_7(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, true,x(v[i][0], v[i][1], v[i][2], v[i][3], v[i][4]\
            , v[i][5], v[i][6]));\
}
#define BAT_CHECK_8(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, true,x(v[i][0], v[i][1], v[i][2], v[i][3], v[i][4]\
            , v[i][5], v[i][6], v[i][7]));\
}
#define BAT_CHECK_9(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, true,x(v[i][0], v[i][1], v[i][2], v[i][3], v[i][4]\
            , v[i][5], v[i][6], v[i][7], v[i][8]));\
}
#define VBAT_CHECK_1(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, v[i][0],x(v[i][1]));\
}
#define VBAT_CHECK_2(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, v[i][0],x(v[i][1], v[i][2]));\
}
#define VBAT_CHECK_3(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, v[i][0],x(v[i][1], v[i][2], v[i][3]));\
}
#define VBAT_CHECK_4(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, v[i][0],x(v[i][1], v[i][2], v[i][3], v[i][4]));\
}
#define VBAT_CHECK_5(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, v[i][0],x(v[i][1], v[i][2], v[i][3], v[i][4]\
        , v[i][5]));\
}
#define VBAT_CHECK_6(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, v[i][0],x(v[i][1], v[i][2], v[i][3], v[i][4]\
            , v[i][5], v[i][6]));\
}
#define VBAT_CHECK_7(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, v[i][0],x(v[i][1], v[i][2], v[i][3], v[i][4]\
            , v[i][5], v[i][6], v[i][7]));\
}
#define VBAT_CHECK_8(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, v[i][0],x(v[i][1], v[i][2], v[i][3], v[i][4]\
            , v[i][5], v[i][6], v[i][7], v[i][8]));\
}
#define VBAT_CHECK_9(x,v)\
{\
    for (size_t i = 0; i < v.size(); i++)\
        BAT_CHECK_EQ(i, v[i][0],x(v[i][1], v[i][2], v[i][3], v[i][4]\
            , v[i][5], v[i][6], v[i][7], v[i][8], v[i][9]));\
}
#define VTEST(x)\
    void x();\
    ut_func_holder __ufo_##x((void *)x, #x);\
    void x()
#define VTEST_ADD(x) ut_test.add_func((void *)x, #x, false);
#define VTEST_TOP_ADD(x) ut_test.add_func((void *)x, #x, true);
#define VTEST_RUN_ALL() ut_test.run_all();
#define VTEST_REGION_PUSH(x) ut_level_holder __ulo_##x(#x);
#define VTEST_REGION_POP(x) ut_level_holder __ulc_##x("__root__");
#define VTEST_DISABLE_ALL_REGION() ut_test.disable_all_level();
#define VTEST_DISABLE_REGION(x) ut_test.disable_run_level(x);
#define VTEST_ALLOW_REGION(x) ut_test.allow_run_level(x);
#define VTEST_INIT(argc, argv) ut_test.init(argc, argv);
#ifndef VASSERT
#define VASSERT(x) assert(x);
#endif

} // namespace

#endif // __V_TEST_H__

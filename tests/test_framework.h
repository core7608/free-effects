#pragma once

#include <string>
#include <vector>
#include <functional>
#include <iostream>

namespace FreeEffect {
namespace Test {

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

class TestRunner {
public:
    static TestRunner& instance() {
        static TestRunner runner;
        return runner;
    }
    
    void addTest(const std::string& name, std::function<void()> testFunc) {
        m_tests.push_back({name, testFunc});
    }
    
    int runAll() {
        int passed = 0;
        int failed = 0;
        
        std::cout << "Running " << m_tests.size() << " tests...\n\n";
        
        for (const auto& test : m_tests) {
            try {
                test.func();
                std::cout << "  PASS: " << test.name << "\n";
                passed++;
            } catch (const std::exception& e) {
                std::cout << "  FAIL: " << test.name << " - " << e.what() << "\n";
                failed++;
            } catch (...) {
                std::cout << "  FAIL: " << test.name << " - Unknown exception\n";
                failed++;
            }
        }
        
        std::cout << "\n" << passed << " passed, " << failed << " failed out of " << m_tests.size() << "\n";
        return failed > 0 ? 1 : 0;
    }

private:
    struct Test {
        std::string name;
        std::function<void()> func;
    };
    std::vector<Test> m_tests;
};

class TestRegistrar {
public:
    TestRegistrar(const std::string& name, std::function<void()> func) {
        TestRunner::instance().addTest(name, func);
    }
};

} // namespace Test
} // namespace FreeEffect

#define TEST(name) \
    static void test_##name(); \
    static FreeEffect::Test::TestRegistrar registrar_##name(#name, test_##name); \
    static void test_##name()

#define ASSERT_TRUE(expr) \
    do { if (!(expr)) throw std::runtime_error("ASSERT_TRUE failed: " #expr); } while(0)

#define ASSERT_FALSE(expr) \
    do { if ((expr)) throw std::runtime_error("ASSERT_FALSE failed: " #expr); } while(0)

#define ASSERT_EQ(a, b) \
    do { if ((a) != (b)) throw std::runtime_error("ASSERT_EQ failed: " #a " != " #b); } while(0)

#define ASSERT_NE(a, b) \
    do { if ((a) == (b)) throw std::runtime_error("ASSERT_NE failed: " #a " == " #b); } while(0)

#define ASSERT_THROW(expr, exception_type) \
    do { \
        bool caught = false; \
        try { expr; } catch (const exception_type&) { caught = true; } \
        if (!caught) throw std::runtime_error("ASSERT_THROW failed: expected " #exception_type); \
    } while(0)

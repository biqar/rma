/*
 * factory.hpp
 *
 *  Created on: 17 Jan 2018
 *      Author: Dean De Leo
 */

#ifndef DISTRIBUTION_FACTORY_HPP_
#define DISTRIBUTION_FACTORY_HPP_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "errorhandling.hpp"

#define ADD_DISTRIBUTION(name, description, callable) distribution::factory().add(name, description, callable, __FILE__, __LINE__)

namespace distribution {

// forward declarations
class Distribution;
class Factory;

Factory& factory();

class Factory {
    friend Factory& factory();
    static Factory singleton;
    Factory() = default;
    Factory(const Factory&) = delete;
    Factory& operator=(const Factory&) = delete;

    class InvokerBase {
    protected:
        std::string m_name;
        std::string m_description;
        const char* m_source;
        int m_line;

        InvokerBase(const std::string& name, const std::string& description, const char* source, int line);

    public:
        virtual ~InvokerBase();

        const std::string& name() const;
        const std::string& description() const;
        const char* source() const;
        int line() const;

        virtual std::unique_ptr<Distribution> make() = 0;
    };

    template <typename Callable>
    class Invoker : public InvokerBase {
    protected:
        Callable m_callable;

    public:
        Invoker(const std::string& name, const std::string& description, Callable callable, const char* source, int line) :
            InvokerBase(name, description, source, line), m_callable(callable) { }

        std::unique_ptr<Distribution> make() override {
            return std::invoke(m_callable);
        }
    };

    std::vector<std::unique_ptr<InvokerBase>> m_distributions;


public:
    template<typename Callable>
    void add(const std::string& name, const std::string& description, Callable callable, const char* source, int line) {
        auto it = std::find_if(begin(m_distributions), end(m_distributions), [name](const std::unique_ptr<InvokerBase>& impl){
            return impl->name() == name;
        });
        if(it != end(m_distributions)){
            auto& r = *it;
            RAISE_EXCEPTION(Exception, "The experiment '" << name << "' has already been registered from: " << r->source() << ":" << r->line() << ". Attempting to register it again from: " << source << ":" << line);
        }

        m_distributions.push_back(std::unique_ptr<InvokerBase>{ new Invoker<Callable>(name, description, callable, source, line)});
    }

    /**
     * Get the list of registered distributions
     */
    const auto& list(){ return m_distributions; }

    void sort_list();

    std::unique_ptr<Distribution> make(const std::string& name);
};


} // namespace distribution

#endif /* DISTRIBUTION_FACTORY_HPP_ */

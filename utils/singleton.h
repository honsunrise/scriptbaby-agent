//
// Created by zhsyourai on 1/16/17.
//

#ifndef PONGO_AGENT_SINGLETON_H
#define PONGO_AGENT_SINGLETON_H


#include <cassert>
#include <boost/thread/mutex.hpp>

template<class T>
class singleton {
public:
    static T &get_instance() {
        assert(!is_destructed);
        (void) is_destructed; // prevent removing is_destructed in Release configuration
        static T instance;

        boost::mutex::scoped_lock lock(get_mutex());
        return instance;
    }

    singleton() {}

    virtual ~singleton() { is_destructed = true; }
private:
    static bool is_destructed;

    static boost::mutex &get_mutex() {
        static boost::mutex mutex;
        return mutex;
    }
};

// force creating mutex before main() is called
template<class T>
bool singleton<T>::is_destructed = (singleton<T>::get_mutex(), false);

#endif //PONGO_AGENT_SINGLETON_H

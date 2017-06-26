/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

#include <pv/debugPtr.h>

namespace epics {
namespace debug {

struct tracker {
    std::mutex mutex;
    ptr_base::ref_set_t refs;
};

void shared_ptr_base::track_new()
{
    if(track) {
        std::lock_guard<std::mutex> G(track->mutex);
        track->refs.insert(this);
    }
    snap_stack();
}

// create new tracker if ptr!=nullptr, otherwise clear
void shared_ptr_base::track_new(void* ptr)
{
    track_clear();
    if(ptr){
        track.reset(new tracker);
        std::lock_guard<std::mutex> G(track->mutex);
        track->refs.insert(this);
    }
    snap_stack();
}

void shared_ptr_base::track_assign(const shared_ptr_base &o)
{
    if(track!=o.track) {
        track_clear();
        track = o.track;
        if(track) {
            std::lock_guard<std::mutex> G(track->mutex);
            track->refs.insert(this);
        }
        snap_stack();
    }
}

void shared_ptr_base::track_clear()
{
    if(track) {
        std::lock_guard<std::mutex> G(track->mutex);
        track->refs.erase(this);
    }
    track.reset();
    m_depth = 0;
}

void shared_ptr_base::swap(shared_ptr_base &o)
{
    // we cheat a bit here to avoid lock order, and to lock only twice
    if(track) {
        std::lock_guard<std::mutex> G(track->mutex);
        track->refs.insert(&o);
        track->refs.erase(this);
    }
    track.swap(o.track);
    if(track) {
        std::lock_guard<std::mutex> G(track->mutex);
        track->refs.insert(this);
        track->refs.erase(&o);
    }
    //TODO: keep original somehow???
    snap_stack();
    o.snap_stack();
}

void shared_ptr_base::snap_stack()
{
    if(!track) {
        m_depth = 0;
        return;
    }
#if defined(EXCEPT_USE_BACKTRACE)
    {
        m_depth=backtrace(m_stack,EXCEPT_DEPTH);
    }
#else
    {}
#endif

}

void shared_ptr_base::show_stack(std::ostream& strm) const
{
    strm<<"ptr "<<this;
    if(m_depth<=0) return;
#if 0 && defined(EXCEPT_USE_BACKTRACE)
    {

        char **symbols=backtrace_symbols(m_stack, m_depth);

        strm<<": ";
        for(int i=0; i<m_depth; i++) {
            strm<<symbols[i]<<", ";
        }

        std::free(symbols);
    }
#else
    {
        strm<<": ";
        for(int i=0; i<m_depth; i++) {
            strm<<std::hex<<m_stack[i]<<" ";
        }
    }
#endif

}

void ptr_base::show_refs(std::ostream& strm, bool self, bool weak) const
{
    if(!track) {
        strm<<"# No refs\n";
    } else {
        std::lock_guard<std::mutex> G(track->mutex);
        for(auto ref : track->refs) {
            if(!self && ref==this) continue;
            strm<<'#';
            ref->show_stack(strm);
            strm<<'\n';
        }
    }
}

void ptr_base::spy_refs(ref_set_t &refs) const
{
    if(track) {
        std::lock_guard<std::mutex> G(track->mutex);
        refs.insert(track->refs.begin(), track->refs.end());
    }
}

}} // namespace epics::debug
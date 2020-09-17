#include "transaction_manager.hpp"

using namespace tx;
using namespace std;
using namespace std::chrono_literals;

//- Forward declarations and common type aliases common to the test functions below().
//
struct test_item;

using item_list  = std::vector<test_item>;
using index_list = std::vector<size_t>;

using entropy  = random_device;
using prn_gen  = mt19937_64;
using int_dist = uniform_int_distribution<>;
using hasher   = hash<string_view>;



//-------------------------------------------------------------------------------------------------
//  Class:      barrier
//
//  Purpose:    This is just a simple counting barrier to avoid races during thread startup
//              for mode 1.
//-------------------------------------------------------------------------------------------------
//
class barrier
{
  public:
    barrier(uint32_t rel_cnt);
    barrier(barrier const&) = delete;
    void    operator =(barrier const&) = delete;

    ~barrier();

    void    wait();
    void    release();

  private:
    using mtx_lock  = std::unique_lock<std::mutex>;
    using mutex     = std::mutex;
    using cond_var  = std::condition_variable;

	uint32_t	m_threshold;
	uint32_t	m_wait_count;
	uint32_t	m_global_cycle;
    mutex       m_mutex;
    cond_var    m_cond;
};

barrier::barrier(uint32_t rel_cnt)
:   m_threshold(rel_cnt)
,   m_wait_count(0)
,   m_global_cycle(0)
,   m_mutex()
,   m_cond()
{}

barrier::~barrier()
{}

void
barrier::wait()
{
    uint32_t    local_cycle;
    mtx_lock    lock(m_mutex);

    local_cycle = m_global_cycle;

    if (++m_wait_count == m_threshold)
    {
        ++m_global_cycle;
        m_wait_count = 0;
        m_cond.notify_all();
    }
    else
    {
        while (local_cycle == m_global_cycle)
        {
            m_cond.wait(lock);
        }
    }
}

void
barrier::release()
{
    mtx_lock    lock(m_mutex);

    ++m_global_cycle;
    m_wait_count = 0;
    m_cond.notify_all();
}


//--------------------------------------------------------------------------------------------------
//  class:      test_item
//
//  Purpose:    This class is publicly derived from lockable_item, and holds data subject to
//              concurrent updates.
//--------------------------------------------------------------------------------------------------
//
struct test_item : public lockable_item
{
    static constexpr size_t     buf_size = 32;

    char    ma_chars[buf_size];

    void    st_update(FILE* fp, prn_gen& gen, int_dist& char_dist);
    void    tx_update(transaction_manager const& tx, FILE* fp, prn_gen& gen, int_dist& char_dist);
};


//- This member function actually updates the on-board data for the non-STO tests.  After doing
//  so, it checks to see if a race has occurred.
//
void
test_item::st_update(FILE* fp, prn_gen& gen, int_dist& char_dist)
{
    char            local_chars[buf_size];
    string_view     local_view(local_chars, buf_size);
    string_view     shared_view(ma_chars, buf_size);
    hasher          hash;

    for (size_t i = 0;  i < buf_size;  ++i)
    {
        local_chars[i] = ma_chars[i] = (char) char_dist(gen);
    }

    if (hash(shared_view) != hash(local_view))
    {
        fprintf(fp, "RACE FOUND!, TX 0  item %zd\n", this->id());
    }
}


//- This member function actually updates the on-board data for the STO tests.  After doing
//  so, it checks to see if a race has occurred.  It is exactly the same as st_update(),
//  except that it takes the transaction as an argument so that its ID can be printed if
//  a race is detected.
//
void
test_item::tx_update(transaction_manager const& tx, FILE* fp, prn_gen& gen, int_dist& char_dist)
{
    char            local_chars[buf_size];
    string_view     local_view(local_chars, buf_size);
    string_view     shared_view(ma_chars, buf_size);
    hasher          hash;

    for (size_t i = 0;  i < buf_size;  ++i)
    {
        local_chars[i] = ma_chars[i] = (char) char_dist(gen);
    }

    if (hash(shared_view) != hash(local_view))
    {
        fprintf(fp, "RACE FOUND!, TX %zd  item %zd\n", tx.id(), this->id());
    }
}


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//  Function:   st_access_test()
//
//  Purpose:    This function performs updates directly with no concurrency control.  It used for
//              both the single-threaded baseline test (mode 0) as well as the multi-threaded,
//              race-producing test (mode 3).
//--------------------------------------------------------------------------------------------------
//
void
st_access_test(item_list& items, FILE* fp, size_t tx_count, size_t refs_count)
{
    entropy     rd;
    prn_gen     gen(rd());
    int_dist    refs_index_dist(0, (int)(items.size()-1));
    int_dist    refs_count_dist(1, (int) refs_count);
    int_dist    char_dist(0, 127);

    stopwatch   sw;
    index_list  indices;
    size_t      index;

    sw.start();

    for (size_t i = 0;  i < tx_count;  ++i)
    {
        indices.clear();
        refs_count = refs_count_dist(gen);

        for (size_t j = 0;  j < refs_count;  ++j)
        {
            index = refs_index_dist(gen);
            indices.push_back(index);
        }

        for (size_t j = 0;  j < refs_count;  ++j)
        {
            index = indices[j];
            items[index].st_update(fp, gen, char_dist);
        }
    }

    sw.stop();
    fprintf(fp, "TX 0 took %d msec\n", sw.milliseconds_elapsed<int>());
}


//--------------------------------------------------------------------------------------------------
//  Function:   mx_access_test()
//
//  Purpose:    This function employs a single mutex to create a critical section where updates
//              are performed (mode 2).
//--------------------------------------------------------------------------------------------------
//
void
mx_access_test(item_list& items, FILE* fp, size_t tx_count, size_t refs_count)
{
    static mutex    mtx;

    entropy     rd;
    prn_gen     gen(rd());
    int_dist    refs_index_dist(0, (int)(items.size()-1));
    int_dist    refs_count_dist(1, (int) refs_count);
    int_dist    char_dist(0, 127);

    stopwatch   sw;
    index_list  indices;
    size_t      index;

    sw.start();

    for (size_t i = 0;  i < tx_count;  ++i)
    {
        indices.clear();
        refs_count = refs_count_dist(gen);

        for (size_t j = 0;  j < refs_count;  ++j)
        {
            index = refs_index_dist(gen);
            indices.push_back(index);
        }

        mtx.lock();

        for (size_t j = 0;  j < refs_count;  ++j)
        {
            index = indices[j];
            items[index].st_update(fp, gen, char_dist);
        }

        mtx.unlock();

        if (i < 10)
        {
            std::this_thread::yield();
        }
    }

    sw.stop();
    fprintf(fp, "TX 0 took %d msec\n", sw.milliseconds_elapsed<int>());
}


//--------------------------------------------------------------------------------------------------
//  Function:   tx_access_test()
//
//  Purpose:    This function exercise the STO algorithm (mode 1).
//--------------------------------------------------------------------------------------------------
//
void
tx_access_test
(item_list& items, FILE* fp, size_t tx_count, size_t refs_count, int loglvl, barrier& gate)
{
    entropy     rd;
    prn_gen     gen(rd());
    int_dist    refs_index_dist(0, (int)(items.size()-1));
    int_dist    refs_count_dist(1, (int) refs_count);
    int_dist    char_dist(0, 127);

    transaction_manager tx(loglvl, fp);
    stopwatch   sw;
    index_list  indices;
    size_t      index;
    bool        acquired;

    gate.wait();
    sw.start();

    for (size_t i = 0;  i < tx_count;  ++i)
    {
        indices.clear();
        refs_count = refs_count_dist(gen);

        for (size_t j = 0;  j < refs_count;  ++j)
        {
            index = refs_index_dist(gen);
            indices.push_back(index);
        }

        tx.begin();
        acquired = true;

        for (size_t j = 0;  acquired  &&  j < refs_count;  ++j)
        {
            index    = indices[j];
            acquired = (acquired  &&  tx.acquire(items[index]));
        }

        if (acquired)
        {
            for (size_t j = 0;  j < refs_count;  ++j)
            {
                index = indices[j];
                items[index].tx_update(tx, fp, gen, char_dist);
            }
            tx.commit();
        }
        else
        {
            tx.rollback();
        }

        if (i < 10)
        {
            std::this_thread::yield();
        }
    }

    sw.stop();
    fprintf(fp, "TX %zd took %d msec\n", tx.id(), sw.milliseconds_elapsed<int>());
}


//--------------------------------------------------------------------------------------------------
//  Function:   test_tx()
//
//  Purpose:    Primary test driver called from main().
//--------------------------------------------------------------------------------------------------
//
void
test_tx(FILE* fp, size_t item_count, size_t thread_count, size_t tx_count,
        size_t refs_count, size_t mode, int loglvl)
{
    using future_list = std::vector<std::future<void>>;

    stopwatch   sw;
    future_list fv;

    if (mode == 0)
    {
        thread_count = 0;
    }
    else
    {
        thread_count = max((size_t) 1u, thread_count);
    }

    fprintf(fp, "testing with\n");
    fprintf(fp, "  lockable items: %zd\n", item_count);
    fprintf(fp, "  threads       : %zd\n", thread_count);
    fprintf(fp, "  transactions  : %zd\n", tx_count);
    fprintf(fp, "  refs per tx   : %zd\n", refs_count);

    sw.start();
    item_list   items(item_count);
    sw.stop();
    fprintf(fp, "\nrecord init required %d msec\n\n", sw.milliseconds_elapsed<int>());

    sw.start();

    //- Mode 0 is a single-threaded run, in order to gather a baseline performance number.
    //
    if (mode == 0)
    {
        st_access_test(items, fp, tx_count, refs_count);
    }

    //- Mode 1 is a multi-threaded, transaction-based.  This tests the algorithm.
    //
    else if (mode == 1)
    {
        barrier     gate((uint32_t) thread_count + 1);

        for (size_t i = 0;  i < thread_count;  ++i)
        {
            fv.push_back(std::async(std::launch::async,
                                    std::bind(&tx_access_test,
                                              std::ref(items),
                                              fp,
                                              tx_count,
                                              refs_count,
                                              loglvl,
                                              std::ref(gate))));
        }

        gate.wait();

        for (size_t i = 0;  i < thread_count;  ++i)
        {
            fv[i].wait();
        }
    }

    //- Mode 2 is a multi-threaded, but with a single mutex guarding all updates.
    //
    else if (mode == 2)
    {
        for (size_t i = 0;  i < thread_count;  ++i)
        {
            fv.push_back(std::async(std::launch::async,
                                    std::bind(&mx_access_test,
                                              std::ref(items),
                                              fp,
                                              tx_count,
                                              refs_count)));
        }
        for (size_t i = 0;  i < thread_count;  ++i)
        {
            fv[i].wait();
        }
    }

    //- Mode 3 is a multi-threaded, but with no protection.  Its purpose is to demonstrate that
    //  data races can occur without any concurrency control.
    //
    else if (mode == 3)
    {
        for (size_t i = 0;  i < thread_count;  ++i)
        {
            fv.push_back(std::async(std::launch::async,
                                    std::bind(&st_access_test,
                                              std::ref(items),
                                              fp,
                                              tx_count,
                                              refs_count)));
        }
        for (size_t i = 0;  i < thread_count;  ++i)
        {
            fv[i].wait();
        }
    }
    sw.stop();
    fprintf(fp, "\ntransactions required %d msec\n", sw.milliseconds_elapsed<int>());
    sw.start();
    items.clear();
    sw.stop();
    fprintf(fp, "clean-up required %d msec\n", sw.milliseconds_elapsed<int>());
}

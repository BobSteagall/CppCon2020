#ifndef TRANSACTION_HPP_DEFINED
#define TRANSACTION_HPP_DEFINED

#include "lockable_item.hpp"

namespace tx {

class transaction_manager
{
  public:
    ~transaction_manager();
    transaction_manager(int log_level, FILE* fp=nullptr);

    tx_id_type  id() const noexcept;
    tsv_type    tsv() const noexcept;

    void    begin();
    void    commit();
    void    rollback();

    bool    acquire(lockable_item& item) noexcept;

  private:
    using item_ptr_list = std::vector<lockable_item*>;
    using mutex         = std::mutex;
    using tx_lock       = std::unique_lock<std::mutex>;
    using cond_var      = std::condition_variable;
    using atomic_tsv    = std::atomic<tsv_type>;
    using atomic_tx_id  = std::atomic<tx_id_type>;

    tx_id_type      m_tx_id;
    tsv_type        m_tx_tsv;
    item_ptr_list   m_item_ptrs;
    mutex           m_mutex;
    cond_var        m_cond;
    FILE*           m_fp;
    int             m_log_level;

    static  atomic_tsv      sm_tsv_generator;
    static  atomic_tx_id    sm_tx_id_generator;

  private:
    void    log_begin() const;
    void    log_commit() const;
    void    log_rollback() const;

    void    log_acquisition_success(lockable_item const& item) const;
    void    log_acquisition_failure(lockable_item const& item) const;
    void    log_acquisition_same(lockable_item const& item) const;
    void    log_acquisition_waiting(lockable_item const& item, transaction_manager* p_curr_tx) const;
};

inline tx_id_type
transaction_manager::id() const noexcept
{
    return m_tx_id;
}

inline tsv_type
transaction_manager::tsv() const noexcept
{
    return m_tx_tsv;
}

}       //- tx namespace
#endif  //- TRANSACTION_HPP_DEFINED

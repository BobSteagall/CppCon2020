#ifndef LOCKABLE_ITEM_HPP_DEFINED
#define LOCKABLE_ITEM_HPP_DEFINED

#include "forward.hpp"

namespace tx {

class lockable_item
{
  public:
    lockable_item();

    item_id_type    id() const noexcept;
    tsv_type        last_tsv() const noexcept;

  private:
    friend class transaction_manager;

    using txm_pointer    = std::atomic<transaction_manager*>;
    using atomic_item_id = std::atomic<item_id_type>;

    txm_pointer     mp_owning_txm;  //- Pointer to transaction manager that owns this object
    tsv_type        m_last_tsv;     //- Timestamp of last owner
    item_id_type    m_item_id;      //- Data member for debugging/tracking

    static  atomic_item_id      sm_item_id_generator;
};

inline
lockable_item::lockable_item()
:   mp_owning_txm(nullptr)
,   m_last_tsv(0)
,   m_item_id(++sm_item_id_generator)
{}

inline item_id_type
lockable_item::id() const noexcept
{
    return m_item_id;
}

inline tsv_type
lockable_item::last_tsv() const noexcept
{
    return m_last_tsv;
}

}       //- tx namespace
#endif  //- LOCKABLE_ITEM_HPP_DEFINED

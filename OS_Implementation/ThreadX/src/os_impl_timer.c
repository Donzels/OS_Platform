#include "osal_internal_timer.h"
#include "osal_internal_globaldefs.h"
#include "os_threadx.h"
#include "osal_internal_heap.h"

#if (OSAL_RTOS_SUPPORT == THREADX_SUPPORT)

static void os_timer_cb(ULONG arg)
{
    osal_timer_t *timer = (osal_timer_t *)arg;
    if (timer != NULL && timer->func != NULL)
    {
        timer->func(timer->timer_handle, timer->arg);
    }
}

int32_t os_timer_create_impl(osal_timer_handle_t *p_timer_handle, osal_timer_internal_record_t *timer_record)
{
    int32_t ret = OSAL_SUCCESS;
    
    TX_TIMER *cur_timer_handle = (TX_TIMER *)os_heap_malloc_impl(sizeof(TX_TIMER));
    if (cur_timer_handle == NULL)
    {
        return OSAL_INVALID_POINTER;
    }

    timer_record->timer_id.timer_handle = (osal_timer_handle_t)cur_timer_handle;

    UINT auto_activate = timer_record->auto_reload ? TX_AUTO_ACTIVATE : 0;
    UINT status = tx_timer_create(cur_timer_handle, 
                                  (CHAR *)timer_record->timer_name,
                                  os_timer_cb, 
                                  (ULONG)&timer_record->timer_id,
                                  timer_record->timer_period,
                                  timer_record->auto_reload ? timer_record->timer_period : 0,
                                  auto_activate);
    
    if (status != TX_SUCCESS)
    {
        os_heap_free_impl(cur_timer_handle);
        ret = OSAL_INVALID_POINTER;
    }
    else
    {
        *p_timer_handle = (osal_timer_handle_t)cur_timer_handle;
    }
    return ret;
}

int32_t os_timer_start_impl(osal_timer_handle_t timer_handle, osal_tick_type_t ticks_to_wait)
{
    int32_t ret = OSAL_SUCCESS;
    UINT status;

    TX_TIMER *handle = (TX_TIMER *)timer_handle;
    if (handle == NULL)
    {
        return OSAL_ERROR;
    }

    status = tx_timer_activate(handle);
    
    if (status != TX_SUCCESS)
    {
        ret = OSAL_ERROR;
    }
    return ret;
}

int32_t os_timer_stop_impl(osal_timer_handle_t timer_handle, osal_tick_type_t ticks_to_wait)
{
    int32_t ret = OSAL_SUCCESS;
    UINT status;

    TX_TIMER *handle = (TX_TIMER *)timer_handle;
    if (handle == NULL)
    {
        return OSAL_ERROR;
    }

    status = tx_timer_deactivate(handle);
    
    if (status != TX_SUCCESS)
    {
        ret = OSAL_ERROR;
    }
    return ret;
}

int32_t os_timer_period_change_impl(osal_timer_handle_t timer_handle, osal_tick_type_t new_period, osal_tick_type_t ticks_to_wait)
{
    int32_t ret = OSAL_SUCCESS;
    UINT status;

    TX_TIMER *handle = (TX_TIMER *)timer_handle;
    if (handle == NULL)
    {
        return OSAL_ERROR;
    }

    // For periodic timer, both should be new_period; for one-shot, reschedule_ticks = 0
    status = tx_timer_change(handle, new_period, new_period);
    
    if (status != TX_SUCCESS)
    {
        ret = OSAL_ERROR;
    }
    return ret;
}

int32_t os_timer_delete_impl(osal_timer_handle_t timer_handle, osal_tick_type_t ticks_to_wait)
{
    int32_t ret = OSAL_SUCCESS;
    UINT status;

    TX_TIMER *handle = (TX_TIMER *)timer_handle;
    if (handle == NULL)
    {
        return OSAL_ERROR;
    }

    status = tx_timer_delete(handle);
    
    if (status != TX_SUCCESS)
    {
        ret = OSAL_ERROR;
    }
    else
    {
        os_heap_free_impl(handle);
    }
    return ret;
}

int32_t os_timer_reset_impl(osal_timer_handle_t timer_handle, osal_tick_type_t ticks_to_wait)
{
    int32_t ret = OSAL_SUCCESS;
    UINT status;

    TX_TIMER *handle = (TX_TIMER *)timer_handle;
    if (handle == NULL)
    {
        return OSAL_ERROR;
    }

    // Deactivate and reactivate to reset
    tx_timer_deactivate(handle);
    status = tx_timer_activate(handle);
    
    if (status != TX_SUCCESS)
    {
        ret = OSAL_ERROR;
    }
    return ret;
}

osal_tick_type_t os_timer_period_get_impl(osal_timer_handle_t timer_handle)
{
    /*
     * ThreadX does not provide a direct API to get timer period
     * Would need to store period separately if this function is needed
     * For now, return 0 as a placeholder
     */
    return 0;
}

#endif // OSAL_RTOS_SUPPORT

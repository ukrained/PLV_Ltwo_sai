#include "sai.h"
#include "stub_sai.h"
#include "assert.h"

#define STUB_MAX_LAGS 5
#define STUB_MAX_LAG_PORTS 16


/* ==========================================================================================
 *   THE  TYPES  DECLARATIONS
 * ========================================================================================== */

typedef struct {
    sai_object_id_t oid;        // LAG OID / non-zero marks valid LAG entries
    uint32_t        ports_mask; // ports which belongs to LAG / system ports bitmask
    uint32_t        ports_cnt;  // number of ports in LAG / just count LAG members
} stub_lag_table_entry_t;


/* ==========================================================================================
 *   THE  FUNCTIONS  DECLARATIONS
 * ========================================================================================== */

sai_status_t stub_lag_attr_get(_In_ const sai_object_key_t   *key,
                               _Inout_ sai_attribute_value_t *value,
                               _In_ uint32_t                  attr_index,
                               _Inout_ vendor_cache_t        *cache,
                               void                          *arg);

sai_status_t stub_lag_member_attr_get(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg);


/* ==========================================================================================
 *   THE  STATIC  DATA  SETS
 * ========================================================================================== */

/* stub table for primitive LAG management */
stub_lag_table_entry_t lags_table[STUB_MAX_LAGS] = { 0 };


static const sai_attribute_entry_t lag_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST, false, false, false, true, "LAG port list", SAI_ATTR_VAL_TYPE_OID },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false, "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t lag_vendor_attribs[] = {
    {
        SAI_LAG_ATTR_PORT_LIST,                           // .id
        { false, false, false, true },                    // .is_implemented
        { false, false, false, true },                    // .is_supported
        stub_lag_attr_get, (void*)SAI_LAG_ATTR_PORT_LIST, // .getter
        NULL, NULL                                        // .setter
    }
};

static const sai_attribute_entry_t lag_member_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID, true, true, false, true, "LAG member LAG ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_LAG_MEMBER_ATTR_PORT_ID, true, true, false, true, "LAG member PORT ID", SAI_ATTR_VAL_TYPE_OID },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false, "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t lag_member_vendor_attribs[] = {
    {
        SAI_LAG_MEMBER_ATTR_LAG_ID,                                  // .id
        { true, false, false, true },                                // .is_implemented
        { true, false, false, true },                                // .is_supported
        stub_lag_member_attr_get, (void*)SAI_LAG_MEMBER_ATTR_LAG_ID, // .getter
        NULL, NULL                                                   // .setter
    },
    {
        SAI_LAG_MEMBER_ATTR_PORT_ID,                                  // .id
        { true, false, false, true },                                 // .is_implemented
        { true, false, false, true },                                 // .is_supported
        stub_lag_member_attr_get, (void*)SAI_LAG_MEMBER_ATTR_PORT_ID, // .getter
        NULL, NULL                                                    // .setter
    }
};

/* ==========================================================================================
 *   THE  AUXILIARY  FUNCTIONS  AND  CALLBACKS
 * ========================================================================================== */

// used to debug LAGs table content
static void stub_lags_table_dbg_dump()
{
    printf("\n[ LAGs table dump ]\n");
    for (uint32_t i = 0; i < STUB_MAX_LAGS; i++) {
        printf("LAG[%d].oid = 0x%010lx ", i, lags_table[i].oid);
        printf(".port_mask = 0x%016x\n", lags_table[i].ports_mask);
    }
    printf("\n");
}

static void lag_key_to_str(_In_ sai_object_id_t lag_id, _Out_ char *key_str)
{
    uint32_t lag;

    if (SAI_STATUS_SUCCESS != stub_object_to_type(lag_id, SAI_OBJECT_TYPE_LAG, &lag)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid lag");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "lag %x", lag);
    }
}

static void lag_member_key_to_str(_In_ sai_object_id_t lag_member_id, _Out_ char *key_str)
{
    uint32_t port;

    if (SAI_STATUS_SUCCESS != stub_object_to_type(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER, &port)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid lag member");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "port %x", port);
    }
}

/* Get the LAG port list [sai_object_list_t] */
sai_status_t stub_lag_attr_get(_In_ const sai_object_key_t   *key,
                               _Inout_ sai_attribute_value_t *value,
                               _In_ uint32_t                  attr_index,
                               _Inout_ vendor_cache_t        *cache,
                               void                          *arg)
{
    sai_status_t    status;
    sai_object_id_t ports[PORT_NUMBER] = { 0 };
    uint32_t        lag_ports_count = 0;
    uint32_t        lag_number;

    STUB_LOG_ENTER();

    stub_lags_table_dbg_dump();

    switch ((int64_t)arg) {
        case SAI_LAG_ATTR_PORT_LIST:
            status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_LAG, &lag_number);
            if (SAI_STATUS_SUCCESS != status) {
                printf("[error] failed to get lag object data: 0x%x\n", status);
                goto out;

            } else if (lags_table[lag_number].oid == 0) {
                status = SAI_STATUS_ITEM_NOT_FOUND;
                printf("[error] specified LAG %d not found: 0x%x\n", lag_number, status);
                goto out;
            }

            for (uint32_t i = 0; i < PORT_NUMBER; i++) {
                if (lags_table[lag_number].ports_mask & (1 << i)) {
                    status = stub_create_object(SAI_OBJECT_TYPE_PORT, i, &ports[lag_ports_count++]);
                    if (SAI_STATUS_SUCCESS != status) {
                        printf("[error] failed when get port %d from LAG %lx: %d.\n", i, key->object_id, status);
                        goto out;
                    }
                }
            }

            status = stub_fill_objlist(ports, lag_ports_count, &value->objlist);
            if (SAI_STATUS_SUCCESS != status) {
                printf("[error] failed to fill objlist: 0x%x\n", status);
                goto out;
            }
            break;

        default:
            status = SAI_STATUS_FAILURE;
            printf("[error] unknown LAG attribute requested: 0x%x\n", status);
            goto out;
    }

out:
    STUB_LOG_EXIT();

    return status;
}


/* Get the LAG member attribute [sai_object_id_t] */
sai_status_t stub_lag_member_attr_get(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg)
{
    uint32_t        port_num;
    sai_status_t    status = SAI_STATUS_SUCCESS;

    STUB_LOG_ENTER();

    // stub_lags_table_dbg_dump();

    switch((int64_t)arg) {
        case SAI_LAG_MEMBER_ATTR_LAG_ID:
            // get the LAG OID
            status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_LAG_MEMBER, &port_num);
            if (SAI_STATUS_SUCCESS != status) {
                printf("[error] failed to get lag member object data: 0x%x\n", status);
                goto out;

            }

            for (uint32_t i = 0; i < STUB_MAX_LAGS; i++) {
                if (lags_table[i].oid && lags_table[i].ports_mask & (1 << port_num)) {
                    value->oid = lags_table[i].oid;
                }
            }
            break;

        case SAI_LAG_MEMBER_ATTR_PORT_ID:
            // get the Port OID
            status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_LAG_MEMBER, &port_num);
            if (SAI_STATUS_SUCCESS != status) {
                printf("[error] failed to get lag member object data: 0x%x\n", status);
                goto out;

            }

            for (uint32_t i = 0; i < STUB_MAX_LAGS; i++) {
                if (lags_table[i].oid && lags_table[i].ports_mask & (1 << port_num)) {
                    value->oid = lags_table[i].oid;
                }
            }
            break;

        default:
            status = SAI_STATUS_FAILURE;
            printf("[error] unknown LAG member attribute requested: 0x%x\n", status);
            goto out;
    }

out:
    STUB_LOG_EXIT();

    return status;
}

/* ==========================================================================================
 *   THE  C.R.U.D. FUNCTIONS
 * ========================================================================================== */

sai_status_t stub_create_lag(_Out_ sai_object_id_t* lag_id,
                             _In_ uint32_t attr_count,
                             _In_ sai_attribute_t* attr_list)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    char         list_str[MAX_LIST_VALUE_STR_LEN];
    uint32_t     lag_entry_id = STUB_MAX_LAGS;


    status = check_attribs_metadata(attr_count, attr_list, lag_attribs, lag_vendor_attribs, SAI_OPERATION_CREATE);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] incorrect attributes for LAG creation: 0x%x\n", status);
        goto out;
    }

    for (uint8_t i = 0; i < STUB_MAX_LAGS; i++) {
        // look for free lag entry
        if (lags_table[i].oid == 0) {
            lag_entry_id = i;
            break;
        }
    }

    if (lag_entry_id == STUB_MAX_LAGS) {
        status = SAI_STATUS_INSUFFICIENT_RESOURCES;
        printf("[error] failed to create stub LAG (no resources): 0x%x\n", status);
        goto out;
    }

    status = stub_create_object(SAI_OBJECT_TYPE_LAG, lag_entry_id, &lags_table[lag_entry_id].oid);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create stub LAG: 0x%x\n", status);
        goto out;

    } else {
        status = sai_attr_list_to_str(attr_count, attr_list, lag_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
        if (SAI_STATUS_SUCCESS != status) {
            printf("[error] failed to create attr list: 0x%x\n", status);
            goto out;
        }

        printf("Create LAG: 0x%010lx (%s)\n", lags_table[lag_entry_id].oid, list_str);
        *lag_id = lags_table[lag_entry_id].oid;
    }

    // clear the values
    lags_table[lag_entry_id].ports_cnt = 0;
    lags_table[lag_entry_id].ports_mask = 0x0;

out:
    return status;
}

sai_status_t stub_remove_lag(_In_ sai_object_id_t lag_id)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    for (uint8_t i = 0; i < STUB_MAX_LAGS; i++) {
        // look for lag entry
        if (lags_table[i].oid == lag_id) {

            // check LAG is use
            if (lags_table[i].ports_mask) {
                status = SAI_STATUS_OBJECT_IN_USE;
                printf("[error] failed to remove LAG (in use): 0x%x\n", status);
                goto out;
            }

            lags_table[i].oid = 0;
            printf("Remove LAG: 0x%010lx\n", lag_id);

            goto out;
        }
    }

    status = SAI_STATUS_ITEM_NOT_FOUND;

out:
    return status;
}

sai_status_t stub_set_lag_attribute(_In_ sai_object_id_t lag_id,
                                    _In_ const sai_attribute_t* attr)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t stub_get_lag_attribute(_In_ sai_object_id_t lag_id,
                                    _In_ uint32_t attr_count,
                                    _Inout_ sai_attribute_t* attr_list)
{
    const sai_object_key_t key = { .object_id = lag_id };
    char                   key_str[MAX_KEY_STR_LEN];

    STUB_LOG_ENTER();

    lag_key_to_str(lag_id, key_str);

    return sai_get_attributes(&key, key_str, lag_attribs, lag_vendor_attribs, attr_count, attr_list);
}

sai_status_t stub_create_lag_member(_Out_ sai_object_id_t* lag_member_id,
                                    _In_ uint32_t attr_count,
                                    _In_ sai_attribute_t* attr_list)
{
    sai_status_t                 status = SAI_STATUS_SUCCESS;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t* lag_id_attr_val;
    const sai_attribute_value_t* port_id_attr_val;
    uint32_t                     lag_id_attr_idx;
    uint32_t                     port_id_attr_idx;
    uint32_t                     lag_number;  // raw LAG number for DB indexing
    uint32_t                     port_number; // raw port number for DB indexing


    status = check_attribs_metadata(attr_count, attr_list, lag_member_attribs, lag_member_vendor_attribs, SAI_OPERATION_CREATE);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] incorrect attributes for LAG Member creation: 0x%x\n", status);
        goto out;
    }

    status = find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_LAG_ID, &lag_id_attr_val, &lag_id_attr_idx);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to find LAG ID attribute: 0x%x\n", status);
        goto out;
    }

    // process LAG ID value
    status = stub_object_to_type(lag_id_attr_val->oid, SAI_OBJECT_TYPE_LAG, &lag_number);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get lag object data: 0x%x\n", status);
        goto out;

    } else if (lags_table[lag_number].oid == 0) {
        status = SAI_STATUS_ITEM_NOT_FOUND;
        printf("[error] specified LAG %d not found: 0x%x\n", lag_number, status);
        goto out;

    } else if (lags_table[lag_number].ports_cnt >= STUB_MAX_LAG_PORTS) {
        // no more room for new ports in LAG
        status = SAI_STATUS_INSUFFICIENT_RESOURCES;
        printf("[error] specified LAG %d can't have more ports: 0x%x\n", lag_number, status);
        goto out;
    }

    status = find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_PORT_ID, &port_id_attr_val, &port_id_attr_idx);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to find PORT ID attribute: 0x%x\n", status);
        goto out;

    }

    // process Port ID value
    status = stub_object_to_type(port_id_attr_val->oid, SAI_OBJECT_TYPE_PORT, &port_number);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get port object data: 0x%x\n", status);
        goto out;

    } else {
        for (uint32_t i = 0; i < STUB_MAX_LAGS; i++) {
            if (lags_table[i].ports_mask & (1 << port_number)) {
                status = SAI_STATUS_ITEM_ALREADY_EXISTS;
                printf("[error] port is already added to LAG %d: 0x%x\n", i, status);
                goto out;
            }
        }
    }

    status = stub_create_object(SAI_OBJECT_TYPE_LAG_MEMBER, port_number, lag_member_id);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create stub LAG member: 0x%x\n", status);
        goto out;

    }

    // add port to LAG portmask and increase ports count
    lags_table[lag_number].ports_mask |= (1 << port_number);
    lags_table[lag_number].ports_cnt++;

    status = sai_attr_list_to_str(attr_count, attr_list, lag_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create attr list: 0x%x\n", status);
        goto out;
    }

    printf("Create LAG MEMBER: 0x%010lx {LAG_ID: %d, PORT_ID: %d}\n", *lag_member_id, lag_number, port_number);

out:
    return status;
}

sai_status_t stub_remove_lag_member(_In_ sai_object_id_t lag_member_id)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_number;

    status = stub_object_to_type(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER, &port_number);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get lag member object data: 0x%x\n", status);
        goto out;

    }

    for (uint8_t i = 0; i < STUB_MAX_LAGS; i++) {
        // look for initialized lag entry
        if (lags_table[i].oid) {

            // check port in LAG and remove
            if (lags_table[i].ports_mask & (1 << port_number)) {
                lags_table[i].ports_mask ^= (1 << port_number);
                lags_table[i].ports_cnt--;

                printf("Remove LAG MEMBER: 0x%010lx {LAG_ID: %d, PORT_ID: %d}\n", lag_member_id, i, port_number);
                goto out;
            }
        }
    }

    status = SAI_STATUS_ITEM_NOT_FOUND;

out:
    return status;
}

sai_status_t stub_set_lag_member_attribute(_In_ sai_object_id_t lag_member_id,
                                           _In_ const sai_attribute_t* attr)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t stub_get_lag_member_attribute(_In_ sai_object_id_t lag_member_id,
                                           _In_ uint32_t attr_count,
                                           _Inout_ sai_attribute_t* attr_list)
{
    const sai_object_key_t key = { .object_id = lag_member_id };
    char                   key_str[MAX_KEY_STR_LEN];

    STUB_LOG_ENTER();

    lag_member_key_to_str(lag_member_id, key_str);

    return sai_get_attributes(&key, key_str, lag_member_attribs, lag_member_vendor_attribs, attr_count, attr_list);
}

const sai_lag_api_t lag_api = {
    stub_create_lag,
    stub_remove_lag,
    stub_set_lag_attribute,
    stub_get_lag_attribute,
    stub_create_lag_member,
    stub_remove_lag_member,
    stub_set_lag_member_attribute,
    stub_get_lag_member_attribute
};
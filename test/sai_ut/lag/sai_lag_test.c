#include <stdio.h>
#include "sai.h"
#include "stub_sai.h"

const char* test_profile_get_value(_In_ sai_switch_profile_id_t profile_id,
                                   _In_ const char* variable)
{
    return 0;
}

int test_profile_get_next_value(_In_ sai_switch_profile_id_t profile_id,
                                _Out_ const char** variable,
                                _Out_ const char** value)
{
    return -1;
}

const service_method_table_t test_services = {
    test_profile_get_value,
    test_profile_get_next_value
};

// original test flow: test simple LAG and LAG member configuration
sai_status_t test_lag_flow_1(uint32_t ports_count, sai_object_id_t* port_list)
{
    sai_status_t              status;

    sai_lag_api_t*            lag_api;
    sai_object_id_t           lag_oid[2];
    sai_object_id_t           lag_member_oid[4];
    sai_attribute_t           lag_attributes;
    sai_attribute_t           lag_member_attributes[2];
    sai_object_id_t           lag_port_list[64];


    printf("\n RUNNING >>> LAG FLOW 1\n\n");

    status = sai_api_query(SAI_API_LAG, (void**) &lag_api);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get SAI LAG APIs: 0x%x\n", status);
        return -1;
    }

    // case 1. Create LAG 1 and its LAG Members

    status = lag_api->create_lag(&lag_oid[0], 0, NULL);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create LAG: 0x%x\n", status);
        return -1;
    }

    lag_member_attributes[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lag_member_attributes[0].value.oid = lag_oid[0];
    lag_member_attributes[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lag_member_attributes[1].value.oid = port_list[0];

    status = lag_api->create_lag_member(&lag_member_oid[0], 2, lag_member_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create LAG member: 0x%x\n", status);
        return -1;
    }

    lag_member_attributes[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lag_member_attributes[1].value.oid = port_list[1];

    status = lag_api->create_lag_member(&lag_member_oid[1], 2, lag_member_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create LAG member: 0x%x\n", status);
        return -1;
    }

    // case 2. Create LAG 2 and its LAG Members

    status = lag_api->create_lag(&lag_oid[1], 0, NULL);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create LAG: 0x%x\n", status);
        return -1;
    }

    lag_member_attributes[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lag_member_attributes[0].value.oid = lag_oid[1];
    lag_member_attributes[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lag_member_attributes[1].value.oid = port_list[2];

    status = lag_api->create_lag_member(&lag_member_oid[2], 2, lag_member_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create LAG member: 0x%x\n", status);
        return -1;
    }

    lag_member_attributes[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lag_member_attributes[1].value.oid = port_list[3];

    status = lag_api->create_lag_member(&lag_member_oid[3], 2, lag_member_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create LAG member: 0x%x\n", status);
        return -1;
    }

    // case 3. Get LAG 1 ports list

    lag_attributes.id = SAI_LAG_ATTR_PORT_LIST;
    lag_attributes.value.objlist.list = lag_port_list;
    lag_attributes.value.objlist.count = 64;

    status = lag_api->get_lag_attribute(lag_oid[0], 1, &lag_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get LAG attributes (ports list): 0x%x\n", status);
        return -1;
    }

    printf("GET LAG 0x%010lx PORTS LIST (%d ports): ", lag_oid[0], lag_attributes.value.objlist.count);
    for (int32_t i = 0; i < lag_attributes.value.objlist.count; i++) {
        printf("%d:0x%010lx ", i, lag_attributes.value.objlist.list[i]);
    }
    printf("\n");

    // case 4. Get LAG 2 ports list

    lag_attributes.id = SAI_LAG_ATTR_PORT_LIST;
    lag_attributes.value.objlist.list = lag_port_list;
    lag_attributes.value.objlist.count = 64;

    status = lag_api->get_lag_attribute(lag_oid[1], 1, &lag_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get LAG attributes (ports list): 0x%x\n", status);
        return -1;
    }

    printf("GET LAG 0x%010lx PORTS LIST (%d ports): ", lag_oid[1], lag_attributes.value.objlist.count);
    for (int32_t i = 0; i < lag_attributes.value.objlist.count; i++) {
        printf("%d:0x%010lx ", i, lag_attributes.value.objlist.list[i]);
    }
    printf("\n");


    // case 5. Get LAG Member 1 LAG ID

    lag_member_attributes[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lag_member_attributes[0].value.oid = 0;

    status = lag_api->get_lag_member_attribute(lag_member_oid[0], 1, &lag_member_attributes[0]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get LAG attributes (ports list): 0x%x\n", status);
        return -1;
    }

    printf("GET LAG MEMBER 0x%010lx LAG_ID: 0x%010lx\n", lag_member_oid[0], lag_member_attributes[0].value.oid);

    // case 6. Get LAG Member 3 Port ID

    lag_member_attributes[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lag_member_attributes[1].value.oid = 0;

    status = lag_api->get_lag_member_attribute(lag_member_oid[2], 1, &lag_member_attributes[1]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get LAG attributes (ports list): 0x%x\n", status);
        return -1;
    }

    printf("GET LAG MEMBER 0x%010lx PORT_ID: 0x%010lx\n", lag_member_oid[2], lag_member_attributes[1].value.oid);


    // case 7. Remove LAG Member 2

    status = lag_api->remove_lag_member(lag_member_oid[1]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to remove LAG member 2: 0x%x\n", status);
        return -1;
    }

    // case 8. Get LAG 1 ports list

    lag_attributes.id = SAI_LAG_ATTR_PORT_LIST;
    lag_attributes.value.objlist.list = lag_port_list;
    lag_attributes.value.objlist.count = 64;

    status = lag_api->get_lag_attribute(lag_oid[0], 1, &lag_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get LAG attributes (ports list): 0x%x\n", status);
        return -1;
    }

    printf("GET LAG 0x%010lx PORTS LIST (%d ports): ", lag_oid[0], lag_attributes.value.objlist.count);
    for (int32_t i = 0; i < lag_attributes.value.objlist.count; i++) {
        printf("%d:0x%010lx ", i, lag_attributes.value.objlist.list[i]);
    }
    printf("\n");

    // case 9. Remove LAG Member 3

    status = lag_api->remove_lag_member(lag_member_oid[2]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to remove LAG member 3: 0x%x\n", status);
        return -1;
    }

    // case 10. Get LAG 2 ports list

    lag_attributes.id = SAI_LAG_ATTR_PORT_LIST;
    lag_attributes.value.objlist.list = lag_port_list;
    lag_attributes.value.objlist.count = 64;

    status = lag_api->get_lag_attribute(lag_oid[1], 1, &lag_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get LAG attributes (ports list): 0x%x\n", status);
        return -1;
    }

    printf("GET LAG 0x%010lx PORTS LIST (%d ports): ", lag_oid[1], lag_attributes.value.objlist.count);
    for (int32_t i = 0; i < lag_attributes.value.objlist.count; i++) {
        printf("%d:0x%010lx ", i, lag_attributes.value.objlist.list[i]);
    }
    printf("\n");

    // case 11. Remove LAG Members 1 & 4

    status = lag_api->remove_lag_member(lag_member_oid[0]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to remove LAG member 1: 0x%x\n", status);
        return -1;
    }

    status = lag_api->remove_lag_member(lag_member_oid[3]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to remove LAG member 4: 0x%x\n", status);
        return -1;
    }

    // case 12. Remove LAGs 1 & 2

    status = lag_api->remove_lag(lag_oid[0]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to remove LAG : 0x%x\n", status);
        return -1;
    }

    status = lag_api->remove_lag(lag_oid[1]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to remove LAG : 0x%x\n", status);
        return -1;
    }

    return status;
}


// new test flow: test some LAG component corner-cases
sai_status_t test_lag_flow_2(uint32_t ports_count, sai_object_id_t* port_list)
{
    sai_status_t              status;

    sai_lag_api_t*            lag_api;
    sai_object_id_t           lag_oid[6];
    sai_object_id_t           lag_member_oid[32];
    sai_attribute_t           lag_attributes;
    sai_attribute_t           lag_member_attributes[2];
    sai_object_id_t           lag_port_list[64];


    printf("\n RUNNING >>> LAG FLOW 2\n\n");

    status = sai_api_query(SAI_API_LAG, (void**) &lag_api);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get SAI LAG APIs: 0x%x\n", status);
        return -1;
    }

    // case 1. Create 5 LAGs

    for (uint32_t i = 0; i < 5; i++) {
        status = lag_api->create_lag(&lag_oid[i], 0, NULL);
        if (SAI_STATUS_SUCCESS != status) {
            printf("[error] failed to create LAG: 0x%x\n", status);
            return -1;
        }
    }

    // case 2. Create 6th LAG - fail expected

    status = lag_api->create_lag(&lag_oid[4], 0, NULL);
    if (SAI_STATUS_INSUFFICIENT_RESOURCES != status) {
        printf("[error] expected fail on LAG create: 0x%x\n", status);
        return -1;
    }

    // case 3. Delete 3rd LAG

    status = lag_api->remove_lag(lag_oid[2]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to delete LAG: 0x%x\n", status);
        return -1;
    }

    // case 4. Create 6th LAG - pass expected

    status = lag_api->create_lag(&lag_oid[5], 0, NULL);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create LAG: 0x%x\n", status);
        return -1;
    }

    // case 5. Create 16 LAG members on LAG 1

    lag_member_attributes[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lag_member_attributes[0].value.oid = lag_oid[0];
    lag_member_attributes[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;

    for (uint32_t i = 0; i < 16; i++) {
        lag_member_attributes[1].value.oid = port_list[i];

        status = lag_api->create_lag_member(&lag_member_oid[i], 2, lag_member_attributes);
        if (SAI_STATUS_SUCCESS != status) {
            printf("[error] failed to create LAG member: 0x%x\n", status);
            return -1;
        }
    }

    // case 6. Create 17th LAG member on LAG 1 - fail expected

    lag_member_attributes[1].value.oid = port_list[16];

    status = lag_api->create_lag_member(&lag_member_oid[16], 2, lag_member_attributes);
    if (SAI_STATUS_INSUFFICIENT_RESOURCES != status) {
        printf("[error] expected fail on LAG member create: 0x%x\n", status);
        return -1;
    }

    // case 7. Remove 10th LAG member on LAG 1

    status = lag_api->remove_lag_member(lag_member_oid[9]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to remove LAG member 2: 0x%x\n", status);
        return -1;
    }

    // case 8. Create 17th LAG member on LAG 1 - success expected

    lag_member_attributes[1].value.oid = port_list[16];

    status = lag_api->create_lag_member(&lag_member_oid[16], 2, lag_member_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to create LAG member: 0x%x\n", status);
        return -1;
    }

    // case 9. Get LAG 1 ports list

    lag_attributes.id = SAI_LAG_ATTR_PORT_LIST;
    lag_attributes.value.objlist.list = lag_port_list;
    lag_attributes.value.objlist.count = 64;

    status = lag_api->get_lag_attribute(lag_oid[0], 1, &lag_attributes);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get LAG attributes (ports list): 0x%x\n", status);
        return -1;
    }

    printf("GET LAG 0x%010lx PORTS LIST (%d ports): ", lag_oid[0], lag_attributes.value.objlist.count);
    for (int32_t i = 0; i < lag_attributes.value.objlist.count; i++) {
        printf("%d:0x%010lx ", i, lag_attributes.value.objlist.list[i]);
    }
    printf("\n");

    // case 10. Remove all LAGs

    for (uint32_t i = 0; i < 6; i++) {
        if (i == 2) {
            // skip third LAG OID - its empty
            continue;
        }

        status = lag_api->remove_lag(lag_oid[i]);

        // expected to delete all LAGs successfully except first LAG (has members) and 3rd LAG (not exist)
        if (SAI_STATUS_SUCCESS != status && i != 0) {
            printf("[error] failed to remove LAG: 0x%x\n", status);
            return -1;
        } else if (i == 0 && SAI_STATUS_OBJECT_IN_USE != status) {
            printf("[error] expected fail on first LAG: 0x%x\n", status);
            return -1;
        }
    }

    // case 11. Remove all LAG 1 members

    for (uint32_t i = 0; i < 17; i++) {
        status = lag_api->remove_lag_member(lag_member_oid[i]);

        // expected to delete all LAG members successfully except 10th LAG member (not exist)
        if (SAI_STATUS_SUCCESS != status && i != 9) {
            printf("[error] failed to remove LAG member: 0x%x\n", status);
            return -1;
        } else if (i == 9 && SAI_STATUS_ITEM_NOT_FOUND != status) {
            printf("[error] expected fail on 10th LAG: 0x%x\n", status);
            return -1;
        }
    }

    // case 12. Remove LAG 1

    status = lag_api->remove_lag(lag_oid[0]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to remove LAG : 0x%x\n", status);
        return -1;
    }

    return status;
}

int main()
{
    sai_status_t              status;
    sai_switch_api_t*         switch_api;
    sai_attribute_t           switch_attrs[2];
    sai_switch_notification_t notifications;
    sai_object_id_t           port_list[64];


    status = sai_api_initialize(0, &test_services);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to initialize SAI API: 0x%x\n", status);
        return -1;
    }

    // ===================================================================== Switch init

    status = sai_api_query(SAI_API_SWITCH, (void**) &switch_api);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get SAI switch APIs: 0x%x\n", status);
        return -1;
    }

    status = switch_api-> initialize_switch(0, "HW_ID", 0, &notifications);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to initialize switch: 0x%x\n", status);
        return -1;
    }

    switch_attrs[0].id = SAI_SWITCH_ATTR_PORT_LIST;
    switch_attrs[0].value.objlist.list = port_list;
    switch_attrs[0].value.objlist.count = 64;

    status = switch_api->get_switch_attribute(1, switch_attrs);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] failed to get switch port list: 0x%x\n", status);
        return -1;
    }

    printf("Switch ports available:\n");
    for (int32_t i = 0; i < switch_attrs[0].value.objlist.count; i++) {
        if (i % 4 == 0) {
            printf("\n");
        }
        printf("%-2d: 0x%010lx ", i, port_list[i]);
    }
    printf("\n\n");


    // ===================================================================== Test cases

    status = test_lag_flow_1(switch_attrs[0].value.objlist.count, port_list);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] LAG test flow 1 failed: 0x%x\n", status);
        return -1;
    }

    status = test_lag_flow_2(switch_attrs[0].value.objlist.count, port_list);
    if (SAI_STATUS_SUCCESS != status) {
        printf("[error] LAG test flow 2 failed: 0x%x\n", status);
        return -1;
    }


    // ===================================================================== Switch de-init

    switch_api->shutdown_switch(0);

    status = sai_api_uninitialize();

    return 0;
}
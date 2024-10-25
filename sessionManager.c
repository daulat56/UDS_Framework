#include "server.h"
#include <stdio.h>

SessionInfo g_session_info = {
    .current_session = DEFAULT_SESSION,
    .p2_server_max = P2_SERVER_MAX,
    .p2_star_server_max = P2_STAR_SERVER_MAX
};

void initializeSession(void) {
    g_session_info.current_session = DEFAULT_SESSION;
    g_session_info.p2_server_max = P2_SERVER_MAX;
    g_session_info.p2_star_server_max = P2_STAR_SERVER_MAX;
}

void updateSession(SubFunctionType new_session) {
    g_session_info.current_session = new_session;
    
    // Update timing parameters based on the new session
    switch (new_session) {
        case PROGRAMMING_SESSION:
            g_session_info.p2_server_max = 100; // Example value
            g_session_info.p2_star_server_max = 10000; // Example value
            break;
        case EXTENDED_SESSION:
            g_session_info.p2_server_max = 75; // Example value
            g_session_info.p2_star_server_max = 7500; // Example value
            break;
        default:
            g_session_info.p2_server_max = P2_SERVER_MAX;
            g_session_info.p2_star_server_max = P2_STAR_SERVER_MAX;
            break;
    }
    
    //printf("Session updated to: 0x%02X\n", new_session);
    //printf("P2_SERVER_MAX: %u ms\n", g_session_info.p2_server_max);
    //printf("P2_STAR_SERVER_MAX: %u ms\n", g_session_info.p2_star_server_max);
}
bool isSessionAllowedInService(ServiceState service, SubFunctionType requested_session) {
    UDS_Table udsServiceTable = {};
    getUDSTable(&udsServiceTable);

    // First, check if the service is allowed in the current session
    for (int i = 0; i < udsServiceTable.size; i++) {
        if (udsServiceTable.row[i].service_id == service) {
            if ((udsServiceTable.row[i].allowed_sessions & (1 << (g_session_info.current_session - 1))) == 0) {
                return false; // Service not allowed in current session
            }
            break;
        }
    }

    // Now, check the session transition rules
    switch (g_session_info.current_session) {
        case DEFAULT_SESSION:
            // All sessions are allowed from default session
            return true;
        case PROGRAMMING_SESSION:
            // Only default and programming sessions are allowed
            return (requested_session == DEFAULT_SESSION || requested_session == PROGRAMMING_SESSION);
        case EXTENDED_SESSION:
            // Only default and extended sessions are allowed
            return (requested_session == DEFAULT_SESSION || requested_session == EXTENDED_SESSION);
        default:
            return false; // Unknown current session
    }
}

bool isSessionAllowed(ServiceState service, SubFunctionType session) {
    UDS_Table udsServiceTable = {};
    getUDSTable(&udsServiceTable);    
    for (int i = 0; i < udsServiceTable.size; i++) {
        if (udsServiceTable.row[i].service_id == service) {
            return (udsServiceTable.row[i].allowed_sessions && (1 << (session - 1))) != 0;
        }
    }
    //printf("the session allowed value is : %d\n",isSessionAllowed);
    return false;
}
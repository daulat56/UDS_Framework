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

void updateSession(SessionType new_session) {
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
    
    printf("Session updated to: 0x%02X\n", new_session);
    printf("P2_SERVER_MAX: %u ms\n", g_session_info.p2_server_max);
    printf("P2_STAR_SERVER_MAX: %u ms\n", g_session_info.p2_star_server_max);
}

bool isSessionAllowed(ServiceState service, SessionType session) {
    UDS_Table udsServiceTable = {};
    getUDSTable(&udsServiceTable);
    
    for (int i = 0; i < udsServiceTable.size; i++) {
        if (udsServiceTable.row[i].service_id == service) {
            return (udsServiceTable.row[i].allowed_sessions & (1 << (session - 1))) != 0;
        }
    }
    
    return false;
}
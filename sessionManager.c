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
#define MAX_SESSIONS 3
//bool isSessionAllowedInService(ServiceState service, SubFunctionType requested_session) {
bool isSubfunctionSupportedInSession(SubFunctionEntry *SubFunction, SubFunctionType current_session){
    bool sessionFound = false;
    //printf("Allowed sessions bitmask: 0x%02X and the current session:  0x%02X \n", SubFunction->allowed_sessions,current_session);
   // First check if the subfunction is allowed in the current session using the bitmask
    for (int i = 0; i < sizeof(SubFunction->allowed_sessions)/sizeof(SubFunction->allowed_sessions[0]); i++) {
        if(SubFunction->allowed_sessions[i]==current_session){
            printf("Session 0x%02X is allowed for this subfunction\n", current_session);
            sessionFound = true;
            break;
        }
    }
    if (!sessionFound) {
        return false;
    }

/*
    // Now, check the session transition rules
    switch (current_session) {
        printf("the current session is :0x%02X\n",current_session);
        case DEFAULT_SESSION:
            // All sessions are allowed from default session
            return true;
        case PROGRAMMING_SESSION:
            // Only default and programming sessions are allowed
            //return (current_session == DEFAULT_SESSION || current_session == PROGRAMMING_SESSION);
            return (SubFunction->subfunction == DEFAULT_SESSION || SubFunction->subfunction == PROGRAMMING_SESSION);
            
        case EXTENDED_SESSION:
            // Only default and extended sessions are allowed
            //return (current_session == DEFAULT_SESSION || current_session == EXTENDED_SESSION);
            return (SubFunction->subfunction == DEFAULT_SESSION || SubFunction->subfunction == EXTENDED_SESSION);
        default:
            printf("Unknown current session: 0x%02X\n", current_session);
            return false; // Unknown current session
    }
    */
   return true;
}

bool isSessionAllowed(ServiceState service, SubFunctionType session) {
    UDS_Table udsServiceTable = {};
    getUDSTable(&udsServiceTable);
    
    // Find the service in the table
    for (int i = 0; i < udsServiceTable.size; i++) {
        if (udsServiceTable.row[i].service_id == service) {
            // Check if session is in allowed_sessions array
            for (int j = 0; j < sizeof(udsServiceTable.row[i].allowed_sessions)/sizeof(udsServiceTable.row[i].allowed_sessions[0]); j++) {
                if (udsServiceTable.row[i].allowed_sessions[j] == session) {
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

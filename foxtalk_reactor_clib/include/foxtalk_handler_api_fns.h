//
// Created by fox on 10/11/24.
//

#ifndef REACTOR_FOXTALK_HANDLER_API_FNS_H
#define REACTOR_FOXTALK_HANDLER_API_FNS_H

extern "C" {
    typedef void (*Claim)();
    typedef void (*Remove)();
    typedef bool (*GetNextQueryResult)();

    typedef struct {
        Claim claim;
        Remove remove;
        GetNextQueryResult getNextQueryResult;
    } HandlerFunctions;
};

#endif //REACTOR_FOXTALK_HANDLER_API_FNS_H

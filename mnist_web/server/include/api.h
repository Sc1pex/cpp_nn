#pragma once

#include <httc/router.h>
#include <memory>
#include "state.h"

void network_api_routes(std::shared_ptr<httc::Router> router, std::shared_ptr<State> state);

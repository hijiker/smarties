//
//  smarties
//  Copyright (c) 2018 CSE-Lab, ETH Zurich, Switzerland. All rights reserved.
//  Distributed under the terms of the MIT license.
//
//  Created by Guido Novati (novatig@ethz.ch).
//

#pragma once
#include "../Agent.h"
#include "../Communicators/Communicator_internal.h"
#include <map>

class Builder;

class Environment
{
protected:
    mt19937 * const g; //only ok if only thread 0 accesses
    Communicator_internal* comm_ptr = nullptr;
    void commonSetup();

public:
    Settings & settings;
    Uint nAgents, nAgentsPerRank;
    const Real gamma;

    vector<Agent*> agents;
    StateInfo  sI;
    ActionInfo aI;
    Environment(Settings & _settings);

    virtual ~Environment();

    virtual void setDims ();

    virtual bool pickReward(const Agent& agent) const;
    virtual bool predefinedNetwork(Builder & input_net) const;
    Communicator_internal create_communicator();

    virtual Uint getNdumpPoints();
    virtual Rvec getDumpState(Uint k);

    // for a given environment, size of the IRL reward dictionary
    virtual Uint getNumberRewardParameters();

    // compute the reward given a certain state and param vector
    virtual Real getReward(const std::vector<memReal> s, const Rvec params);

    // compute the gradient of the reward
    virtual Rvec getRewardGrad(const std::vector<memReal> s, const Rvec params);
};

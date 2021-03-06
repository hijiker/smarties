//
//  smarties
//  Copyright (c) 2018 CSE-Lab, ETH Zurich, Switzerland. All rights reserved.
//  Distributed under the terms of the MIT license.
//
//  Created by Guido Novati (novatig@ethz.ch).
//

#pragma once

#include "Functions.h"

struct Activation
{
  Uint _nOuts(vector<Uint> _sizes, vector<Uint> _bOut) {
    assert(_sizes.size() == _bOut.size() && (size_t) nLayers == _bOut.size());
    Uint ret = 0;
    for(Uint i=0; i<_bOut.size(); i++) if(_bOut[i]) ret += _sizes[i];
    if(!ret) die("err nOutputs");
    return ret;
  }
  Uint _nInps(vector<Uint> _sizes, vector<Uint> _bInp) {
    assert(_sizes.size() == _bInp.size() && (size_t) nLayers == _bInp.size());
    Uint ret = 0;
    for(Uint i=0; i<_bInp.size(); i++) if(_bInp[i]) ret += _sizes[i];
    return ret;
  }

  Activation(vector<Uint>_sizes, vector<Uint>_bOut, vector<Uint>_bInp):
    nLayers(_sizes.size()), nOutputs(_nOuts(_sizes,_bOut)), nInputs(_nInps(_sizes,_bInp)), sizes(_sizes), output(_bOut), input(_bInp),
    suminps(allocate_vec(_sizes)), outvals(allocate_vec(_sizes)), errvals(allocate_vec(_sizes)) {
    assert(suminps.size()== (size_t) nLayers);
    assert(outvals.size()== (size_t) nLayers);
    assert(errvals.size()== (size_t) nLayers);
  }

  ~Activation() {
    for(auto& p : suminps) if(p not_eq nullptr) free(p);
    for(auto& p : outvals) if(p not_eq nullptr) free(p);
    for(auto& p : errvals) if(p not_eq nullptr) free(p);
  }

  template<typename T>
  inline void setInput(const vector<T> inp) const {
    assert( (size_t) nInputs == inp.size());
    for(int j=0; j<nInputs; j++)
      assert(!std::isnan(inp[j]) && !std::isinf(inp[j]));
    int k=0;
    for(int i=0; i<nLayers; i++) if(input[i]) {
      std::copy(&inp[k], &inp[k]+sizes[i], outvals[i]);
      //memcpy(outvals[i], &inp[k], sizes[i]*sizeof(nnReal));
      k += sizes[i];
    }
    assert(k == nInputs);
  }
  inline vector<Real> getInput() const {
    vector<Real> ret(nInputs);
    int k=0;
    for(int i=0; i<nLayers; i++) if(input[i]) {
      std::copy(outvals[i], outvals[i]+sizes[i], &ret[k]);
      //memcpy(&ret[k], outvals[i], sizes[i]*sizeof(nnReal));
      k += sizes[i];
    }
    assert(k == nInputs);
    return ret;
  }

  inline vector<Real> getInputGradient(const Uint ID) const {
    assert(written == true);
    vector<Real> ret(sizes[ID]);
    std::copy(errvals[ID], errvals[ID]+sizes[ID], &ret[0]);
    //memcpy(&ret[0], errvals[ID], sizes[ID]*sizeof(nnReal));
    return ret;
  }

  template<typename T>
  inline void setOutputDelta(const vector<T> delta) const {
    assert( (size_t) nOutputs == delta.size()); //alternative not supported
    for(int j=0; j<nOutputs; j++)
      assert(!std::isnan(delta[j]) && !std::isinf(delta[j]));
    int k=0;
    for(int i=0; i<nLayers; i++) if(output[i]) {
      std::copy(&delta[k], &delta[k]+sizes[i], errvals[i]);
      //memcpy(errvals[i], &delta[k], sizes[i]*sizeof(nnReal));
      k += sizes[i];
    }
    assert(k == nOutputs);
    written = true;
  }

  template<typename T>
  inline void addOutputDelta(const vector<T> delta) const {
    assert( (size_t) nOutputs == delta.size()); //alternative not supported
    int k=0;
    for(int i=0; i<nLayers; i++) if(output[i])
      for (Uint j=0; j<sizes[i]; j++, k++) errvals[i][j] += delta[k];
    assert(k == nOutputs);
    written = true;
  }

  inline vector<nnReal> getOutputDelta() const {
    assert(written == true);
    vector<nnReal> ret(nOutputs);
    int k=0;
    for(int i=0; i<nLayers; i++) if(output[i]) {
      std::copy(errvals[i], errvals[i]+sizes[i], &ret[k]);
      //memcpy(&ret[k], errvals[i], sizes[i]*sizeof(nnReal));
      k += sizes[i];
    }
    assert(k == nOutputs);
    return ret;
  }

  inline vector<Real> getOutput() const {
    assert(written == true);
    vector<Real> ret(nOutputs);
    int k=0;
    for(int i=0; i<nLayers; i++) if(output[i]) {
      std::copy(outvals[i], outvals[i]+sizes[i], &ret[k]);
      //memcpy(&ret[k], outvals[i], sizes[i]*sizeof(nnReal));
      k += sizes[i];
    }
    for(int j=0; j<nOutputs; j++)
      assert(!std::isnan(ret[j]) && !std::isinf(ret[j]));
    assert(k == nOutputs);
    return ret;
  }

  inline void clearOutput() const {
    for(int i=0; i<nLayers; i++) {
      assert(outvals[i] not_eq nullptr);
      std::memset( outvals[i], 0, roundUpSimd(sizes[i])*sizeof(nnReal) );
    }
  }

  inline void clearErrors() const {
    for(int i=0; i<nLayers; i++) {
      assert(errvals[i] not_eq nullptr);
      std::memset( errvals[i], 0, roundUpSimd(sizes[i])*sizeof(nnReal) );
    }
  }

  inline void clearInputs() const {
    for(int i=0; i<nLayers; i++) {
      assert(suminps[i] not_eq nullptr);
      std::memset( suminps[i], 0, roundUpSimd(sizes[i])*sizeof(nnReal) );
    }
  }

  inline nnReal* X(const int layerID) const {
    assert(layerID < nLayers);
    return suminps[layerID];
  }
  inline nnReal* Y(const int layerID) const {
    assert(layerID < nLayers);
    return outvals[layerID];
  }
  inline nnReal* E(const int layerID) const {
    assert(layerID < nLayers);
    return errvals[layerID];
  }

  const int nLayers, nOutputs, nInputs;
  const vector<Uint> sizes, output, input;
  //contains all inputs to each neuron (inputs to network input layer is empty)
  const vector<nnReal*> suminps;
  //contains all neuron outputs that will be the incoming signal to linked layers (outputs of input layer is network inputs)
  const vector<nnReal*> outvals;
  //deltas for each neuron
  const vector<nnReal*> errvals;
  mutable bool written = false;
};

inline void deallocateUnrolledActivations(vector<Activation*>& r)
{
  for (auto & ptr : r) _dispose_object(ptr);
  r.clear();
}
inline void deallocateUnrolledActivations(vector<Activation*>* r)
{
  for (auto & ptr : *r) _dispose_object(ptr);
  r->clear();
}

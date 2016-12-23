#ifndef NODES_TPM_WRAPPER_HPP
#define NODES_TPM_WRAPPER_HPP

#include <cstring>
#include <typeinfo>
extern "C" {
#include "libtexproma.h"
}
#include "Function.hpp"

namespace tpmWrapper {

/************
 *** DATA ***
 ************/

class MonoBuf {
 public:
  MonoBuf() { data_ = (tpm_mono_buf)malloc(TP_WIDTH * TP_HEIGHT); }
  MonoBuf(MonoBuf&& other) {
    data_ = (tpm_mono_buf)malloc(1);
    *this = other;
  }
  MonoBuf(const MonoBuf& other) : MonoBuf() { *this = other; }
  MonoBuf(tpm_mono_buf buf) : MonoBuf() { *this = buf; }
  void operator=(tpm_mono_buf buf) { copy(data_, buf); }
  void operator=(const MonoBuf& other) { copy(data_, other.data_); }
  void operator=(MonoBuf&& other) { std::swap(data_, other.data_); }
  tpm_mono_buf getRawPtr() { return data_; }
  ~MonoBuf() { free(data_); }

 private:
  void copy(tpm_mono_buf dst, tpm_mono_buf src) {
    std::memcpy(dst, src, TP_WIDTH * TP_HEIGHT);
  }
  tpm_mono_buf data_;
};

class ColorBuf {
 public:
  ColorBuf() { data_ = (tpm_color_buf)malloc(TP_WIDTH * TP_HEIGHT * 4); }
  ColorBuf(ColorBuf&& other) {
    data_ = (tpm_color_buf)malloc(1);
    *this = other;
  }
  ColorBuf(const ColorBuf& other) : ColorBuf() { *this = other; }
  ColorBuf(tpm_color_buf buf) : ColorBuf() { *this = buf; }
  void operator=(tpm_color_buf buf) { copy(data_, buf); }
  void operator=(const ColorBuf& other) { copy(data_, other.data_); }
  void operator=(ColorBuf&& other) { std::swap(data_, other.data_); }
  tpm_color_buf getRawPtr() { return data_; }
  ~ColorBuf() { free(data_); }

 private:
  tpm_color_buf data_;
  void copy(tpm_color_buf dst, tpm_color_buf src) {
    std::memcpy(dst, src, TP_WIDTH * TP_HEIGHT * 4);
  }
};

/*****************
 *** FUNCTIONS ***
 *****************/

struct TpmSine : public Function {
  Types getParamTypes() const override { return makeTypeVector<float>(); }
  Types getReturnTypes() const override { return makeTypeVector<MonoBuf>(); }
  std::string getName() const override { return "tpm_sine"; }
  Values operator()(Values params) const override {
    Values returns;
    MonoBuf buff;
    tpm_sine(buff.getRawPtr(), extractRawData<float>(params[0]));
    returns.push_back(makeSharedData(buff));
    return returns;
  }
};

struct TpmNoise : public Function {
  Types getParamTypes() const override {
    return makeTypeVector<unsigned, unsigned>();
  }
  Types getReturnTypes() const override { return makeTypeVector<MonoBuf>(); }
  std::string getName() const override { return "tpm_noise"; }
  Values operator()(Values params) const override {
    Values returns;
    MonoBuf buff;
    tpm_noise(buff.getRawPtr(), extractRawData<unsigned>(params[0]),
              extractRawData<unsigned>(params[1]));
    returns.push_back(makeSharedData(buff));
    return returns;
  }
};

struct TpmPlasma : public Function {
  Types getParamTypes() const override { return Types(); }
  Types getReturnTypes() const override { return makeTypeVector<MonoBuf>(); }
  std::string getName() const override { return "tpm_plasma"; }
  Values operator()(Values params) const override {
    Values returns;
    MonoBuf buff;
    tpm_plasma(buff.getRawPtr());
    returns.push_back(makeSharedData(buff));
    return returns;
  }
};

struct TpmLight : public Function {
  Types getParamTypes() const override {
    return makeTypeVector<unsigned, float>();
  }
  Types getReturnTypes() const override { return makeTypeVector<MonoBuf>(); }
  std::string getName() const override { return "tpm_light"; }
  Values operator()(Values params) const override {
    Values returns;
    MonoBuf buff;
    tpm_light(buff.getRawPtr(), extractRawData<unsigned>(params[0]),
              extractRawData<float>(params[1]));
    returns.push_back(makeSharedData(buff));
    return returns;
  }
};

struct TpmPerlinNoise : public Function {
  Types getParamTypes() const override { return makeTypeVector<unsigned>(); }
  Types getReturnTypes() const override { return makeTypeVector<MonoBuf>(); }
  std::string getName() const override { return "tpm_perlin_noise"; }
  Values operator()(Values params) const override {
    Values returns;
    MonoBuf buff;
    tpm_perlin_noise(buff.getRawPtr(), extractRawData<unsigned>(params[0]));
    returns.push_back(makeSharedData(buff));
    return returns;
  }
};

struct TpmInvert : public Function {
  Types getParamTypes() const override { return makeTypeVector<MonoBuf>(); }
  Types getReturnTypes() const override { return makeTypeVector<MonoBuf>(); }
  std::string getName() const override { return "tpm_invert"; }
  Values operator()(Values params) const override {
    Values returns;
    MonoBuf buff;
    tpm_invert(buff.getRawPtr(), extractRawData<MonoBuf>(params[0]).getRawPtr());
    returns.push_back(makeSharedData(buff));
    return returns;
  }
};

struct TpmAdd : public Function {
  Types getParamTypes() const override { return makeTypeVector<MonoBuf, MonoBuf>(); }
  Types getReturnTypes() const override { return makeTypeVector<MonoBuf>(); }
  std::string getName() const override { return "tpm_add"; }
  Values operator()(Values params) const override {
    Values returns;
    MonoBuf buff;
    tpm_add(buff.getRawPtr(), extractRawData<MonoBuf>(params[0]).getRawPtr(), extractRawData<MonoBuf>(params[1]).getRawPtr());
    returns.push_back(makeSharedData(buff));
    return returns;
  }
};

}

#endif
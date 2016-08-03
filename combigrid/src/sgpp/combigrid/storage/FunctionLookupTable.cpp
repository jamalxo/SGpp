// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/combigrid/storage/FunctionLookupTable.hpp>
#include <sgpp/combigrid/utils/Utils.hpp>
#include <sgpp/combigrid/serialization/FloatSerializationStrategy.hpp>
#include <sgpp/combigrid/serialization/DefaultSerializationStrategy.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace sgpp {
namespace combigrid {

FunctionLookupTable::FunctionLookupTable(MultiFunction func)
    : hashmap(
          new std::unordered_map<base::DataVector, double, DataVectorHash, DataVectorEqualTo>()),
      func(func),
      tableMutex() {}

double FunctionLookupTable::operator()(const base::DataVector& x) {
  auto it = hashmap->find(x);
  if (it == hashmap->end()) {
    auto y = func(x);
    addEntry(x, y);
    return y;
  }

  return it->second;
}

double FunctionLookupTable::eval(const base::DataVector& x) { return (*this)(x); }

double FunctionLookupTable::evalThreadsafe(const base::DataVector& x) {
  tableMutex.lock();
  auto it = hashmap->find(x);
  if (it == hashmap->end()) {
    tableMutex.unlock();
    auto y = func(x);
    tableMutex.lock();
    addEntry(x, y);
    tableMutex.unlock();
    return y;
  }

  std::lock_guard<std::mutex> guard(tableMutex);
  auto y = it->second;
  tableMutex.unlock();
  return y;
}

void FunctionLookupTable::addEntry(const base::DataVector& x, double y) { (*hashmap)[x] = y; }

std::string FunctionLookupTable::serialize() {
  FloatSerializationStrategy<double> strategy;

  std::vector<std::string> entries;

  for (auto it = hashmap->begin(); it != hashmap->end(); ++it) {
    std::vector<std::string> vectorEntries;

    auto& vec = it->first;

    for (size_t i = 0; i < vec.getSize(); ++i) {
      vectorEntries.push_back(strategy.serialize(vec[i]));
    }

    entries.push_back(join(vectorEntries, ", ") + " -> " + strategy.serialize(it->second));
  }

  return join(entries, "\n");
}

void FunctionLookupTable::deserialize(const std::string& value) {
  FloatSerializationStrategy<double> strategy;

  std::vector<std::string> entries = split(value, "\n");

  for (size_t i = 0; i < entries.size(); ++i) {
    if (entries[i].length() == 0) {
      continue;
    }

    std::vector<std::string> keyValuePair = split(entries[i], " -> ");

    if (keyValuePair.size() != 2) {
      throw std::runtime_error(
          "FunctionLookupTable::deserialize(): keyValuePair does not have two entries but " +
          DefaultSerializationStrategy<size_t>().serialize(keyValuePair.size()) +
          " entries in line " + DefaultSerializationStrategy<size_t>().serialize(i) + ": " +
          entries[i]);
    }

    std::vector<std::string> dataVectorEntries = split(keyValuePair[0], ", ");

    base::DataVector x(dataVectorEntries.size());

    for (size_t j = 0; j < dataVectorEntries.size(); ++j) {
      x[j] = strategy.deserialize(dataVectorEntries[j]);
    }

    double y = strategy.deserialize(keyValuePair[1]);

    addEntry(x, y);
  }
}

bool FunctionLookupTable::containsEntry(const base::DataVector& x) {
  auto it = hashmap->find(x);
  return it != hashmap->end();
}

size_t FunctionLookupTable::getNumEntries() const { return hashmap->size(); }
}  // namespace combigrid
}  // namespace sgpp
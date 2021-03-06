// Copyright 2020 Michael Vlach
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AGRAPH_GRAPH_HPP
#define AGRAPH_GRAPH_HPP

#include "GraphBase.hpp"
#include "GraphData.hpp"

namespace agraph
{
//! The Graph provides in-memory directional
//! graph implementation.
//!
//! The class is a specialization of the GraphBase.
//! It provides in memory graph representation to be
//! used for graph algorithms.
//!
//! Example of usage:
//! \snippet GraphTest.cpp [Usage]
class Graph : public GraphBase<GraphData, Graph>
{
};
}

#endif

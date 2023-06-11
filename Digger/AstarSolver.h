#pragma once
#include <vector>
#include <functional>
#include <array>
#include <map>
#include <set>

namespace Astar
{
	namespace {

		template<typename State>
		struct ComputeNode {
			ComputeNode(const State& s, size_t actionID = -1, ComputeNode<State>* node = nullptr)
				: state(s), actionID(actionID), prevNode(node), G(0.f), Cost(0.f) {
			}

			ComputeNode* prevNode{nullptr};
			State state;
			size_t actionID;
			float G;
			float Cost;
		};
	}

	template <typename State, typename Action>
	std::list<int> FindActionPath(
		State BeginState,
		std::vector<Action> Actions,
		std::function<bool(const State& state, const Action& act)> ValidPreCheck,
		std::function<bool(const State& state)> ValidPostCheck,
		std::function<bool(const State& state)> Visit,
		std::function<State(const State& state, const Action& act)> Act,
		std::function<float(const State& state, int action)> GetStateCost,
		std::function<float(const State& state, float PathCost)> GetCost)
	{
		typedef ComputeNode<State>* NodeType;
		std::list<NodeType> NodeList;

		std::map<State, NodeType> openList;
		std::set<State> closedList;
		NodeType currentNode = new ComputeNode<State>(BeginState);

		NodeList.push_back(currentNode);
		closedList.emplace(currentNode->state);

		auto calculateCost = [&GetCost, &GetStateCost](NodeType n) {
			if (!n) return;
			NodeType current = n;
			n->G = 0.f;
			while (current) {
				n->G += GetStateCost(current->state, (int)current->actionID);
				current = current->prevNode;
			}
			n->Cost = GetCost(n->state, n->G);
		};

		auto transition = [&Act](NodeType currentState, const Action& act, size_t actionID) {

			State newState = Act(currentState->state, act);

			auto Node = new ComputeNode<State>(newState, actionID, currentState);
			return Node;
		};

		auto getNextLegalStates = [&ValidPreCheck, &ValidPostCheck, &transition, &Actions, &NodeList](NodeType n) -> std::vector<NodeType> {

			std::vector<NodeType> nextStates;
			nextStates.clear();
			nextStates.reserve(Actions.size());

			for (size_t act = 0; act < Actions.size(); act++) {
				if (!ValidPreCheck(n->state, Actions[act])) continue;
				auto newState = transition(n, Actions[act], act);
				if (newState && ValidPostCheck(newState->state)) {
					nextStates.push_back(newState);
					NodeList.push_back(newState);
				}
				else delete newState;
			}
			return nextStates;
		};

		while (currentNode != 0) {
			if (Visit(currentNode->state)) {
				for (auto child : getNextLegalStates(currentNode)) {
					if (!child) continue;

					calculateCost(child);

					if (closedList.find(child->state) != closedList.end()) {
						continue;
					}
					if (auto s = openList.find(child->state); s != openList.end()) {
						if (s->second->G > child->G) {
							openList[child->state] = child;
						}
					}
					else openList.emplace(child->state, child);
				}
			}
			else {
				break;
			}

			if (openList.empty()) {
				break;
			}

			currentNode = openList.begin()->second;
			float maxValue = currentNode->Cost;
			for (auto& [state, item] : openList) {
				float nVal = item->Cost;
				if (nVal < maxValue) {
					maxValue = nVal;
					currentNode = item;
				}
			}

			closedList.emplace(currentNode->state);
			openList.erase(currentNode->state);
		}

		closedList.clear();
		openList.clear();

		std::list<int> actionPath;

		while (currentNode && currentNode->prevNode) {
			actionPath.push_front((int)currentNode->actionID);
			currentNode = currentNode->prevNode;
		}

		for (auto& n : NodeList) {
			delete n;
		}
		NodeList.clear();

		return actionPath;
	}
}

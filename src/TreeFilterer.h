#ifndef Zadeh_ArrayFilterer_H
#define Zadeh_ArrayFilterer_H

#include "common.h"
#include "data_interface.h"
#include "options.h"
#include "filter.h"

namespace zadeh {


struct TreeNode {
    const CandidateString data;
    const size_t index;
    vector<size_t> parent_indices{};    // TODO use a pointer/reference

    explicit TreeNode(CandidateString &&data_, const size_t index_, vector<size_t> parent_indices_) noexcept
      : data{ move(data_) }, index{ index_ }, parent_indices{ parent_indices_ } {}
};


template<typename ArrayType, typename NodeType, typename IndexType = size_t, typename ReferenceType = ArrayType, typename AllocatorType = std::allocator<NodeType>>
class TreeFilterer {
  private:
    /* const */ string data_key = "data"s;
    /* const */ string children_key = "children"s;
    /** an array of the TreeNode which includes the data and its address (index, level) in the tree for each */
    vector<std::vector<CandidateString>> partitioned_candidates{};

    /** Should we keep a reference to the candidates. Set to `true` if you want to call `::filter` method */
    bool keepReference;
    /** Reference to the candidates used in `::filter` method */
    ReferenceType candidates_view;

  public:
    vector<TreeNode> candidates_vector;

    // default constructor is needed for generation of all the move/copy methods
    TreeFilterer() = default;

    explicit TreeFilterer(const string &data_key_, const string &children_key_)
      : data_key{ data_key_ },
        children_key{ children_key_ } {
    }

    /** create a Tree object and make an entries array */
    explicit TreeFilterer(const ArrayType &candidates_, const string &data_key_, const string &children_key_, const bool keepReference_ = true)
      : data_key{ data_key_ },
        children_key{ children_key_ } {
        set_candiates(candidates_, keepReference_);
    }

    auto set_candidates(const ArrayType &candidates_, const bool keepReference_ = true) {
        keepReference = keepReference_;
        make_candidates_vector(candidates_, vector<size_t>{}); /* consider the given array of trees the childs of an imaginary parent that has no index*/
        set_partitioned_candidates();


        if (keepReference) {
            // store a view of candidates in case filter was called
            candidates_view = get_ref<ReferenceType, NodeType>(candidates_);
        }
    }

    auto set_candidates(const ArrayType &candidates_, const string &data_key_, const string &children_key_, const bool keepReference_ = true) {
        keepReference = keepReference_;
        data_key = data_key_;
        children_key = children_key_;
        set_candidates(candidates_);
    }

    auto filter_indices(const std::string &query, const AllocatorType &env, const size_t maxResults = 0, const bool usePathScoring = true, const bool useExtensionBonus = false) {
        // optimization for no candidates
        if (partitioned_candidates.empty()) {
            return init<ArrayType, AllocatorType>(static_cast<size_t>(0), env);    // return an empty vector (should we throw?)
        }

        const Options options(query, maxResults, usePathScoring, useExtensionBonus);
        const auto filtered_indices = zadeh::filter(partitioned_candidates, query, options);
        const auto filter_indices_length = filtered_indices.size();

        auto res = init<ArrayType, AllocatorType>(filter_indices_length, env);    // array of TreeNode
        for (size_t i_candidate = 0; i_candidate < filter_indices_length; i_candidate++) {
            auto entry = candidates_vector[filtered_indices[i_candidate]];

            // create {data, index, level}
            auto node = init<NodeType, AllocatorType>(env);
            set_at(node, entry.data, "data"s);
            set_at(node, entry.index, "index"s);

            const auto parent_indices = entry.parent_indices;
            auto parent_indices_len = parent_indices.size();

            // copy vector<size_t> to ArrayType // TODO is this needed?
            auto parent_indices_array = init<ArrayType, AllocatorType>(parent_indices_len, env);
            for (uint32_t i_parent_indix = 0, parent_indices_len = parent_indices.size(); i_parent_indix < parent_indices_len; i_parent_indix++) {
                set_at(parent_indices_array, init<IndexType, AllocatorType>(parent_indices[i_parent_indix], env), i_parent_indix);
            }
            set_at(node, move(parent_indices_array), "parent_indices"s);
            set_at(res, move(node), i_candidate);
        }
        return res;
    }

    // auto filter(const std::string &query, const AllocatorType &env, const size_t maxResults = 0, const bool usePathScoring = true, const bool useExtensionBonus = false) {
    //     // optimization for no candidates
    //     if (partitioned_candidates.empty()) {
    //         return init<ArrayType, AllocatorType>(static_cast<size_t>(0), env);    // return an empty vector (should we throw?)
    //     }
    //
    //     const Options options(query, maxResults, usePathScoring, useExtensionBonus);
    //     const auto filtered_indices = zadeh::filter(partitioned_candidates, query, options);
    //     const auto filter_indices_length = filtered_indices.size();
    //
    //     auto res = init<ArrayType, AllocatorType>(filter_indices_length, env);    // array of candidate objects (with their address in index and level)
    //     auto candidates = candidates_view.Value();
    //     for (uint32_t i = 0; i < filter_indices_length; i++) {
    //         auto entry = candidates_vector[filtered_indices[i]];
    //
    //         // create {data, index, level}
    //         auto obj = init<NodeType, AllocatorType>(env);
    //         set_at(obj, entry.data, "data"s);
    //         set_at(obj, entry.index, "index"s);
    //         set_at(obj, entry.level, "level"s);
    //         res[i] = obj;
    //     }
    //     return res;
    // }

  private:
    /**
        Recursive function that fills the candidates_vector from the given children_nodes that have a common parent node
        @param children_nodes: an array of trees
        @param parent_indices: the indices of the parent node
    */
    void make_candidates_vector(const ArrayType &children_nodes, vector<size_t> parent_indices) {
        const auto children_num = get_size(children_nodes);
        for (auto i_child = 0u; i_child < children_num; i_child++) {
            make_candidates_vector(get_at<ArrayType, NodeType>(children_nodes, i_child), i_child, parent_indices);
        }
    }

    /**
        Recursive function that fills the candidates_vector from the given node
        @param node: a tree node
        @param index: the index of the child in the parent node
        @param parent_indices: the indices of the parent node
    */
    void make_candidates_vector(const NodeType &node, size_t index, vector<size_t> parent_indices) {
        // make the TreeNode and push it back
        candidates_vector.emplace_back(
          get_at<NodeType, CandidateString, string>(node, data_key),    // first, get the current data
          index,
          parent_indices);

        // add children if any
        auto may_children = get_children<NodeType, ArrayType>(node, children_key);
        if (may_children.has_value()) {
            // copy parent_indices and add the current index // TODO use a pointer?
            auto new_parent_indices = vector<size_t>();
            new_parent_indices = parent_indices;
            new_parent_indices.emplace_back(index);
            make_candidates_vector(may_children.value(), new_parent_indices);
        }
    }


    auto set_partitioned_candidates() {

        const auto N = candidates_vector.size();
        const auto num_chunks = get_num_chunks(N);


        partitioned_candidates.clear();
        partitioned_candidates.resize(num_chunks);

        auto cur_start = 0u;
        for (auto i = 0u; i < num_chunks; i++) {

            auto chunk_size = N / num_chunks;
            // Distribute remainder among the chunks.
            if (i < N % num_chunks) {
                chunk_size++;
            }
            for (auto j = cur_start; j < cur_start + chunk_size; j++) {
                partitioned_candidates[i].emplace_back(candidates_vector[j].data);    // different
            }
            cur_start += chunk_size;
        }
    }
};


}    // namespace zadeh
#endif

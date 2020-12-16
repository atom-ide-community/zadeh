#ifndef Fuzzaldrin_tree_h_
#define Fuzzaldrin_tree_h_

#include <optional>
#include "common.h"

/** Get the children of a jsTree (Napi::Object) */
inline std::optional<Napi::Array> getChildren(const Napi::Object &&jsTree, const string &&childrenKey) {
    // determine if it has children
    if (jsTree.HasOwnProperty(childrenKey)) {
        const auto childrenRaw = jsTree.Get(move_const(childrenKey));
        if (childrenRaw.IsArray()) {
            const auto childrenArray = childrenRaw.As<Napi::Array>();
            if (childrenArray.Length() != 0) {
                return childrenArray;
            }
        }
    }
    return {};
}


struct CandidateObject {
    const CandidateString data;
    const size_t level = 0;
    const size_t index = 0;

    explicit CandidateObject(const CandidateString &&data_, const size_t level_, const size_t index_) noexcept
      : data{ move_const(data_) }, level{ level_ }, index{ index_ } {}
};

struct Tree {
    /* const */ string dataKey = "data"s;
    /* const */ string childrenKey = "children"s;
    /** an array of the CandidateObject which includes the data and its address (index, level) in the tree for each */
    vector<CandidateObject> entriesArray;


    /** Recursive function that fills the entriesArray from the given jsTreeArray */
    void makeEntriesArray(const Napi::Array &&jsTreeArray, const size_t level) {
        const auto entriesArrayLength = jsTreeArray.Length();
        entriesArray.reserve(entriesArrayLength);// reserve enough space
        for (auto iEntry = 0u; iEntry < entriesArrayLength; iEntry++) {
            makeEntriesArray(jsTreeArray[iEntry].As<Napi::Object>(), level, iEntry);
        }
    }

    /** 1st argument is a single object */
    void makeEntriesArray(const Napi::Object &&jsTree, const size_t level, const size_t iEntry) {
        // make the CandidateObject and push it back
        entriesArray.emplace_back(
          jsTree.Get(dataKey).ToString().Utf8Value(),// first, get the current data
          level,
          iEntry

        );

        // add children if any
        const auto mayChildren = getChildren(move_const(jsTree), move_const(childrenKey));
        if (mayChildren.has_value()) {
            // recurse
            makeEntriesArray(move_const(mayChildren.value()), level + 1);
        }
    }

    // default constructor is needed for generation of all the move/copy methods
    explicit Tree() = default;

    /** create a Tree object and make an entries array */
    // NOTE: this is made to only accept Napi::Array because we cannot export templates to JavaScript
    explicit Tree(const Napi::Array &&jsTreeArrayOrObject_, const string &&dataKey_, const string &&childrenKey_)
      : dataKey{ move_const(dataKey_) },
        childrenKey{ move_const(childrenKey_) } {
        makeEntriesArray(move_const(jsTreeArrayOrObject_), 0);
    }

};

#endif// Fuzzaldrin_tree_h_

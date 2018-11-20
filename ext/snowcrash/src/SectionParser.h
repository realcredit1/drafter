//
//  SectionParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/4/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTIONPARSER_H
#define SNOWCRASH_SECTIONPARSER_H

#include <stdexcept>
#include <iostream>
#include "SignatureSectionProcessor.h"

namespace snowcrash
{

    /**
     *  Blueprint section parser
     */
    template <typename T, typename Adapter>
    struct SectionParser {

        /**
         *  \brief  Parse a section of blueprint
         *  \param  node    Initial node to start parsing at
         *  \param  siblings    Siblings of the initial node
         *  \param  pd      Parser data
         *  \param  T       Parsed output
         *  \return Iterator to the first unparsed block
         */
        static MarkdownNodeIterator parse(const MarkdownNodeIterator& node,
            const MarkdownNodes& siblings,
            SectionParserData& pd,
            const ParseResultRef<T>& out)
        {
            std::cerr << "WHERE: "                       //
                      << "<" << typeid(T).name() << ", " //
                      << typeid(Adapter).name() << ">::" //
                      << __func__ << std::endl;

            std::cerr << "SPXXX " << __LINE__ << std::endl;
            std::cerr << "siblings: " << siblings.size() << std::endl;
            std::cerr << "SPXXX " << __LINE__ << std::endl;

            std::cerr << "SPXXX " << __LINE__ << std::endl;
            siblings.end();
            std::cerr << "SPXXX " << __LINE__ << std::endl;

            SectionLayout layout = DefaultSectionLayout;
            MarkdownNodeIterator cur = Adapter::startingNode(node, pd);
            const MarkdownNodes& collection = Adapter::startingNodeSiblings(node, siblings);

            std::cerr << "SPXXX " << __LINE__ << std::endl;
            std::cerr << "collection: " << collection.size() << std::endl;
            std::cerr << "SPXXX " << __LINE__ << std::endl;

            // Signature node
            MarkdownNodeIterator lastCur = cur;

            std::cerr << "SPXXX " << __LINE__ << std::endl;

            cur = SectionProcessor<T>::processSignature(cur, collection, pd, layout, out);

            std::cerr << "SPXXX " << __LINE__ << std::endl;

            // Exclusive Nested Sections Layout
            if (layout == ExclusiveNestedSectionLayout) {

                cur = parseNestedSections(cur, collection, pd, out);

                SectionProcessor<T>::finalize(node, pd, out);

                std::cerr << "SPXXX " << __LINE__ << std::endl;

                return Adapter::nextStartingNode(node, siblings, cur);
            }

            // Parser redirect layout
            if (layout == RedirectSectionLayout) {
                SectionProcessor<T>::finalize(node, pd, out);

                std::cerr << "SPXXX " << __LINE__ << std::endl;

                return Adapter::nextStartingNode(node, siblings, cur);
            }

            // Default layout
            if (lastCur == cur) {

                std::cerr << "SPXXX " << __LINE__ << std::endl;

                return Adapter::nextStartingNode(node, siblings, cur);
            }

            std::cerr << "SPXXX " << __LINE__ << std::endl;

            // Description nodes
            while (cur != collection.end() && SectionProcessor<T>::isDescriptionNode(cur, pd.sectionContext())) {

                std::cerr << "SPXXX " << __LINE__ << std::endl;

                lastCur = cur;
                cur = SectionProcessor<T>::processDescription(cur, collection, pd, out);

                std::cerr << "SPXXX " << __LINE__ << std::endl;

                if (lastCur == cur)
                    return Adapter::nextStartingNode(node, siblings, cur);
            }

            // Content nodes
            while (cur != collection.end() && SectionProcessor<T>::isContentNode(cur, pd.sectionContext())) {

                std::cerr << "SPXXX " << __LINE__ << std::endl;

                lastCur = cur;
                cur = SectionProcessor<T>::processContent(cur, collection, pd, out);

                std::cerr << "SPXXX " << __LINE__ << std::endl;

                if (lastCur == cur) {

                    std::cerr << "SPXXX " << __LINE__ << std::endl;

                    return Adapter::nextStartingNode(node, siblings, cur);
                }
            }

            std::cerr << "SPXXX " << __LINE__ << std::endl;

            // Nested Sections
            cur = parseNestedSections(cur, collection, pd, out);

            std::cerr << "SPXXX " << __LINE__ << std::endl;

            SectionProcessor<T>::finalize(node, pd, out);

            std::cerr << "SPXXX " << __LINE__ << std::endl;

            return Adapter::nextStartingNode(node, siblings, cur);
        }

        /** Parse nested sections */
        static MarkdownNodeIterator parseNestedSections(const MarkdownNodeIterator& node,
            const MarkdownNodes& collection,
            SectionParserData& pd,
            const ParseResultRef<T>& out)
        {
            std::cerr << "SPXXX " << __LINE__ << std::endl;
            std::cerr << "collection: " << collection.size() << std::endl;
            std::cerr << "SPXXX " << __LINE__ << std::endl;

            MarkdownNodeIterator cur = node;
            MarkdownNodeIterator lastCur = cur;

            SectionType lastSectionType = UndefinedSectionType;

            SectionProcessor<T>::preprocessNestedSections(node, collection, pd, out);

            std::cerr << "SPXXX " << __LINE__ << std::endl;
            std::cerr << "cur: " << static_cast<int>(cur->type) << std::endl;
            std::cerr << "SPXXX " << __LINE__ << std::endl;

            // Nested sections
            while (cur != collection.end()) {
                std::cerr << "SPXXX " << __LINE__ << std::endl;

                lastCur = cur;
                SectionType nestedType = SectionProcessor<T>::nestedSectionType(cur);

                std::cerr << "SPXXX " << __LINE__ << std::endl;

                pd.sectionsContext.push_back(nestedType);

                std::cerr << "SPXXX " << __LINE__ << std::endl;
                std::cerr << "cur: " << static_cast<int>(cur->type) << std::endl;
                std::cerr << "SPXXX " << __LINE__ << std::endl;

                if (nestedType != UndefinedSectionType) {

                    cur = SectionProcessor<T>::processNestedSection(cur, collection, pd, out);
                    std::cerr << "SPXXX " << __LINE__ << std::endl;
                } else if (Adapter::nextSkipsUnexpected
                    || SectionProcessor<T>::isUnexpectedNode(cur, pd.sectionContext())) {

                    cur = SectionProcessor<T>::processUnexpectedNode(cur, collection, pd, lastSectionType, out);
                    std::cerr << "SPXXX " << __LINE__ << std::endl;
                }

                std::cerr << "SPXXX " << __LINE__ << std::endl;
                std::cerr << "cur: " << static_cast<int>(cur->type) << std::endl;
                std::cerr << "SPXXX " << __LINE__ << std::endl;
                std::cerr << "collection: " << collection.size() << std::endl;
                std::cerr << "SPXXX " << __LINE__ << std::endl;
                std::cerr << "pd: " << static_cast<int>(pd.sectionContext()) << std::endl;
                std::cerr << "SPXXX " << __LINE__ << std::endl;

                if (cur != collection.end()
                    && (pd.sectionContext() != UndefinedSectionType
                           || (cur->type != mdp::ParagraphMarkdownNodeType
                                  && cur->type != mdp::CodeMarkdownNodeType))) {

                    std::cerr << "SPXXX " << __LINE__ << std::endl;
                    lastSectionType = pd.sectionContext();
                    std::cerr << "SPXXX " << __LINE__ << std::endl;
                }

                std::cerr << "SPXXX " << __LINE__ << std::endl;
                pd.sectionsContext.pop_back();

                std::cerr << "SPXXX " << __LINE__ << std::endl;
                if (lastCur == cur)
                    break;
            }
            std::cerr << "SPXXX " << __LINE__ << std::endl;

            return cur;
        }
    };

    /** Parser Adapter for parsing header-defined sections */
    struct HeaderSectionAdapter {

        /** \return Node to start parsing with */
        static const MarkdownNodeIterator startingNode(const MarkdownNodeIterator& seed, const SectionParserData& pd)
        {
            if (seed->type != mdp::HeaderMarkdownNodeType) {
                // ERR: Expected header
                mdp::CharactersRangeSet sourceMap
                    = mdp::BytesRangeSetToCharactersRangeSet(seed->sourceMap, pd.sourceCharacterIndex);
                throw Error("expected header block, e.g. '# <text>'", BusinessError, sourceMap);
            }

            return seed;
        }

        /** \return Collection of siblings to starting Node */
        static const MarkdownNodes& startingNodeSiblings(const MarkdownNodeIterator&, const MarkdownNodes& siblings)
        {
            return siblings;
        }

        /** \return Starting node for next parsing */
        static const MarkdownNodeIterator nextStartingNode(
            const MarkdownNodeIterator&, const MarkdownNodes&, const MarkdownNodeIterator& cur)
        {
            return cur;
        }

        /**
         *  \brief Adapter Markdown node skipping behavior trait.
         *
         *  Adapter trait signalizing that the adapter can possibly skip some Markdown nodes on a nextStartingNode()
         * call.
         *  If set to true, a call to nextStartingNode() can skip some nodes causing some information loss. False
         * otherwise.
         */
        static constexpr bool nextSkipsUnexpected = false;
    };

    /** Parser Adapter for parsing list-defined sections */
    struct ListSectionAdapter {

        static const MarkdownNodeIterator startingNode(const MarkdownNodeIterator& seed, const SectionParserData& pd)
        {
            std::cerr << "SPXXX " << __LINE__ << std::endl;

            if (seed->type != mdp::ListItemMarkdownNodeType) {
                std::cerr << "SPXXX " << __LINE__ << std::endl;

                // ERR: Expected list item
                mdp::CharactersRangeSet sourceMap
                    = mdp::BytesRangeSetToCharactersRangeSet(seed->sourceMap, pd.sourceCharacterIndex);
                std::cerr << "SPXXX " << __LINE__ << std::endl;

                throw Error("expected list item block, e.g. '+ <text>'", BusinessError, sourceMap);
            }

            std::cerr << "SPXXX " << __LINE__ << std::endl;

            for (auto it = seed->children().begin(); it != seed->children().end(); ++it) {
                std::cerr << "SPXXX " << __LINE__ << std::endl;

                if (it->sourceMap.begin()->length != 0) {
                    std::cerr << "SPXXX " << __LINE__ << std::endl;

                    return it;
                }
            }

            std::cerr << "SPXXX " << __LINE__ << std::endl;

            // there are no parseable items, let it pass in old manner
            return seed->children().begin();
        }

        static const MarkdownNodes& startingNodeSiblings(const MarkdownNodeIterator& seed, const MarkdownNodes&)
        {
            return seed->children();
        }

        static const MarkdownNodeIterator nextStartingNode(
            const MarkdownNodeIterator& seed, const MarkdownNodes& siblings, const MarkdownNodeIterator&)
        {
            std::cerr << "SPXXX " << __LINE__ << std::endl;

            if (seed == siblings.end())
                return seed;

            std::cerr << "SPXXX " << __LINE__ << std::endl;

            auto result = ++MarkdownNodeIterator(seed);

            return result;
        }

        static constexpr bool nextSkipsUnexpected = true;
    };

    /** Parser Adapter for parsing blueprint sections */
    struct BlueprintSectionAdapter {

        /** \return Node to start parsing with */
        static const MarkdownNodeIterator startingNode(const MarkdownNodeIterator& seed, const SectionParserData& pd)
        {
            return seed;
        }

        /** \return Collection of siblings to starting Node */
        static const MarkdownNodes& startingNodeSiblings(
            const MarkdownNodeIterator& seed, const MarkdownNodes& siblings)
        {
            return siblings;
        }

        /** \return Starting node for next parsing */
        static const MarkdownNodeIterator nextStartingNode(
            const MarkdownNodeIterator& seed, const MarkdownNodes& siblings, const MarkdownNodeIterator& cur)
        {
            return cur;
        }

        static constexpr bool nextSkipsUnexpected = false;
    };
}

#endif

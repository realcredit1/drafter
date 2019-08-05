//
//  MSONTypeSectionParser.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 11/12/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONOneOfParser.h"
#include "MSONPropertyMemberParser.h"

namespace snowcrash
{

    /** Implementation of processNestedSection */
    MarkdownNodeIterator SectionProcessor<mson::TypeSection>::processNestedSection(const MarkdownNodeIterator& node,
        const MarkdownNodes& siblings,
        SectionParserData& pd,
        const ParseResultRef<mson::TypeSection>& out)
    {

        MarkdownNodeIterator cur = node;
        SectionType parentSectionType = pd.parentSectionContext();

        mson::MemberType element;
        SourceMap<mson::MemberType> elementSM;

        if (node->type == mdp::HeaderMarkdownNodeType) {
            return cur;
        }

        if (parentSectionType == MSONPropertyMembersSectionType || parentSectionType == MSONValueMembersSectionType) {

            switch (pd.sectionContext()) {
                case MSONMixinSectionType: {
                    IntermediateParseResult<mson::Mixin> mixin(out.report);
                    cur = MSONMixinParser::parse(node, siblings, pd, mixin);

                    element = mixin.node;

                    if (pd.exportSourceMap()) {
                        elementSM.mixin = mixin.sourceMap;
                    }

                    break;
                }

                case MSONOneOfSectionType: {
                    if (parentSectionType != MSONPropertyMembersSectionType) {

                        // WARN: One of can not be a nested member for a non object structure type
                        mdp::CharactersRangeSet sourceMap
                            = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                        out.report.warnings.push_back(
                            Warning("one-of can not be a nested member for a type not sub typed from object",
                                LogicalErrorWarning,
                                sourceMap));

                        return cur;
                    }

                    IntermediateParseResult<mson::OneOf> oneOf(out.report);
                    cur = MSONOneOfParser::parse(node, siblings, pd, oneOf);

                    element = mson::MemberType::OneOfSection{ oneOf.node };

                    if (pd.exportSourceMap()) {
                        elementSM = oneOf.sourceMap;
                    }

                    break;
                }

                case MSONSectionType: {
                    if (parentSectionType == MSONPropertyMembersSectionType) {

                        IntermediateParseResult<mson::PropertyMember> propertyMember(out.report);
                        cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                        element = propertyMember.node;

                        if (pd.exportSourceMap()) {
                            elementSM.property = propertyMember.sourceMap;
                        }
                    } else {

                        IntermediateParseResult<mson::ValueMember> valueMember(out.report);
                        cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                        element = valueMember.node;

                        if (pd.exportSourceMap()) {
                            elementSM.value = valueMember.sourceMap;
                        }
                    }

                    break;
                }

                default:
                    break;
            }
        } else if (parentSectionType == MSONSampleDefaultSectionType) {

            switch (pd.sectionContext()) {
                case MSONMixinSectionType:
                case MSONOneOfSectionType: {
                    // WARN: mixin and oneOf not supported in sample/default
                    std::stringstream ss;

                    ss << "sample and default type sections cannot have `" << SectionName(pd.sectionContext())
                       << "` type";

                    mdp::CharactersRangeSet sourceMap
                        = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                    out.report.warnings.push_back(Warning(ss.str(), LogicalErrorWarning, sourceMap));
                    break;
                }

                case MSONSectionType: {
                    if ((out.node.baseType == mson::ValueBaseType || out.node.baseType == mson::ImplicitValueBaseType)
                        && node->type == mdp::ListItemMarkdownNodeType) {

                        IntermediateParseResult<mson::ValueMember> valueMember(out.report);
                        cur = MSONValueMemberParser::parse(node, siblings, pd, valueMember);

                        element = valueMember.node;

                        if (pd.exportSourceMap()) {
                            elementSM.value = valueMember.sourceMap;
                        }
                    } else if ((out.node.baseType == mson::ObjectBaseType
                                   || out.node.baseType == mson::ImplicitObjectBaseType)
                        && node->type == mdp::ListItemMarkdownNodeType) {

                        IntermediateParseResult<mson::PropertyMember> propertyMember(out.report);
                        cur = MSONPropertyMemberParser::parse(node, siblings, pd, propertyMember);

                        element = propertyMember.node;

                        if (pd.exportSourceMap()) {
                            elementSM.property = propertyMember.sourceMap;
                        }
                    }

                    if (out.node.baseType == mson::PrimitiveBaseType
                        || out.node.baseType == mson::ImplicitPrimitiveBaseType) {

                        if (!out.node.content.value.empty()) {
                            TwoNewLines(out.node.content.value);
                        }

                        mdp::ByteBuffer content = mdp::MapBytesRangeSet(node->sourceMap, pd.sourceData);
                        out.node.content.value += content;

                        if (pd.exportSourceMap() && !content.empty()) {
                            out.sourceMap.value.sourceMap.append(node->sourceMap);
                        }

                        cur = ++MarkdownNodeIterator(node);
                    }

                    break;
                }

                default:
                    break;
            }
        }

        if (!element.empty()) {
            out.node.content.elements().push_back(element);

            if (pd.exportSourceMap()) {
                out.sourceMap.elements().collection.push_back(elementSM);
            }
        }

        return cur;
    }

    /** Implementation of nestedSectionType */
    SectionType SectionProcessor<mson::TypeSection>::nestedSectionType(const MarkdownNodeIterator& node)
    {

        SectionType nestedType = UndefinedSectionType;

        // Check if mson mixin section
        nestedType = SectionProcessor<mson::Mixin>::sectionType(node);

        if (nestedType != UndefinedSectionType) {
            return nestedType;
        }

        // Check if mson one of section
        nestedType = SectionProcessor<mson::OneOf>::sectionType(node);

        if (nestedType != UndefinedSectionType) {
            return nestedType;
        }

        return MSONSectionType;
    }

    MarkdownNodeIterator SectionProcessor<mson::TypeSection>::finalizeSignature(const MarkdownNodeIterator& node,
        SectionParserData& pd,
        const Signature& signature,
        const ParseResultRef<mson::TypeSection>& out)
    {

        bool assignValues = false;

        if (iequal(signature.identifier, "Default")) {

            out.node.klass = mson::TypeSection::DefaultClass;
            assignValues = true;
        } else if (iequal(signature.identifier, "Sample")) {

            out.node.klass = mson::TypeSection::SampleClass;
            assignValues = true;
        } else if (iequal(signature.identifier, "Items") || iequal(signature.identifier, "Members")) {

            if (out.node.baseType != mson::ValueBaseType && out.node.baseType != mson::ImplicitValueBaseType) {

                // WARN: Items/Members should only be allowed for value types
                std::stringstream ss;

                ss << "type section `" << signature.identifier;
                ss << "` not allowed for a type sub-typed from a primitive or object type";

                mdp::CharactersRangeSet sourceMap
                    = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(), LogicalErrorWarning, sourceMap));

                return node;
            }

            out.node.klass = mson::TypeSection::MemberTypeClass;
        } else if (iequal(signature.identifier, "Properties")) {

            if (out.node.baseType != mson::ObjectBaseType && out.node.baseType != mson::ImplicitObjectBaseType) {

                // WARN: Properties should only be allowed for object types
                std::stringstream ss;

                ss << "type section `" << signature.identifier;
                ss << "` is only allowed for a type sub-typed from an object type";

                mdp::CharactersRangeSet sourceMap
                    = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(Warning(ss.str(), LogicalErrorWarning, sourceMap));

                return node;
            }

            out.node.klass = mson::TypeSection::MemberTypeClass;
        }

        if (assignValues && (!signature.values.empty() || !signature.value.empty())) {

            if (out.node.baseType == mson::PrimitiveBaseType || out.node.baseType == mson::ImplicitPrimitiveBaseType) {

                out.node.content.value = signature.value;

                if (pd.exportSourceMap()) {
                    out.sourceMap.value.sourceMap = node->sourceMap;
                }
            } else if (out.node.baseType == mson::ValueBaseType || out.node.baseType == mson::ImplicitValueBaseType) {

                for (size_t i = 0; i < signature.values.size(); i++) {

                    mson::MemberType element;
                    SourceMap<mson::MemberType> elementSM;

                    {
                        mson::ValueMember valueMember;
                        valueMember.valueDefinition.values.emplace_back( //
                            mson::parseValue(signature.values[i]));
                        element = std::move(valueMember);
                    }

                    out.node.content.elements().push_back(element);

                    if (pd.exportSourceMap()) {

                        elementSM.value.valueDefinition.sourceMap = node->sourceMap;
                        out.sourceMap.elements().collection.push_back(elementSM);
                    }
                }
            } else if (out.node.baseType == mson::ObjectBaseType || out.node.baseType == mson::ImplicitObjectBaseType) {

                // WARN: sample/default is for an object but it has values in signature
                mdp::CharactersRangeSet sourceMap
                    = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceCharacterIndex);
                out.report.warnings.push_back(
                    Warning("a sample and/or default type section for a type which is sub-typed from an object "
                            "cannot have value(s) beside the keyword",
                        LogicalErrorWarning,
                        sourceMap));
            }
        }

        if (assignValues && !signature.remainingContent.empty()
            && (out.node.baseType == mson::PrimitiveBaseType || out.node.baseType == mson::ImplicitPrimitiveBaseType)) {

            out.node.content.value += signature.remainingContent;

            if (pd.exportSourceMap()) {
                out.sourceMap.value.sourceMap.append(node->sourceMap);
            }
        }

        return ++MarkdownNodeIterator(node);
    }
}

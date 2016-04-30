/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef BASE_JSON_CC
#define BASE_JSON_CC

#include "services/json.h"

#include <cassert>
#include <cctype>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <fstream>

#include <boost/format.hpp>

#include "services/lang.h"

using std::auto_ptr;
using std::string;

static const char* const jsonTypeName[] = {
    "object", "array", "string", "number", "bool", "null"
};

JsonNode::JsonNode(JsonType type)
    : type(type)
{}

bool JsonNode::Holds(JsonType type) const
{
    return this->type == type;
}

JsonObject* JsonNode::AsObject()
{
    CheckType(JSON_OBJECT);
    return static_cast<JsonObject*>(this);
}

const JsonObject* JsonNode::AsObject() const
{
    CheckType(JSON_OBJECT);
    return static_cast<const JsonObject*>(this);
}

JsonArray* JsonNode::AsArray()
{
    CheckType(JSON_ARRAY);
    return static_cast<JsonArray*>(this);
}

const JsonArray* JsonNode::AsArray() const
{
    CheckType(JSON_ARRAY);
    return static_cast<const JsonArray*>(this);
}

string JsonNode::AsString() const
{
    CheckType(JSON_STRING);
    return (static_cast<const JsonString*>(this))->Value();
}

int JsonNode::AsNumber() const
{
    CheckType(JSON_NUMBER);
    return (static_cast<const JsonNumber*>(this))->Value();
}

bool JsonNode::AsBool() const
{
    CheckType(JSON_BOOL);
    return (static_cast<const JsonBool*>(this))->Value();
}

void JsonNode::CheckType(JsonType requested) const
{
    if (!Holds(requested)) {
        throw TypeError(boost::str(boost::format("type mismatch: expected <%s>, found <%s>")
                                   %jsonTypeName[requested]
                                   %jsonTypeName[type]));
    }
}

JsonObject::JsonObject()
    : JsonNode(JSON_OBJECT)
{}

JsonObject::~JsonObject()
{
    for (NodeMap::iterator it = nodeMap.begin(); it != nodeMap.end(); ++it)
        delete it->second;
}

bool JsonObject::HasMember(const string& member) const
{
    return nodeMap.find(member) != nodeMap.end();
}

void JsonObject::Set(const string& member, JsonNode* value)
{
    if (HasMember(member)) {
        assert(nodeMap[member] != value);
        Remove(member);
    }
    nodeMap[member] = value;
}

void JsonObject::Set(const string& member, const char* value)
{
    Set(member, new JsonString(value));
}

void JsonObject::Set(const string& member, const string& value)
{
    Set(member, new JsonString(value));
}

void JsonObject::Set(const string& member, int value)
{
    Set(member, new JsonNumber(value));
}

void JsonObject::Set(const string& member, bool value)
{
    Set(member, new JsonBool(value));
}

void JsonObject::Remove(const string& member)
{
    NodeMap::iterator it = nodeMap.find(member);
    if (it != nodeMap.end()) {
        delete it->second;
        nodeMap.erase(it);
    }
}

const JsonNode* JsonObject::Get(const string& member) const
{
    NodeMap::const_iterator it = nodeMap.find(member);
    if (it == nodeMap.end())
        throw KeyError(member);
    return it->second;
}

JsonNode* JsonObject::Get(const string& member)
{
    NodeMap::iterator it = nodeMap.find(member);
    if (it == nodeMap.end())
        throw KeyError(member);
    return it->second;
}

void JsonObject::Serialize(string& result, bool indent, unsigned int indentWidth, unsigned int level)
{
    result.push_back('{');
    bool first = true;
    for (NodeMap::iterator it = nodeMap.begin(); it != nodeMap.end(); ++it) {
        if (!first)
            result.append(",");
        if (indent) {
            result.push_back('\n');
            result.append(string(level + indentWidth, ' '));
        } else {
            result.push_back(' ');
        }
        first = false;
        result.push_back('"');
        result.append(it->first);
        result.append("\": ");
        it->second->Serialize(result, indent, indentWidth, level + indentWidth);
    }
    if (indent && !nodeMap.empty()) {
        result.push_back('\n');
        result.append(string(level, ' '));
    }
    result.push_back('}');
}

JsonObject::iterator JsonObject::begin()
{
    return nodeMap.begin();
}

JsonObject::const_iterator JsonObject::begin() const
{
    return nodeMap.begin();
}

JsonObject::iterator JsonObject::end()
{
    return nodeMap.end();
}

JsonObject::const_iterator JsonObject::end() const
{
    return nodeMap.end();
}

JsonArray::JsonArray()
    : JsonNode(JSON_ARRAY)
{}

JsonArray::~JsonArray()
{
    for (NodeVector::iterator it = nodes.begin(); it != nodes.end(); ++it)
        delete *it;
}

void JsonArray::Add(JsonNode* element)
{
    nodes.push_back(element);
}

void JsonArray::Set(size_t index, JsonNode* element)
{
    delete Get(index);
    nodes[index] = element;
}

void JsonArray::Remove(size_t index)
{
    CheckIndex(index);
    delete nodes[index];
    nodes.erase(nodes.begin() + index);
}

void JsonArray::Pop()
{
    if (nodes.empty())
        throw std::underflow_error("pop from empty array");
    delete nodes.back();
    nodes.pop_back();
}

JsonNode* JsonArray::Get(size_t index)
{
    CheckIndex(index);
    return nodes[index];
}

const JsonNode* JsonArray::Get(size_t index) const
{
    CheckIndex(index);
    return nodes[index];
}

void JsonArray::Serialize(string& result, bool indent, unsigned int indentWidth, unsigned int level)
{
    result.push_back('[');
    bool first = true;
    for (NodeVector::iterator it = nodes.begin(); it != nodes.end(); ++it) {

        if (!first)
            result.push_back(',');
        if (indent) {
            result.push_back('\n');
            result.append(string(level + indentWidth, ' '));
        } else if (!first) {
            result.push_back(' ');
        }
        first = false;
        (*it)->Serialize(result, indent, indentWidth, level + indentWidth);
    }

    if (indent && !nodes.empty()) {
        result.push_back('\n');
        result.append(string(level, ' '));
    }
    result.push_back(']');
}

JsonArray::iterator JsonArray::begin()
{
    return nodes.begin();
}

JsonArray::const_iterator JsonArray::begin() const
{
    return nodes.begin();
}

JsonArray::iterator JsonArray::end()
{
    return nodes.end();
}

JsonArray::const_iterator JsonArray::end() const
{
    return nodes.end();
}

inline void JsonArray::CheckIndex(size_t index) const
{
    if (index >= Length())
        throw std::out_of_range("index out of range");
}

JsonString::JsonString(const string& value)
    : JsonNode(JSON_STRING),
      value(value)
{}

void JsonString::Serialize(string& result, bool indent, unsigned int indentWidth, unsigned int level)
{
    (void)indent, (void)indentWidth, (void)level;
    result.append('"' + Value() + '"');
}

JsonNumber::JsonNumber(int value)
    : JsonNode(JSON_NUMBER),
      value(value)
{}

void JsonNumber::Serialize(string& result, bool indent, unsigned int indentWidth, unsigned int level)
{
    (void)indent, (void)indentWidth, (void)level;
    std::stringstream stream;
    stream << Value();
    string temp;
    stream >> temp;
    result.append(temp);
}

JsonBool::JsonBool(bool value)
    : JsonNode(JSON_BOOL),
      value(value)
{}

void JsonBool::Serialize(string& result, bool indent, unsigned int indentWidth, unsigned int level)
{
    (void)indent, (void)indentWidth, (void)level;
    result.append(Value() ? "true" : "false");
}

JsonNull::JsonNull()
    : JsonNode(JSON_NULL)
{}

void JsonNull::Serialize(string& result, bool indent, unsigned int indentWidth, unsigned int level)
{
    (void)indent, (void)indentWidth, (void)level;
    result.append("null");
}

JsonNode* JsonParser::Parse(std::istream& stream)
{
    stream >> std::noskipws;
    input = std::istream_iterator<char>(stream);
    lookAhead = false;
    return ParseNode();
}

JsonNode* JsonParser::ParseNode()
{
    TokenType type = NextToken();
    switch (type) {
    case TOKEN_OBJECT_BEGIN:
        return ParseObject();
    case TOKEN_ARRAY_BEGIN:
        return ParseArray();
    case TOKEN_STRING:
        return new JsonString(token);
    case TOKEN_NUMBER:
        return new JsonNumber(strtol(token.c_str(), NULL, 10));
    case TOKEN_TRUE:
        return new JsonBool(true);
    case TOKEN_FALSE:
        return new JsonBool(false);
    case TOKEN_NULL:
        return new JsonNull();
    case TOKEN_NONE:
    default:
        throw SyntaxError();
    }
}

JsonNode* JsonParser::ParseObject()
{
    auto_ptr<JsonObject> object(new JsonObject);
    TokenType type;

    type = NextToken();
    while (type != TOKEN_OBJECT_END) {
        if (type != TOKEN_STRING)
            throw SyntaxError();
        string member = token;

        NextTokenChecked(TOKEN_COLON);
        JsonNode* value = ParseNode();
        object->Set(member, value);

        type = NextToken();
        if (type != TOKEN_COMMA && type != TOKEN_OBJECT_END)
            throw SyntaxError();
        if (type == TOKEN_COMMA)
            type = NextToken();
    }

    return object.release();
}

JsonNode* JsonParser::ParseArray()
{
    auto_ptr<JsonArray> array(new JsonArray);

    TokenType type = LookAhead();
    if (type == TOKEN_ARRAY_END)
        NextToken();

    while (type != TOKEN_ARRAY_END) {
        array->Add(ParseNode());
        type = NextToken();
        if (type != TOKEN_COMMA && type != TOKEN_ARRAY_END)
            throw SyntaxError();
    }

    return array.release();
}

JsonParser::TokenType JsonParser::NextToken()
{
    if (lookAhead) {
        lookAhead = false;
        return laTokenType;
    }

    token.clear();

    SkipWhitespace();

    if (input == eos)
        return TOKEN_NONE;

    switch (*input) {
    case '{':
        ++input;
        return TOKEN_OBJECT_BEGIN;
    case '}':
        ++input;
        return TOKEN_OBJECT_END;
    case '[':
        ++input;
        return TOKEN_ARRAY_BEGIN;
    case ']':
        ++input;
        return TOKEN_ARRAY_END;
    case ',':
        ++input;
        return TOKEN_COMMA;
    case ':':
        ++input;
        return TOKEN_COLON;
    case '"':
        return ReadString();
    case 'f':
        return ReadKeyword("false", TOKEN_FALSE);
    case 't':
        return ReadKeyword("true", TOKEN_TRUE);
    case 'n':
        return ReadKeyword("null", TOKEN_NULL);
    default:
        if (isdigit(*input))
            return ReadNumber();
        else
            return TOKEN_NONE;
    }
}

void JsonParser::NextTokenChecked(TokenType expectedType)
{
    if (NextToken() != expectedType)
        throw SyntaxError();
}

JsonParser::TokenType JsonParser::LookAhead()
{
    assert(!lookAhead);

    laTokenType = NextToken();
    lookAhead = true;
    return laTokenType;
}

void JsonParser::SkipWhitespace()
{
    while (input != eos && isspace(*input))
        ++input;
}

JsonParser::TokenType JsonParser::ReadString()
{
    ++input;

    while (input != eos) {
        if (*input == '"') {
            ++input;
            return TOKEN_STRING;
        }
        token.push_back(*input);
        ++input;
    }

    return TOKEN_NONE;
}

JsonParser::TokenType JsonParser::ReadKeyword(const string& keyword, TokenType tokenType)
{
    for (size_t i = 0; i < keyword.size(); i++) {
        if (input == eos || *input != keyword[i])
            return TOKEN_NONE;
        ++input;
    }
    return tokenType;
}

JsonParser::TokenType JsonParser::ReadNumber()
{
    assert(isdigit(*input));

    while (input != eos && isdigit(*input))
        token.push_back(*input++);

    return TOKEN_NUMBER;
}

#endif //BASE_JSON_CC

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

#ifndef BASE_JSON_H
#define BASE_JSON_H

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <iterator>
#include <iostream>

#include "services/lang.h"

enum JsonType {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER, 
    JSON_BOOL,
    JSON_NULL
};

class JsonObject;
class JsonArray;

class JsonNode {
public:
    struct JsonError : public std::runtime_error {
        JsonError(const std::string& what)
            : std::runtime_error(what)
        {}
    };

    struct TypeError : public JsonError {
        TypeError(const std::string& desc) : JsonError(desc) {}
    };

    virtual ~JsonNode() {}

    JsonType GetType() const { return type; }
    bool Holds(JsonType type) const;

    JsonObject* AsObject();
    const JsonObject* AsObject() const;

    JsonArray* AsArray();
    const JsonArray* AsArray() const;

    std::string AsString() const;
    int AsNumber() const;
    bool AsBool() const;

    virtual void Serialize(std::string& result,
                           bool         indent = false,
                           unsigned int indentWidth = 4,
                           unsigned int level = 0) = 0;

protected:
    JsonNode(JsonType type);

private:
    void CheckType(JsonType requested) const;

    JsonType type;

    DISABLE_COPY_AND_ASSIGNMENT(JsonNode);
};

class JsonObject : public JsonNode {
public:
    struct KeyError : public JsonError {
        KeyError(const std::string& key)
            : JsonError(key)
        {}
    };

    JsonObject();
    ~JsonObject();

    bool HasMember(const std::string& member) const;

    void Set(const std::string& member, JsonNode* value);
    void Set(const std::string& member, const char* value);
    void Set(const std::string& member, const std::string& value);
    void Set(const std::string& member, int value);
    void Set(const std::string& member, bool value);
    void Remove(const std::string& member);

    const JsonNode* Get(const std::string& member) const;
    JsonNode* Get(const std::string& member);

    size_t Size() const { return nodeMap.size(); }

    void Serialize(std::string& result,
                   bool         indent = false,
                   unsigned int indentWidth = 4,
                   unsigned int level = 0);

private:
    typedef std::map<std::string, JsonNode*> NodeMap;
    NodeMap nodeMap;

public:
    typedef NodeMap::iterator iterator;
    typedef NodeMap::const_iterator const_iterator;
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
};

class JsonArray : public JsonNode {
public:
    JsonArray();
    ~JsonArray();

    void Add(JsonNode* element);
    void Set(size_t index, JsonNode* element);

    void Remove(size_t index);
    void Pop();

    JsonNode* Get(size_t index);
    const JsonNode* Get(size_t index) const;

    size_t Length() const { return nodes.size(); }

    void Serialize(std::string& result,
                   bool         indent = false,
                   unsigned int indentWidth = 4,
                   unsigned int level = 0);

private:
    void CheckIndex(size_t index) const;

    typedef std::vector<JsonNode*> NodeVector;
    NodeVector nodes;

public:
    typedef NodeVector::iterator iterator;
    typedef NodeVector::const_iterator const_iterator;
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
};

class JsonString : public JsonNode {
public:
    JsonString(const std::string& value);
    std::string Value() const { return value; }
    void Serialize(std::string& result,
                   bool         indent = false,
                   unsigned int indentWidth = 4,
                   unsigned int level = 0);

private:
    std::string value;
};

class JsonNumber : public JsonNode {
public:
    JsonNumber(int value);
    int Value() const { return value; }
    void Serialize(std::string& result,
                   bool         indent = false,
                   unsigned int indentWidth = 4,
                   unsigned int level = 0);

private:
    int value;
};

class JsonBool : public JsonNode {
public:
    JsonBool(bool value);
    bool Value() const { return value; }
    void Serialize(std::string& result,
                   bool         indent = false,
                   unsigned int indentWidth = 4,
                   unsigned int level = 0);

private:
    bool value;
};

class JsonNull : public JsonNode {
public:
    JsonNull();
    void Serialize(std::string& result, bool indent = false, unsigned int level = 0);
    void Serialize(std::string& result,
                   bool         indent = false,
                   unsigned int indentWidth = 4,
                   unsigned int level = 0);
};

class JsonParser {
public:
    class SyntaxError {};

    JsonNode* Parse(std::istream& stream);

private:
    enum TokenType {
        TOKEN_NONE,
        TOKEN_OBJECT_BEGIN,
        TOKEN_OBJECT_END,
        TOKEN_ARRAY_BEGIN,
        TOKEN_ARRAY_END,
        TOKEN_COMMA,
        TOKEN_COLON,
        TOKEN_STRING,
        TOKEN_NUMBER,
        TOKEN_TRUE,
        TOKEN_FALSE,
        TOKEN_NULL
    };

    JsonNode* ParseNode();
    JsonNode* ParseObject();
    JsonNode* ParseArray();

    TokenType NextToken();
    void NextTokenChecked(TokenType expectedType);
    TokenType LookAhead();
    void SkipWhitespace();
    TokenType ReadString();
    TokenType ReadKeyword(const std::string& keyword, TokenType tokenType);
    TokenType ReadNumber();

    std::istream_iterator<char> input;
    std::istream_iterator<char> eos;

    std::string token;
    bool lookAhead;
    TokenType laTokenType;
};

#endif // BASE_JSON_H

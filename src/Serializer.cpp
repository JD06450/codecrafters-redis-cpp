#include "Serializer.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <stack>

#include "Parser.hpp"

namespace json = nlohmann;

std::vector<uint8_t> serialize_array(const json::json &obj)
{
	std::vector<uint8_t> output { (uint8_t) data_type_ids::ARRAY };
	std::string array_len = std::to_string(obj.size());
	output.insert(output.cend(), array_len.cbegin(), array_len.cend());

	output.push_back('\r');
	output.push_back('\n');

	return output;
}

std::vector<uint8_t> serialize_array_and_push(const json::json &obj, std::stack<json::json> &stack)
{
	for (auto it = obj.crbegin(); it != obj.crend(); ++it)
	{
		stack.push(*it);
	}
	return serialize_array(obj);
}

std::vector<uint8_t> serialize_blob(const json::json &obj)
{
	std::vector<uint8_t> output = { (uint8_t) data_type_ids::BLOB };
	std::string blob_len = std::to_string(obj.get_binary().size());
	output.insert(output.cend(), blob_len.cbegin(), blob_len.cend());

	output.push_back('\r');
	output.push_back('\n');
	
	std::vector<uint8_t> blob = obj.get_binary();
	output.insert(output.cend(), blob.cbegin(), blob.cend());

	output.push_back('\r');
	output.push_back('\n');

	return output;
}

std::vector<uint8_t> serialize_string(const json::json &obj)
{
	std::vector<uint8_t> output = { (uint8_t) data_type_ids::STRING };
	std::string str = obj.get<std::string>();
	output.insert(output.cend(), str.cbegin(), str.cend());

	output.push_back('\r');
	output.push_back('\n');

	return output;
}

std::vector<uint8_t> serialize_null(const json::json &obj)
{
	std::string raw_null = { (uint8_t) data_type_ids::BLOB };
	raw_null += "-1\r\n";
	return std::vector<uint8_t>(raw_null.cbegin(), raw_null.cend());
}

std::vector<uint8_t> from_json(const json::json &obj)
{
	std::vector<uint8_t> serialized_output;
	std::stack<json::json> parse_stack;
	parse_stack.push(obj);

	while (parse_stack.size() > 0)
	{
		json::json current_obj = parse_stack.top();
		std::vector<uint8_t> out = {};
		parse_stack.pop();
		if (current_obj.is_array())
			out = serialize_array_and_push(current_obj, parse_stack);
		else if (current_obj.is_binary())
			out = serialize_blob(current_obj);
		else if (current_obj.is_string())
			out = serialize_string(current_obj);
		else if (current_obj.is_null())
			out = serialize_null(current_obj);

		serialized_output.insert(serialized_output.cend(), out.cbegin(), out.cend());
	}

	return serialized_output;
}

/*
So to explain the loop, it might be best to show an example:

Take this for example:
[["1", ["2", "3"]], "Hello", ["4", "5"]]

when we start, we push the array itself into the stack.
from there, we start the loop, which pops the first item off the stack,
and because its an array, it will add all of the elements in the array
to the stack in reverse order, so the stack will look like this:

["4", "5"], "Hello", ["1", ["2, 3"]]

and as we process the root array, we add it to the output string,
so it would look like this:

*3\r\n

We then start the loop again, and pop the next element off of the stack.
Since it is another array, we will add it's elements to the stack in
reverse order, and also add the array header to the output.

["4", "5"], "Hello", ["2", "3"], "1"
(newlines and indents added for readability)
*3\r\n
	*2\r\n

Next we pop off the string "1" from the stack, and add it to the output.

["4", "5"], "Hello", ["2", "3"]
(newlines and indents added for readability)
*3\r\n
	*2\r\n
		+1\r\n (or $1\r\n1\r\n if it's a bulk string/blob)

This will continue until the stack is completely cleared, which signifies
that the entire object has been serialized.

The reason this method works is because each "object" only consists of
a header that identifies the object's type and/or length, followed by
the data itself.
*/
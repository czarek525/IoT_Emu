#pragma once
#include "headers.hpp"
namespace Helper_functions
{
    /**
     * @brief Retrieves a token from a string at a specified index.
     *
     * This function parses the input string and extracts the token at the specified index.
     * Tokens are separated by semicolons (`;`).
     *
     * @param input The input string containing tokens separated by semicolons.
     * @param index The zero-based index of the token to retrieve.
     * @return The token at the specified index, or an empty string if the index is out of range.
     */
    inline std::string getTokenAtIndex(const std::string &input, size_t index){
        std::istringstream iss(input);
        std::string token;
        for (size_t i = 0; i <= index; ++i)
        {
            if (!std::getline(iss, token, ';'))
                return "";
        }
        return token;
    }
    /**
     * @brief Removes leading and trailing braces from a string.
     *
     * This function removes the first and last characters of the input string if they are braces 
     * (`{` and `}`). The function modifies the input string in place. If the string is empty or 
     * does not start and end with braces, it remains unchanged.
     *
     * @param value The string from which braces will be removed.
     */
    inline void deleteBrace(std::string &value)
    {
        if (!value.empty())
        {
            value.erase(0, 1);
            value.erase(value.size() - 1);
        }
    }

    /**
     * @brief Retrieves an object from a unordered map container by name.
     *
     * @tparam T The type of the objects stored in the container.
     * @param container The map containing the objects, where the key is the object's name and 
     *        the value is a shared pointer to the object.
     * @param name The name of the object to retrieve.
     * @return A shared pointer to the object if found, or nullptr if the object with the given 
     *         name is not present in the container.
     */
    template <typename T>
    inline std::shared_ptr<T> getObjectByName(const std::unordered_map<std::string, std::shared_ptr<T>>& container, const std::string& name) {
        const auto it = container.find(name);
        if (it != container.end()) {
            return it->second;
        }
        return nullptr;
}
}
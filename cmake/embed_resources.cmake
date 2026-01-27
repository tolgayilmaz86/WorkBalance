# embed_resources.cmake
# This script embeds binary files as C++ byte arrays

function(embed_resource resource_file output_file variable_name)
    file(READ ${resource_file} file_content HEX)
    
    # Convert hex string to C++ array format
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," array_values ${file_content})
    
    # Get file size
    file(SIZE ${resource_file} file_size)
    
    # Generate C++ source file
    file(WRITE ${output_file}
        "// Auto-generated resource file\n"
        "#include <cstddef>\n\n"
        "extern const unsigned char ${variable_name}[] = {\n"
        "    ${array_values}\n"
        "};\n"
        "extern const size_t ${variable_name}_size = ${file_size};\n"
    )
endfunction()

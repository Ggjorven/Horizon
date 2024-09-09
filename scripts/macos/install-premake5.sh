#!/bin/bash

# Check if premake5 is installed
if ! command -v premake5 &> /dev/null; then
    echo "premake5 not found, installing..."

    # Download premake5
    curl -L https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-macosx.tar.gz -o premake5.tar.gz

    # Extract the tarball
    tar -xvf premake5.tar.gz

    # Find the extracted premake5 binary
    extracted_premake=$(tar -tf premake5.tar.gz | grep premake5)

    # Make the binary executable
    chmod +x $extracted_premake

    # Move the binary to /usr/local/bin
    sudo mv $extracted_premake /usr/local/bin/premake5

    # Clean up
    rm premake5.tar.gz

    rm libluasocket.dylib
    rm example.so

    echo "premake5 installed successfully."
else
    echo "premake5 is already installed."
fi

--ok now we need a function to check if the cpp and shader files already exist
function checkShaderFileConfig(name)
    local shaderPath = _SCRIPT_DIR .. "/assets/shaders/" .. name .. "/" .. name .. ".json"
    local shaderFile = io.open(shaderPath, "r")
if not shaderFile then
    --create the shader config file
    local shaderFile = io.open(shaderPath, "w")
    --replace every instance of <name> in the template with the name
    local shaderConfigTemplate = string.gsub(shaderConfigTemplate, "<name>", name)
    shaderFile:write(shaderConfigTemplate)
    shaderFile:close()
    end
end

--ok need another function for vertex and fragment shaders
function checkShaderFiles(name)
    local shaderPath = _SCRIPT_DIR .. "/assets/shaders/" .. name .. "/"
    local vertexShaderFile = io.open(shaderPath .. "default.vert", "r")
    local fragmentShaderFile = io.open(shaderPath .. "default.frag", "r")
    if not vertexShaderFile then
        --create the vertex shader file
        local vertexShaderFile = io.open(shaderPath .. "default.vert", "w")
        vertexShaderFile:write(vertexShaderTemplate)
        vertexShaderFile:close()
    end
    if not fragmentShaderFile then
        --create the fragment shader file
        local fragmentShaderFile = io.open(shaderPath .. "default.frag", "w")
        fragmentShaderFile:write(fragmentShaderTemplate)
        fragmentShaderFile:close()
    end
end

--ok now for source file
function checkSourceFile(name)
    local sourcePath = _SCRIPT_DIR .. "/examples/" .. name .. "/source/" .. name .. ".cpp"
    -- Check if the source file already exists
    local sourceFile = io.open(sourcePath, "r")
    if not sourceFile then
        -- File does not exist, create it
        sourceFile = io.open(sourcePath, "w")
        local sourceTemplate = string.gsub(sourceTemplate, "<name>", name)
        sourceFile:write(sourceTemplate)
        sourceFile:close()
    else
        sourceFile:close()
    end
end

--and now for the header file
function checkHeaderFile(name, parent)
    local headerPath = _SCRIPT_DIR .. "/examples/" .. name .. "/include/" .. name .. ".h"
    -- Only create the header file if it does not exist
    local headerFile = io.open(headerPath, "r")
    if not headerFile then
        headerFile = io.open(headerPath, "w")
        local headerTemplate = string.gsub(headerTemplate, "<name>", name)
        if parent then
            headerTemplate = string.gsub(headerTemplate, "<parent>", parent)
        else
            headerTemplate = string.gsub(headerTemplate, "<parent>", "scene")
        end
        headerFile:write(headerTemplate)
        headerFile:close()
    else
        headerFile:close()
    end
end

#include "stdafx.h"


namespace database
{

void initDataPaths()
{
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\sky");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\lib");   
}


bool LoadShaderInternal( const std::string & fileName, std::ostream & text )
{
    std::ifstream file(fileName.c_str());

    if (file.bad())
    {
        avError("Failed to load shader '%s'.", fileName.c_str());
        return false;
    }

    bool commentBlock = false;
    char szLineBuffer[1024];
    while (file.getline(szLineBuffer, sizeof(szLineBuffer)))
    {
        const char * pLineBuffer = szLineBuffer;

        // trim left
        while (*pLineBuffer == ' ')
            pLineBuffer++;

        //
        // Note: Not supported spaces after '#' and before 'include'
        //

        if (!commentBlock && strncmp(pLineBuffer, "#include", 8) == 0)
        {
            pLineBuffer += 8;
            const char * beginFileName = strchr(pLineBuffer, '"') + 1;
            const char * endFileName = strchr(beginFileName, '"');

            if (beginFileName == NULL || endFileName == NULL || beginFileName == endFileName)
            {
                avError("#include directive error in shader '%s' shader.", fileName.c_str());
                continue;
            }

            const std::string includeFileName(beginFileName, endFileName);
            std::string includeFullFileName = osgDB::findDataFile(includeFileName);
            
            if (!includeFullFileName.empty())
            {
                if (!LoadShaderInternal(includeFullFileName, text))
                {
                    avError("Failed to load shader '%s'.", fileName.c_str());
                    return false;
                }
            }
            else
            {
                avError("Failed to find shader include '%s'.", includeFileName.c_str());
                avError("Failed to load shader '%s'.", fileName.c_str());
                return false;
            }
        }
        else
        {
            //
            // Note: Not supported all cases when comment block closes before 
            //       #include directive on same text line.
            // /* some comment
            //    blah-blah-blah
            // */ #include "file.inl"
            //

            const char * pComment = pLineBuffer;
            while (pComment != NULL)
            {
                if (!commentBlock && (pComment = strstr(pComment, "/*")) != NULL)
                    commentBlock = true;
                else if (commentBlock && (pComment = strstr(pComment, "*/")) != NULL)
                    commentBlock = false;
            }

            text << szLineBuffer << std::endl;
        }
    }

    return true;
}

std::string LoadShader(const std::string& name)
{
    std::ostringstream shaderText;
    //  shaderText << cIt->first.second; // Add all defines
    if (!LoadShaderInternal(osgDB::findDataFile(name), shaderText))
        return "";
    shaderText << std::ends;

    return shaderText.str();
}

} // ns Database
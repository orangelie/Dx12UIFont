#pragma once

#include "Utils.h"

namespace orangelie
{
	namespace TextFont
	{
        struct FontType
        {
            float left, right;
            int size;
        };

        std::vector<FontType> LoadFontData(const char* filename);
	}
}

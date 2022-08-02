#include "Font.h"

namespace orangelie
{
	namespace TextFont
	{
        std::vector<FontType> LoadFontData(const char* filename)
        {
            std::ifstream fin;
            int i;
            char temp;


            std::vector<FontType> font(95);

            fin.open(filename);
            if (fin.fail())
            {
                return (std::vector<FontType>)0;
            }

            for (i = 0; i < 95; i++)
            {
                fin.get(temp);
                while (temp != ' ')
                {
                    fin.get(temp);
                }
                fin.get(temp);
                while (temp != ' ')
                {
                    fin.get(temp);
                }

                fin >> font[i].left;
                fin >> font[i].right;
                fin >> font[i].size;
            }

            fin.close();

            return font;
        }
	}
}

# Credit to https://patrickhenson.com/2018/06/07/uncrustify-configuration.html for recursive uncrustify command below
find . \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \) -exec uncrustify -lcpp ./.uncrustify.cfg --no-backup --replace {} +

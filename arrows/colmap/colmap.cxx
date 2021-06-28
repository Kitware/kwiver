#include <arrows/colmap/colmap.h>
#include <colmap/util/version.h>

/// Print the colmap version being used
void
kwiver::arrows::colmap
::version()
{
  std::cout << ::colmap::GetVersionInfo() << std::endl;
}

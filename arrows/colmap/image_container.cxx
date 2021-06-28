#include <arrows/colmap/image_container.h>
#include <colmap/util/version.h>

// TODO: Remove this once everything works
struct Color
{
  unsigned char r, g, b;
};

static void
write_to_img( kwiver::vital::image img, int u, int v, Color color )
{
  unsigned char* data = (unsigned char*) img.first_pixel();

  data[ img.w_step() * u + img.h_step() * v ] = color.r;
  data[ img.w_step() * u + img.h_step() * v + img.d_step() ] = color.g;
  data[ img.w_step() * u + img.h_step() * v + 2 * img.d_step() ] = color.b;
}

static Color
read_from_img( kwiver::vital::image img, int u, int v )
{
  unsigned char* data = (unsigned char*) img.first_pixel();

  return Color{
    data[ img.w_step() * u + img.h_step() * v ],
    data[ img.w_step() * u + img.h_step() * v + img.d_step() ],
    data[ img.w_step() * u + img.h_step() * v + 2 * img.d_step() ] };
}

colmap::Bitmap
kwiver::arrows::colmap_arrow::image_container
::vital_to_bitmap( kwiver::vital::image image )
{
  colmap::Bitmap new_img;
  new_img.Allocate( image.width(), image.height(), true );

  for( unsigned int i = 0; i < image.height(); i++ )
  {
    for( unsigned int j = 0; j < image.width(); j++ )
    {
      Color color = read_from_img( image, j, i );
      new_img.SetPixel( j, i,
                        colmap::BitmapColor< unsigned char >( color.r, color.g,
                                                              color.b ) );
    }
  }

  return new_img;
}

colmap::mvs::Image
kwiver::arrows::colmap_arrow::image_container
::vital_to_colmap( kwiver::vital::image& image )
{
  return colmap::mvs::Image();
}

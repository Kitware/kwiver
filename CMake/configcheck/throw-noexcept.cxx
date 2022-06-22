// the function f() does not throw
void f() noexcept;
// fp points to a function that may throwint main()
void ( * fp )() noexcept( false );

int
main()
{
  return 0;
}

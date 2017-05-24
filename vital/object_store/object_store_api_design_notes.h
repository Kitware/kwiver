///
/// This is basically a revised version of track_oracle's element_store.
///
///
/// Each managed object type has a dedicated templated object store singleton
/// within the sprokit process space.
///

///
/// Same handle type for all managed objects, regardless of type
/// Initialized to an invalid default value.
///

struct object_handle_t
{
  opaque_t handle;
  bool is_valid() const;
}

///
/// This struct contains the information about the backend, etc.
///

struct object_manager_backend_traits
{
  backend_enum_t backend_implementation; // memory, postgres, aws, etc...
  // ...other stuff
};

///
/// This typeless base class mostly exists to allow collections of the
/// concrete object_manager_stores.
///
struct object_manager_store_base
{
  // does this object exist?
  virtual bool exists( const object_handle_type& h ) = 0;

  // remove the object from the storage
  virtual void forget( const object_handle_type& h ) = 0;

  // persist the storage according to the context (could be a file,
  // a postgres dump, protobufs, etc.
  virtual bool persist( const persistence_context& c ) = 0;
  // load according to the context
  virtual bool restore( const persistence_context& c ) = 0;
};

///
/// the concrete object manager
///

template< classname T >
object_manager_store: public object_manager_store_base
{
  // TBD: how initialize() gets called exactly
  // ...probably in the pipeline initialization
  explicit object_manager_store::initialize( const object_manager_backend_traits& t );

  // add an object
  object_handle_t store( const T& val );

  // add an object by pointer (take ownership of the storage )
  object_handle_t store( const some_ptr_type< T >& ptr );

  // retrieve an instance of the object
  // throws if handle isn't valid
  T get( const object_handle_t& h );

  // true if h is valid
  bool exists( const object_handle_t& h );

  // all further get()s on h will throw, all calls to exist() return false
  // (unless the opaque_t gets recycled)
  void forget( const object_handle_t& h );
}


///
/// use cases
///


object_handle_t h_copy, h_ptr;

/// uninitialized handles are invalid
assert( ! h_copy.is_valid() );

/// let's make some objects!

{
  foo_t my_instance_of_foo( param_1, param_2, etc etc );
  object_handle_t h_copy = object_store_manager< foo_t >::store( my_instance_of_foo ); // copies
  smart_ptr<foo_t> another_foo = new foo_t( param_3, param_4 );
  object_handle_t h_ptr = object_store_manager< foo_t >::store( another_foo ); // doesn't copy
}

// time passes

{
  foo_t foo_inst_1 = object_store_manager< foo_t >::get( h_copy );
  // can do this
  foo_t foo_inst_2 = object_store_manager< foo_t >::get( h_ptr );
  // can't do this!
  smart_ptr<foo_t> foo_inst_ptr = object_store_manager< foo_t >::get( h_ptr );
  //
  // note that changes to foo_inst_1, foo_inst_2 are NOT reflected in the object_store
  //
}

// this throws to the extent that handles are unique
bar_t bar_instance = object_store_manager< bar_t >::get( h_copy );

// forget the handle
object_store_manager< foo_t >::forget( h_copy );

// now this throws
foo_t foo_inst_5 = object_store_manager< foo_t >::get( h_copy );


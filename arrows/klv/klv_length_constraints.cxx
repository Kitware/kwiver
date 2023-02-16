// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of KLV length constraints class.

#include <arrows/klv/klv_length_constraints.h>

#include <sstream>
#include <stdexcept>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_length_constraints
::klv_length_constraints()
{}

// ----------------------------------------------------------------------------
klv_length_constraints
::klv_length_constraints( size_t fixed_length ) : m_impl{ fixed_length }
{
  if( fixed_length == 0 )
  {
    throw std::logic_error{ "length constraints cannot include zero" };
  }
}

// ----------------------------------------------------------------------------
klv_length_constraints
::klv_length_constraints( size_t minimum, size_t maximum )
  : m_impl{ vital::interval< size_t >{ minimum, maximum } }
{
  if( minimum == 0 || maximum == 0 )
  {
    throw std::logic_error{ "length constraints cannot include zero" };
  }
  if( minimum == maximum )
  {
    throw std::logic_error{ "length constraints cannot exclude all lengths" };
  }
}

// ----------------------------------------------------------------------------
klv_length_constraints
::klv_length_constraints( size_t minimum, size_t maximum, size_t suggested )
  : m_impl{ vital::interval< size_t >{ minimum, maximum } },
    m_suggested{ suggested }
{
  if( minimum == 0 || maximum == 0 )
  {
    throw std::logic_error{ "length constraints cannot include zero" };
  }
  if( minimum == maximum )
  {
    throw std::logic_error{ "length constraints cannot exclude all lengths" };
  }
  set_suggested( suggested );
}

// ----------------------------------------------------------------------------
klv_length_constraints
::klv_length_constraints( std::set< size_t > const& allowed )
  : m_impl{ allowed }
{
  if( allowed.empty() )
  {
    throw std::logic_error{ "length constraints cannot exclude all lengths" };
  }
  if( allowed.count( 0 ) )
  {
    throw std::logic_error{ "length constraints cannot include zero" };
  }
}

// ----------------------------------------------------------------------------
klv_length_constraints
::klv_length_constraints( std::set< size_t > const& allowed, size_t suggested )
  : m_impl{ allowed }, m_suggested{ suggested }
{
  if( allowed.empty() )
  {
    throw std::logic_error{ "length constraints cannot exclude all lengths" };
  }
  if( allowed.count( 0 ) )
  {
    throw std::logic_error{ "length constraints cannot include zero" };
  }
  set_suggested( suggested );
}

// ----------------------------------------------------------------------------
bool
klv_length_constraints
::do_allow( size_t length ) const
{
  struct visitor
  {
    bool
    operator()( std::monostate ) const
    {
      return true;
    }

    bool
    operator()( size_t fixed_length ) const
    {
      return length == fixed_length;
    }

    bool
    operator()( vital::interval< size_t > const& interval ) const
    {
      return interval.contains( length, true, true );
    }

    bool
    operator()( std::set< size_t > const& set ) const
    {
      return set.count( length );
    }

    size_t length;
  };

  return std::visit( visitor{ length }, m_impl );
}

// ----------------------------------------------------------------------------
bool
klv_length_constraints
::is_free() const
{
  return std::holds_alternative< std::monostate >( m_impl );
}

// ----------------------------------------------------------------------------
std::optional< size_t >
klv_length_constraints
::fixed() const
{
  auto const result = std::get_if< size_t >( &m_impl );
  if( result )
  {
    return *result;
  }
  else
  {
    return std::nullopt;
  }
}

// ----------------------------------------------------------------------------
size_t
klv_length_constraints
::fixed_or( size_t backup ) const
{
  auto const result = std::get_if< size_t >( &m_impl );
  if( result )
  {
    return *result;
  }
  else
  {
    return backup;
  }
}

// ----------------------------------------------------------------------------
std::optional< vital::interval< size_t > >
klv_length_constraints
::interval() const
{
  auto const result = std::get_if< vital::interval< size_t > >( &m_impl );
  if( result )
  {
    return *result;
  }
  else
  {
    return std::nullopt;
  }
}

// ----------------------------------------------------------------------------
std::optional< std::set< size_t > >
klv_length_constraints
::set() const
{
  auto const result = std::get_if< std::set< size_t > >( &m_impl );
  if( result )
  {
    return *result;
  }
  else
  {
    return std::nullopt;
  }
}

// ----------------------------------------------------------------------------
size_t
klv_length_constraints
::suggested() const
{
  if( m_suggested )
  {
    return *m_suggested;
  }

  struct visitor
  {
    size_t
    operator()( std::monostate ) const
    {
      return 1;
    }

    size_t
    operator()( size_t fixed_length ) const
    {
      return fixed_length;
    }

    size_t
    operator()( vital::interval< size_t > const& interval ) const
    {
      return interval.lower();
    }

    size_t
    operator()( std::set< size_t > const& set ) const
    {
      return *set.begin();
    }
  };

  return std::visit( visitor{}, m_impl );
}

// ----------------------------------------------------------------------------
void
klv_length_constraints
::set_suggested( size_t length )
{
  if( length == 0 || !do_allow( length ) )
  {
    throw std::logic_error{
            "suggested length is not permitted by constraints" };
  }
  m_suggested = length;
}

// ----------------------------------------------------------------------------
std::string
klv_length_constraints
::description() const
{
  struct visitor
  {
    void
    operator()( std::monostate ) const
    {
      os << "unconstrained length";
    }

    void
    operator()( size_t fixed_length ) const
    {
      os << "exactly " << fixed_length << " bytes";
    }

    void
    operator()( vital::interval< size_t > const& interval ) const
    {
      os << "between " << interval.lower() << " and " << interval.upper()
         << " bytes";
    }

    void
    operator()( std::set< size_t > const& set ) const
    {
      os << "one of these lengths: ";
      for( auto const& entry : set )
      {
        os << entry;
        if( &entry != &*set.end() )
        {
          os << ", ";
        }
      }
    }

    std::ostream& os;
  };

  std::stringstream ss;
  std::visit( visitor{ ss }, m_impl );
  return ss.str();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver

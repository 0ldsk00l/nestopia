////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include <map>
#include "NstIoLog.hpp"
#include "NstIoStream.hpp"
#include "NstWindowUser.hpp"
#include "NstApplicationInstance.hpp"
#include "../core/NstXml.hpp"

namespace Nestopia
{
	namespace Application
	{
		struct Configuration::Node
		{
			Node();

			typedef Nes::Core::Xml Xml;

			Node& Get(const HeapString&);
			const Node* Find(cstring) const;

			void Load(Xml::Node);
			void Save(Xml::Node,wcstring=NULL) const;
			void CheckReference(wcstring) const;

			typedef HeapString Key;
			typedef std::multimap<Key,Node> Map;

			Map map;
			HeapString string;

			struct
			{
				Map* map;
				Map::iterator item;
			}   parent;

			mutable bool referenced;
		};

		Configuration::Configuration()
		: root(*new Node), save(false)
		{
			const Path path( Instance::GetExePath(L"nestopia.xml") );

			if (path.FileExists())
			{
				try
				{
					Nes::Core::Xml xml;

					{
						Io::Stream::In file( path );
						xml.Read( file );
					}

					if (!xml.GetRoot().IsType( L"configuration" ))
						throw 1;

					if (HeapString(Instance::GetVersion()) != xml.GetRoot().GetAttribute(L"version").GetValue())
						Window::User::Warn( L"The configuration file is old and may not work reliably with this version of Nestopia!", L"Configuration file version conflict!" );

					root.Load( xml.GetRoot().GetFirstChild() );
				}
				catch (...)
				{
					Reset( false );
					Window::User::Warn( L"Configuration file load error! Default settings will be used!" );
				}
			}

			if (wcstring ptr = ::GetCommandLine())
			{
				for (uint quote=0; (*ptr > ' ') || (*ptr && quote); ++ptr)
				{
					if (*ptr == '\"')
						quote ^= 1;
				}

				while (*ptr && *ptr <= ' ')
					++ptr;

				if (*ptr)
				{
					if (*ptr != '-')
					{
						wcstring const offset = ptr;

						for (uint quote=0; (*ptr > ' ') || (*ptr && quote); ++ptr)
						{
							if (*ptr == '\"')
								quote ^= 1;
						}

						startupFile.Assign( offset, ptr - offset );
						startupFile.Remove( '\"' );
						startupFile.Trim();

						// Win98/ME/2k fix
						if (startupFile.Length())
							startupFile = Instance::GetLongPath( startupFile.Ptr() );
					}
				}
			}
		}

		Configuration::~Configuration()
		{
			if (save)
			{
				try
				{
					Nes::Core::Xml xml;
					xml.Create( L"configuration" ).AddAttribute( L"version", HeapString(Instance::GetVersion()).Ptr() );
					root.Save( xml.GetRoot() );

					Io::Stream::Out file( Instance::GetExePath(L"nestopia.xml") );
					xml.Write( xml.GetRoot(), file );
				}
				catch (...)
				{
					Window::User::Warn( L"Couldn't save the configuration!" );
				}
			}

			delete &root;
		}

		void Configuration::Reset(bool notify)
		{
			if (notify)
				root.CheckReference( L"" );

			root.map.clear();
		}

		void Configuration::EnableSaving(bool enable)
		{
			save = enable;
		}

		const Path& Configuration::GetStartupFile() const
		{
			return startupFile;
		}

		Configuration::ConstSection Configuration::operator [] (cstring key) const
		{
			return root.Find( key );
		}

		Configuration::Section Configuration::operator [] (cstring key)
		{
			return &root.Get( key );
		}

		Configuration::ConstSection Configuration::ConstSection::operator [] (cstring key) const
		{
			return node ? node->Find( key ) : NULL;
		}

		Configuration::ConstSection Configuration::ConstSection::operator [] (uint i) const
		{
			if (node)
			{
				if (i)
				{
					Node::Map::iterator it( node->parent.item );

					while (++it != node->parent.map->end() && it->first == node->parent.item->first)
					{
						if (!--i)
						{
							it->second.referenced = true;
							return &it->second;
						}
					}
				}
				else
				{
					return *this;
				}
			}

			return NULL;
		}

		Configuration::Section Configuration::Section::operator [] (cstring key)
		{
			return &node->Get( key );
		}

		Configuration::Section Configuration::Section::operator [] (uint i)
		{
			if (i)
			{
				Node::Map::iterator it( node->parent.item );

				while (++it != node->parent.map->end() && it->first == node->parent.item->first)
				{
					if (!--i)
						return &it->second;
				}

				const Node::Map::value_type item( node->parent.item->first,Node() );

				do
				{
					it = node->parent.map->insert( it, item );
					it->second.parent.map = node->parent.map;
					it->second.parent.item = it;
				}
				while (--i);

				return &it->second;
			}
			else
			{
				return *this;
			}
		}

		HeapString& Configuration::Section::Str()
		{
			return node->string;
		}

		GenericString Configuration::ConstSection::Str() const
		{
			return node ? GenericString(node->string) : GenericString();
		}

		ulong Configuration::ConstSection::Int() const
		{
			ulong i;
			return node && (node->string >> i) ? i : 0;
		}

		ulong Configuration::ConstSection::Int(ulong d) const
		{
			ulong i;
			return node && (node->string >> i) ? i : d;
		}

		void Configuration::Section::IntProxy::operator = (ulong i)
		{
			node.string << i;
		}

		void Configuration::Section::YesNoProxy::operator = (bool yes)
		{
			node.string.Assign( yes ? L"yes" : L"no", yes ? 3 : 2 );
		}

		bool Configuration::ConstSection::Yes() const
		{
			return node && node->string == L"yes";
		}

		bool Configuration::ConstSection::No() const
		{
			return node && node->string == L"no";
		}

		Configuration::Node::Node()
		: referenced(false) {}

		Configuration::Node& Configuration::Node::Get(const HeapString& key)
		{
			Map::iterator it(map.lower_bound( key ));

			if (it == map.end() || it->first != key)
			{
				it = map.insert( it, Map::value_type(key,Node()) );
				it->second.parent.map = &map;
				it->second.parent.item = it;
			}

			return it->second;
		}

		const Configuration::Node* Configuration::Node::Find(cstring key) const
		{
			Map::const_iterator it(map.find( key ));

			if (it != map.end())
			{
				it->second.referenced = true;
				return &it->second;
			}

			return NULL;
		}

		void Configuration::Node::Load(Xml::Node xmlNode)
		{
			do
			{
				Map::iterator it(map.insert( Map::value_type(xmlNode.GetType(),Node()) ));

				it->second.parent.map = &map;
				it->second.parent.item = it;

				if (const Xml::Node xmlChild=xmlNode.GetFirstChild())
					it->second.Load( xmlChild );
				else
					it->second.string = xmlNode.GetValue();

				xmlNode = xmlNode.GetNextSibling();
			}
			while (xmlNode);
		}

		void Configuration::Node::Save(Xml::Node node,wcstring const key) const
		{
			NST_ASSERT( node && (!key || *key) );

			if (map.empty())
			{
				if (key)
					node.AddChild( key, string.Ptr() );
			}
			else
			{
				if (key)
					node = node.AddChild( key );

				for (Map::const_iterator it(map.begin()), end(map.end()); it != end; ++it)
					it->second.Save( node, it->first.Ptr() );
			}
		}

		void Configuration::Node::CheckReference(wcstring const key) const
		{
			NST_ASSERT( key );

			if (map.empty())
			{
				if (*key && !referenced)
					Io::Log() << "Configuration: warning, unused/invalid parameter: \"" << key << "\"\r\n";
			}
			else for (Map::const_iterator it(map.begin()), end(map.end()); it != end; ++it)
			{
				it->second.CheckReference( it->first.Ptr() );
			}
		}
	}
}

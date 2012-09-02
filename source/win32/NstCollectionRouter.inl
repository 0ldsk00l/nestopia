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

namespace Nestopia
{
	namespace Collection
	{
		template<typename Output,typename Input,typename Key> template<typename Data,typename Code>
		Router<Output,Input,Key>::Router(KeyParam key,Data* const data,Code code)
		: hooks(NULL)
		{
			Add( key, Callback(data,code) );
		}

		template<typename Output,typename Input,typename Key> template<typename Data,typename Array>
		Router<Output,Input,Key>::Router(Data* const data,const Array& arr)
		: hooks(NULL)
		{
			Add( data, arr, sizeof(array(arr)) );
		}

		template<typename Output,typename Input,typename Key>
		Router<Output,Input,Key>::~Router()
		{
			for (Hook* it = hooks; it; )
			{
				Hook* const him = it;
				it = it->next;
				delete him;
			}
		}

		template<typename Output,typename Input,typename Key>
		Output Router<Output,Input,Key>::Hook::Invoke(Input input)
		{
			for (uint i=0; i < items.Size(); ++i)
				items[i]( input );

			return main ? main( input ) : Output(0);
		}

		template<typename Output,typename Input,typename Key>
		void Router<Output,Input,Key>::Add(KeyParam key,const Callback& callback)
		{
			bool existing;
			Callback& item = items.GetSorted( key, existing ).callback;

			if (existing)
			{
				NST_ASSERT
				(
					hooks &&
					item.template CodePtr<Hook>() == &Hook::Invoke &&
					!item.template DataPtr<Hook>()->main
				);

				item.template DataPtr<Hook>()->main = callback;
			}
			else
			{
				item = callback;
			}
		}

		template<typename Output,typename Input,typename Key> template<typename Data>
		void Router<Output,Input,Key>::Add(Data* const data,const Entry<Data>* list,const uint count)
		{
			items.Reserve( items.Size() + count );

			for (const Entry<Data>* const end = list + count; list != end; ++list)
				Add( list->key, Callback(data,list->function) );
		}

		template<typename Output,typename Input,typename Key> template<typename Data>
		void Router<Output,Input,Key>::HookRouter::Add(Data* const data,const HookEntry<Data>* list,const uint count)
		{
			for (const HookEntry<Data>* const end = list + count; list != end; ++list)
				router.AddHook( list->key, Callback(data,list->function) );
		}

		template<typename Output,typename Input,typename Key>
		void Router<Output,Input,Key>::Set(KeyParam key,const Callback& callback)
		{
			if (Item* const item = items.FindSorted( key ))
			{
				if (item->callback.template CodePtr<Hook>() == &Hook::Invoke)
					item->callback.template DataPtr<Hook>()->main = callback;
				else
					item->callback = callback;
			}
			else
			{
				Add( key, callback );
			}
		}

		template<typename Output,typename Input,typename Key> template<typename Data>
		void Router<Output,Input,Key>::Set(Data* const data,const Entry<Data>* list,const uint count)
		{
			for (const Entry<Data>* const end = list + count; list != end; ++list)
				Set( list->key, Callback(data,list->function) );
		}

		template<typename Output,typename Input,typename Key>
		void Router<Output,Input,Key>::Remove(KeyParam key,const Callback& callback)
		{
			if (Item* const item = items.FindSorted( key ))
			{
				if (item->callback == callback)
				{
					items.Erase( item );
				}
				else if
				(
					item->callback.template CodePtr<Hook>() == &Hook::Invoke &&
					item->callback.template DataPtr<Hook>()->main == callback
				)
				{
					item->callback.template DataPtr<Hook>()->main.Reset();
				}
			}
		}

		template<typename Output,typename Input,typename Key>
		void Router<Output,Input,Key>::Remove(const void* const data)
		{
			for (uint i=0; i < items.Size(); )
			{
				const Callback& callback = items[i].callback;

				if (callback.VoidPtr() == data)
				{
					items.Erase( items.At(i) );
				}
				else
				{
					if
					(
						callback.template CodePtr<Hook>() == &Hook::Invoke &&
						callback.template DataPtr<Hook>()->main.VoidPtr() == data
					)
						callback.template DataPtr<Hook>()->main.Unset();

					++i;
				}
			}
		}

		template<typename Output,typename Input,typename Key>
		void Router<Output,Input,Key>::RemoveAll(const void* const data)
		{
			RemoveHooks( data );
			Remove( data );
		}

		template<typename Output,typename Input,typename Key>
		void Router<Output,Input,Key>::Defrag()
		{
			items.Defrag();
		}

		template<typename Output,typename Input,typename Key>
		void Router<Output,Input,Key>::AddHook(KeyParam key,const typename Hook::Item& newItem)
		{
			Hook* hook;

			bool existing;
			Callback& callback = items.GetSorted( key, existing ).callback;

			if (existing && callback.template CodePtr<Hook>() == &Hook::Invoke)
			{
				hook = callback.template DataPtr<Hook>();
				NST_ASSERT( hook && hook->items.Size() );
			}
			else
			{
				NST_ASSERT( !existing || callback );

				hook = new Hook;

				if (Hook* it = hooks)
				{
					while (it->next)
						it = it->next;

					it->next = hook;
				}
				else
				{
					hooks = hook;
				}

				if (existing)
					hook->main = callback;

				callback.Set( hook, &Hook::Invoke );
			}

			hook->items.PushBack( newItem );
		}

		template<typename Output,typename Input,typename Key>
		uint Router<Output,Input,Key>::RemoveHook(Item* const mainItem,Hook* const hook,typename Hook::Item* const hookItem)
		{
			NST_ASSERT( mainItem );

			hook->items.Erase( hookItem );

			if (hook->items.Size())
				return 0;

			uint result;

			if (hook->main)
			{
				result = 1;
				mainItem->callback = hook->main;
			}
			else
			{
				result = 2;
				items.Erase( mainItem );
			}

			if (hooks == hook)
			{
				hooks = hook->next;
			}
			else
			{
				Hook* it = hooks;

				while (it->next != hook)
					it = it->next;

				it->next = hook->next;
			}

			delete hook;
			return result;
		}

		template<typename Output,typename Input,typename Key>
		void Router<Output,Input,Key>::RemoveHook(KeyParam key,const typename Hook::Item& item)
		{
			if (Item* const mainItem = items.FindSorted( key ))
			{
				NST_ASSERT( mainItem->callback.template CodePtr<Hook>() == &Hook::Invoke );

				Hook* const hook = mainItem->callback.template DataPtr<Hook>();
				NST_ASSERT( hook );

				if (typename Hook::Item* const hookItem = hook->items.FindSorted( item ))
					RemoveHook( mainItem, hook, hookItem );
			}
		}

		template<typename Output,typename Input,typename Key>
		void Router<Output,Input,Key>::RemoveHooks(const void* const data)
		{
			for (uint i=0; i < items.Size(); ++i)
			{
				Item& mainItem = items[i];

				if (mainItem.callback.template CodePtr<Hook>() == &Hook::Invoke)
				{
					Hook* const hook = mainItem.callback.template DataPtr<Hook>();
					NST_ASSERT( hook );

					for (uint j=0; j < hook->items.Size(); )
					{
						typename Hook::Item& hookItem = hook->items[j];

						if (hookItem.VoidPtr() != data)
						{
							++j;
						}
						else if (const uint result = RemoveHook( &mainItem, hook, &hookItem ))
						{
							i -= (result == 2);
							break;
						}
					}
				}
			}
		}

		template<typename Output,typename Input,typename Key>
		typename Router<Output,Input,Key>::Callback& Router<Output,Input,Key>::operator [] (KeyParam key)
		{
			Callback& callback = items.AtSorted( key ).callback;

			if (callback.template CodePtr<Hook>() == &Hook::Invoke)
				return callback.template DataPtr<Hook>()->main;

			return callback;
		}

		template<typename Output,typename Input,typename Key>
		typename Router<Output,Input,Key>::Item& Router<Output,Input,Key>::Items::GetSorted(KeyParam key,bool& existing)
		{
			const uint pos = this->LowerBound( key );
			existing = (pos != this->Size() && this->At(pos)->key == key);

			if (!existing)
			{
				this->Insert( this->At(pos), NULL, 1 );
				new (static_cast<void*>(this->At(pos))) Item( key );
			}

			return *this->At(pos);
		}

		template<typename Output,typename Input,typename Key>
		typename Router<Output,Input,Key>::Item* Router<Output,Input,Key>::Items::FindSorted(KeyParam key)
		{
			const uint pos = this->LowerBound( key );
			return pos != this->Size() && this->At(pos)->key == key ? this->At(pos) : NULL;
		}

		template<typename Output,typename Input,typename Key>
		typename Router<Output,Input,Key>::Item& Router<Output,Input,Key>::Items::AtSorted(KeyParam key)
		{
			const uint pos = this->LowerBound( key );
			NST_ASSERT( pos < this->Size() && this->At(pos)->key == key );
			return *this->At(pos);
		}
	}
}

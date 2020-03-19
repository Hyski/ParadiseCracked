#if !defined(__LCRN_TREE_H_INCLUDED_7552370787048739__)
#define __LCRN_TREE_H_INCLUDED_7552370787048739__

#include <iterator>
#include <memory>

namespace mll
{

namespace utils
{

template<typename Item>
struct lcrn_tree_leaf;
template<typename Item, bool IsConst>
class lcrn_tree_iterator;

//=====================================================================================//
//                               class lcrn_tree_traits                                //
//=====================================================================================//
template<typename Item>
class lcrn_tree_traits
{
public:
	typedef Item value_type;
	typedef unsigned size_type;
	typedef value_type &reference;
	typedef lcrn_tree_iterator<value_type,false> iterator;
	typedef lcrn_tree_iterator<value_type,true> const_iterator;
	typedef lcrn_tree_leaf<value_type> leaf;

	template<bool IsConst> struct _iterator_traits {};

	template<> struct _iterator_traits<true>
	{
		typedef const leaf *leaf_ptr_type;
		typedef const value_type &return_reference_type;
		typedef const value_type *return_pointer_type;
		typedef const value_type &reference;
	};

	template<> struct _iterator_traits<false>
	{
		typedef leaf *leaf_ptr_type;
		typedef value_type &return_reference_type;
		typedef value_type *return_pointer_type;
		typedef value_type &reference;
	};
};

//=====================================================================================//
//                                   class lcrn_tree                                   //
//=====================================================================================//
template<typename Item>
class lcrn_tree : public lcrn_tree_traits<Item>
{
public:
	lcrn_tree();
	~lcrn_tree();

	// Возвращает итератор корня
	iterator begin();
	// Возвращает итератор конца дерева
	iterator end();

	// Возвращает итератор корня
	const_iterator begin() const;
	// Возвращает итератор конца дерева
	const_iterator end() const;

	// Добавляет дочерний лист к листу Node
	// Возвращает итератор на вставленный лист
	iterator insert_son(const iterator &node, const value_type &val);

	// Добавляет братский лист к листу Node
	// Возвращает итератор на вставленный лист
	iterator insert_brother(const iterator &node, const value_type &val);

	// Создает корень дерева
	// Возвращает итератор на корень
	iterator create_root(const value_type &Item);

	// Удаляет лист и всех его детей
	void erase(const iterator &Node);

	// Изменяет родителя node на new_parent
	void change_parent(const iterator &node, const iterator &new_parent);

	// Удаляет лист, а детей прицепляет к его родителю
	void cut(const iterator &Node);

	// Возвращает количество узлов
	size_type size() const {return m_Count;}

	// Возвращает true, в случае, если дерево пусто
	bool empty() const {return m_Count==0;}

	// Очищает содержимое дерева
	void clear();

	// Меняет местами содержимое this и tree
	lcrn_tree &swap(lcrn_tree &tree);

private:
	void kill_family(leaf *One);

	leaf *_tie(leaf *it, leaf *parent);
	void _untie(leaf *it);
	void _verify(leaf *it);

	leaf *_create_leaf(const value_type &val);

	leaf *m_Root;
	size_type m_Count;
};

//=====================================================================================//
//                              class lcrn_tree_iterator                               //
//=====================================================================================//
template<typename Item, bool IsConst>
class lcrn_tree_iterator
:	public lcrn_tree_traits<Item>,
	public std::iterator<std::forward_iterator_tag,Item>
{
	friend class lcrn_tree<Item>;
	typedef lcrn_tree_traits<Item> traits_type;

	typedef typename traits_type::_iterator_traits<IsConst>::leaf_ptr_type leaf_ptr_type;
	typedef typename traits_type::_iterator_traits<IsConst>::return_pointer_type return_pointer_type;
	typedef typename traits_type::_iterator_traits<IsConst>::return_reference_type return_reference_type;

	typedef lcrn_tree_iterator<Item,true> const_iterator;
	typedef lcrn_tree_iterator<Item,false> iterator;
	typedef lcrn_tree_iterator<Item,IsConst> this_iterator;

	friend lcrn_tree_iterator<Item,!IsConst>;

public:
	typedef typename traits_type::_iterator_traits<IsConst>::reference reference;

private:
	// Указатель на лист дерева
	leaf_ptr_type m_CurrentLeaf;
	int m_Level;

	bool _is_sentry() const { return _is_null() || m_CurrentLeaf->_is_sentry(); }
	bool _is_null() const { return !m_CurrentLeaf; }
	
public:
	typedef Item value_type;

public:
	lcrn_tree_iterator();
	lcrn_tree_iterator(const iterator &One);
	lcrn_tree_iterator(leaf_ptr_type Leaf, int Level);

#pragma warning(push)
#pragma warning(disable:4284) // return type for 'operator ->' is 'int'
							  // (ie; not a UDT or reference to a UDT.  Will produce errors
							  // if applied using infix notation)
	return_pointer_type operator ->() const {return &m_CurrentLeaf->_get();}
#pragma warning(pop)
	return_reference_type operator * () const {return  m_CurrentLeaf->_get();}

	bool has_son() const {return m_CurrentLeaf->m_Son!=0 && !m_CurrentLeaf->m_Son->_is_sentry();}
	bool has_brother() const {return m_CurrentLeaf->m_Brother!=0 && !m_CurrentLeaf->m_Brother->_is_sentry();}
	bool has_parent() const {return m_CurrentLeaf->m_Parent!=0;}
	bool is_root() const {return m_CurrentLeaf->m_Referencer==0;}

	size_type level() const {return m_Level;}

	lcrn_tree_iterator get_son() const;
	lcrn_tree_iterator get_brother() const;
	lcrn_tree_iterator get_parent() const;

	// Перейти к сыну
	lcrn_tree_iterator &son();
	// Перейти к брату
	lcrn_tree_iterator &brother();
	// Перейти к родителю
	lcrn_tree_iterator &parent();

	lcrn_tree_iterator son_begin() const { return get_son(); }
	lcrn_tree_iterator son_end() const { return lcrn_tree_iterator(m_CurrentLeaf->m_sonEnd,level()+1); }

	lcrn_tree_iterator operator++(int);
	lcrn_tree_iterator &operator++();

	// Операторы сравнения и присваивания
	bool operator == (const lcrn_tree_iterator &OtherOne) const;
	bool operator != (const lcrn_tree_iterator &OtherOne) const;
	lcrn_tree_iterator &operator = (const iterator &One);

};

//=====================================================================================//
//                                struct lcrn_tree_leaf                                //
//=====================================================================================//
template<typename Item>
struct lcrn_tree_leaf : public lcrn_tree_traits<Item>
{
	lcrn_tree_leaf()
	:	m_Referencer(0),
		m_Son(0),
		m_Brother(0),
		m_sonEnd(0),
		m_Parent(0) {}

	lcrn_tree_leaf(const value_type &item)
	:	m_Referencer(0),
		m_Son(0),
		m_Brother(0),
		m_Parent(0),
		m_sonEnd(0),
		m_Self(new value_type(item)) {}

	lcrn_tree_leaf *m_Son;					// Сын листа
	lcrn_tree_leaf *m_Brother;				// Брат листа
	lcrn_tree_leaf *m_Parent;				// Родитель листа
	lcrn_tree_leaf *m_sonEnd;				// Указатель на дочерний sentry
	lcrn_tree_leaf **m_Referencer;			// Указатель на указатель, ссылающийся на лист
	std::auto_ptr<value_type> m_Self;		// Собственно, данные

	void _untie_from_parent();
	void _tie_to_parent(lcrn_tree_leaf *parent);
	void _tie_to_root(lcrn_tree_leaf **root);
	void _add_child(lcrn_tree_leaf *child);

	bool _is_sentry() const { return !m_Self.get(); }

	value_type &_get() { return *m_Self.get(); }
	const value_type &_get() const { return *m_Self.get(); }
};

//=====================================================================================//
//                       lcrn_tree_leaf<Item>::_tie_to_parent()                        //
//=====================================================================================//
template<typename Item>
void lcrn_tree_leaf<Item>::_tie_to_parent(lcrn_tree_leaf *parent)
{
	// Инициализируем указатель на ссылающийся указатель
	m_Referencer = &parent->m_Son;
	m_Parent = parent;
	
	parent->_add_child(this);
}

//=====================================================================================//
//                        lcrn_tree_leaf<Item>::_tie_to_root()                         //
//=====================================================================================//
template<typename Item>
void lcrn_tree_leaf<Item>::_tie_to_root(lcrn_tree_leaf **root)
{
	m_Referencer = root;
	m_Parent = 0;
	m_Brother = *root;
	if(*root) (*root)->m_Referencer = &m_Brother;
}

//=====================================================================================//
//                         lcrn_tree_leaf<Item>::_add_child()                          //
//=====================================================================================//
template<typename Item>
void lcrn_tree_leaf<Item>::_add_child(lcrn_tree_leaf *child)
{
	if(m_Son)
	{
		m_Son->m_Referencer = &child->m_Brother;
		child->m_Brother = m_Son;
		m_Son = child;
	}
	else
	{
		m_Son = child;
	}
}

//=====================================================================================//
//                     lcrn_tree_leaf<Item>::_untie_from_parent()                      //
//=====================================================================================//
template<typename Item>
void lcrn_tree_leaf<Item>::_untie_from_parent()
{
	if(m_Referencer)
	{
		*m_Referencer = m_Brother;
	}

	if(m_Brother)
	{
		m_Brother->m_Referencer = m_Referencer;
		m_Brother = 0;
	}

	m_Parent = 0;
	m_Referencer = 0;
}

//=====================================================================================//
//                            lcrn_tree<Item>::lcrn_tree()                             //
//=====================================================================================//
template<typename Item>
lcrn_tree<Item>::lcrn_tree()
:	m_Root(new leaf),
	m_Count(0)
{
	m_Root->m_Referencer = &m_Root;
}

//=====================================================================================//
//                            lcrn_tree<Item>::~lcrn_tree()                            //
//=====================================================================================//
template<typename Item>
lcrn_tree<Item>::~lcrn_tree()
{
	clear();
	delete m_Root;
}

//=====================================================================================//
//                               lcrn_tree<Item>::_tie()                               //
//=====================================================================================//
template<typename Item>
typename lcrn_tree<Item>::leaf *lcrn_tree<Item>::_tie(leaf *it, leaf *parent)
{
	if(parent)
	{
		it->_tie_to_parent(parent);
	}
	else
	{
		it->_tie_to_root(&m_Root);
		m_Root = it;
	}

	return it;
}

//=====================================================================================//
//                              lcrn_tree<Item>::_untie()                              //
//=====================================================================================//
template<typename Item>
void lcrn_tree<Item>::_untie(leaf *it)
{
	it->_untie_from_parent();
}

//=====================================================================================//
//                            lcrn_tree<I>::_create_leaf()                             //
//=====================================================================================//
template<typename I>
typename lcrn_tree<I>::leaf *lcrn_tree<I>::_create_leaf(const value_type &val)
{
	leaf *nleaf = new leaf(val); ++m_Count;
	leaf *sentry = new leaf;
	_tie(sentry,nleaf);
	nleaf->m_sonEnd = sentry;
	return nleaf;
}

//=====================================================================================//
//                              lcrn_tree<Item>::begin()                               //
//=====================================================================================//
template<typename Item>
typename lcrn_tree<Item>::iterator lcrn_tree<Item>::begin()
{
	if(size())
	{
		return iterator(m_Root, 0);
	}
	else
	{
		return end();
	}
}

//=====================================================================================//
//                               lcrn_tree<Item>::end()                                //
//=====================================================================================//
template<typename Item>
typename lcrn_tree<Item>::iterator lcrn_tree<Item>::end()
{
	return iterator(0,-1);
}

//=====================================================================================//
//                              lcrn_tree<Item>::begin()                               //
//=====================================================================================//
template<typename Item>
typename lcrn_tree<Item>::const_iterator lcrn_tree<Item>::begin() const
{
	if(size())
	{
		return const_iterator(m_Root, 0);
	}
	else
	{
		return end();
	}
}

//=====================================================================================//
//                               lcrn_tree<Item>::end()                                //
//=====================================================================================//
template<typename Item>
typename lcrn_tree<Item>::const_iterator lcrn_tree<Item>::end() const
{
	return const_iterator(0,-1);
}

//=====================================================================================//
//                            lcrn_tree<Item>::insert_son()                            //
//=====================================================================================//
template<typename Item>
typename lcrn_tree<Item>::iterator lcrn_tree<Item>::insert_son(const iterator &node, const value_type &item)
{
	if(node._is_null())
	{
		return iterator(_tie(_create_leaf(item),0),node.level()+1);
	}
	else
	{
		return insert_brother(node.son_end(),item);
	}
//	return iterator(_tie(_create_leaf(item),node.m_CurrentLeaf),node.level()+1);
}

//=====================================================================================//
//                          lcrn_tree<Item>::insert_brother()                          //
//=====================================================================================//
template<typename Item>
typename lcrn_tree<Item>::iterator lcrn_tree<Item>::insert_brother(const iterator &node,
														  const value_type &item)
{
	if(node.m_CurrentLeaf->m_Referencer)
	{
		leaf *nleaf = _create_leaf(item);
		leaf *cleaf = node.m_CurrentLeaf;

		nleaf->m_Parent = node.m_CurrentLeaf->m_Parent;
		nleaf->m_Referencer = cleaf->m_Referencer;
		cleaf->m_Referencer = &nleaf->m_Brother;
		*nleaf->m_Referencer = nleaf;
		nleaf->m_Brother = cleaf;

		return iterator(nleaf,node.level());
	}
	else
	{
		return insert_son(end(),item);
	}
}

//=====================================================================================//
//                           lcrn_tree<Item>::create_root()                            //
//=====================================================================================//
template<typename Item>
typename lcrn_tree<Item>::iterator lcrn_tree<Item>::create_root(const value_type &item)
{
	return insert_son(end(),item);
//	leaf *nleaf = new leaf(item);
//	nleaf->_tie_to_root(m_Root);
//	return iterator(nleaf,0);
}

//=====================================================================================//
//                              lcrn_tree<Item>::erase()                               //
//=====================================================================================//
template<typename Item>
void lcrn_tree<Item>::erase(const iterator &node)
{
	if(node._is_sentry()) return;

	leaf *l = node.m_CurrentLeaf;
	_untie(l);

	if(l->m_Son) kill_family(l->m_Son);
	delete l; m_Count--;
}

//=====================================================================================//
//                               lcrn_tree<Item>::cut()                                //
//=====================================================================================//
template<typename Item>
void lcrn_tree<Item>::cut(const iterator &itor)
{
	if(itor._is_sentry()) return;

	leaf *l = itor.m_CurrentLeaf;
	leaf *parent = itor.m_CurrentLeaf->m_Parent;

	while(l->m_Son && !l->m_Son->_is_sentry())
	{
		leaf *son = l->m_Son;
		_untie(son);
		_tie(son,parent);
	}

	delete l->m_Son;
	_untie(l); --m_Count; delete l;
}

//=====================================================================================//
//                          lcrn_tree<Item>::change_parent()                           //
//=====================================================================================//
template<typename Item>
void lcrn_tree<Item>::change_parent(const iterator &node, const iterator &new_parent)
{
	if(node._is_sentry()) return;
	leaf *l = node.m_CurrentLeaf;
	_untie(l);
	_tie(l,new_parent.m_CurrentLeaf);
}

//=====================================================================================//
//                              lcrn_tree<Item>::clear()                               //
//=====================================================================================//
template<typename Item>
void lcrn_tree<Item>::clear()
{
	kill_family(m_Root);
	m_Root = new leaf;
}

//=====================================================================================//
//                           lcrn_tree<Item>::kill_family()                            //
//=====================================================================================//
template<typename Item>
void lcrn_tree<Item>::kill_family(leaf *One)
{
	leaf *current = One;

	while (current)
	{
		leaf *prev = current;
		if (current->m_Son) kill_family(current->m_Son);
		current = current->m_Brother;
		if(!prev->_is_sentry()) m_Count--;
		delete prev; 
	}
}

//=====================================================================================//
//                      lcrn_tree<Item> &lcrn_tree<Item>::swap()                       //
//=====================================================================================//
template<typename Item>
lcrn_tree<Item> &lcrn_tree<Item>::swap(lcrn_tree &tree)
{
	std::swap(m_Root,tree.m_Root);
	std::swap(m_Count,tree.m_Count);
	return *this;
}

//=====================================================================================//
//               lcrn_tree_iterator<Item,IsConst>::lcrn_tree_iterator()                //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst>::lcrn_tree_iterator()
:	m_CurrentLeaf(0), m_Level(0)
{
}

//=====================================================================================//
//               lcrn_tree_iterator<Item,IsConst>::lcrn_tree_iterator()                //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst>::lcrn_tree_iterator(const iterator &One)
:	m_CurrentLeaf(One.m_CurrentLeaf), m_Level(One.m_Level)
{
}

//=====================================================================================//
//               lcrn_tree_iterator<Item,IsConst>::lcrn_tree_iterator()                //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst>::lcrn_tree_iterator(leaf_ptr_type Leaf, int Level)
:	m_CurrentLeaf(Leaf), m_Level(Level)
{
}

//=====================================================================================//
//                     lcrn_tree_iterator<Item,IsConst>::get_son()                     //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst> lcrn_tree_iterator<Item,IsConst>::get_son() const
{
	return lcrn_tree_iterator(m_CurrentLeaf->m_Son, m_Level+1);
}

//=====================================================================================//
//                   lcrn_tree_iterator<Item,IsConst>::get_brother()                   //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst> lcrn_tree_iterator<Item,IsConst>::get_brother() const
{
	return lcrn_tree_iterator(m_CurrentLeaf->m_Brother, m_Level);
}

//=====================================================================================//
//                   lcrn_tree_iterator<Item,IsConst>::get_parent()                    //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst> lcrn_tree_iterator<Item,IsConst>::get_parent() const
{
	return lcrn_tree_iterator(m_CurrentLeaf->m_Parent, m_Level-1);
}

//=====================================================================================//
//                       lcrn_tree_iterator<Item,IsConst>::son()                       //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst> &lcrn_tree_iterator<Item,IsConst>::son()
{
	m_CurrentLeaf = m_CurrentLeaf->m_Son;
	m_Level++;
	return *this;
}

//=====================================================================================//
//                     lcrn_tree_iterator<Item,IsConst>::brother()                     //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst> &lcrn_tree_iterator<Item,IsConst>::brother()
{
	m_CurrentLeaf = m_CurrentLeaf->m_Brother; 
	return *this;
}

//=====================================================================================//
//                     lcrn_tree_iterator<Item,IsConst>::parent()                      //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst> &lcrn_tree_iterator<Item,IsConst>::parent()
{
	m_CurrentLeaf = m_CurrentLeaf->m_Parent;
	m_Level--;
	return *this;
}

//=====================================================================================//
//                   lcrn_tree_iterator<Item,IsConst>::operator++()                    //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst> lcrn_tree_iterator<Item,IsConst>::operator++(int)
{
	lcrn_tree_iterator tmp(*this);
	++(*this);
	return tmp;
}

//=====================================================================================//
//                   lcrn_tree_iterator<Item,IsConst>::operator++()                    //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst> &lcrn_tree_iterator<Item,IsConst>::operator++()
{
	// Если == end
	if(!m_CurrentLeaf) return *this;

	// Если есть сын, то перейти к сыну
	if(has_son()) return son();
	// Если есть брат, то перейти к брату
	else if(has_brother()) return brother();

	// Движемся к корню дерева, пока не найдем брата или дойдем до корня
	while(has_parent() && !has_brother()) parent();

	// Если есть брат, то шагаем к нему
	if(has_brother()) return brother();

	// Иначе - конец обхода
	m_CurrentLeaf = 0;
	return *this;
}

//=====================================================================================//
//                   lcrn_tree_iterator<Item,IsConst>::operator ==()                   //
//=====================================================================================//
template<typename Item, bool IsConst>
bool lcrn_tree_iterator<Item,IsConst>::operator ==(const lcrn_tree_iterator &OtherOne) const
{
	return (_is_sentry() && OtherOne._is_sentry()) ||
			m_CurrentLeaf == OtherOne.m_CurrentLeaf;
}

//=====================================================================================//
//                   lcrn_tree_iterator<Item,IsConst>::operator !=()                   //
//=====================================================================================//
template<typename Item, bool IsConst>
bool lcrn_tree_iterator<Item,IsConst>::operator !=(const lcrn_tree_iterator &OtherOne) const
{
	return !(*this == OtherOne);
}

//=====================================================================================//
//                   lcrn_tree_iterator<Item,IsConst>::operator =()                    //
//=====================================================================================//
template<typename Item, bool IsConst>
lcrn_tree_iterator<Item,IsConst> &lcrn_tree_iterator<Item,IsConst>::operator =(const iterator &One)
{
	m_CurrentLeaf = One.m_CurrentLeaf;
	m_Level = One.m_Level;
	return *this;
}

}	// namespace utils

}	// namespace mll

#endif // !defined(__LCRN_TREE_H_INCLUDED_7552370787048739__)
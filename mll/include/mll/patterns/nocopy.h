#if !defined(__NOCOPY_H_INCLUDED_7552370787048739__)
#define __NOCOPY_H_INCLUDED_7552370787048739__

namespace mll
{
namespace patterns
{

//=====================================================================================//
//                                    class nocopy                                     //
//=====================================================================================//
class nocopy
{
	nocopy(const nocopy &);
	nocopy &operator = (const nocopy &);
public:
	nocopy() {}
};

}
}

#endif // !defined(__NOCOPY_H_INCLUDED_7552370787048739__)
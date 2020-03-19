#if !defined(__SUB_FS_ITER_H_INCLUDED_3051904287262003__)
#define __SUB_FS_ITER_H_INCLUDED_3051904287262003__


#include "file_id.h"


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                  class sub_fs_iter                                  //
//=====================================================================================//
class sub_fs_iter
{	
public:
	virtual ~sub_fs_iter() = 0 {}
	
	/// Закончились ли файлы по указанному пути
	virtual bool is_done() = 0;
	/// Смещение на следующий файл
	virtual sub_fs_iter &operator++() = 0;
	/// Формирует объект file_obj из имеющейся информации о файле
	virtual file_id get_id() = 0;
};

}

}

#endif // !defined(__SUB_FS_ITER_H_INCLUDED_3051904287262003__)
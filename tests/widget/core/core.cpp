
#include "context/QT5/qt5_context.h"

namespace core {

context_ptr create_context()
{
    return singleton<Qt5Context>::instance();
}

}


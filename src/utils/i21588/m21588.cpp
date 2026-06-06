#include "i21588/m21588.h"
QVector<double> m21588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

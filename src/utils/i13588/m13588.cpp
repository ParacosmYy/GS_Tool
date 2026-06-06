#include "i13588/m13588.h"
QVector<double> m13588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

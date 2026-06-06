#include "i12828/m12828.h"
QVector<double> m12828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

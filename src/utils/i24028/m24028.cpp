#include "i24028/m24028.h"
QVector<double> m24028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

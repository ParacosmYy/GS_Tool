#include "e8324/m8324.h"
QVector<double> m8324::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

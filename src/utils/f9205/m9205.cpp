#include "f9205/m9205.h"
QVector<double> m9205::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

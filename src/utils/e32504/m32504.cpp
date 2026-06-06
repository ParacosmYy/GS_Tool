#include "e32504/m32504.h"
QVector<double> m32504::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

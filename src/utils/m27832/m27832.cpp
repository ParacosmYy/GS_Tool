#include "m27832/m27832.h"
QVector<double> m27832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

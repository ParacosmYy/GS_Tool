#include "m21832/m21832.h"
QVector<double> m21832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "m30012/m30012.h"
QVector<double> m30012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

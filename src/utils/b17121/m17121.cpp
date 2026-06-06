#include "b17121/m17121.h"
QVector<double> m17121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

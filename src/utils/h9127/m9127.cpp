#include "h9127/m9127.h"
QVector<double> m9127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

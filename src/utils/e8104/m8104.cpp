#include "e8104/m8104.h"
QVector<double> m8104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

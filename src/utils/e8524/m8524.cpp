#include "e8524/m8524.h"
QVector<double> m8524::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

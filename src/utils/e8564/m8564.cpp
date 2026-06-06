#include "e8564/m8564.h"
QVector<double> m8564::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "e7804/m7804.h"
QVector<double> m7804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "e9804/m9804.h"
QVector<double> m9804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

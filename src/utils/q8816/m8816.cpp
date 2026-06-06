#include "q8816/m8816.h"
QVector<double> m8816::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

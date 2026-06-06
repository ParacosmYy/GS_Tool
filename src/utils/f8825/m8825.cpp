#include "f8825/m8825.h"
QVector<double> m8825::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "f8725/m8725.h"
QVector<double> m8725::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

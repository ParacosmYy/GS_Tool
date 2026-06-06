#include "d8063/m8063.h"
QVector<double> m8063::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

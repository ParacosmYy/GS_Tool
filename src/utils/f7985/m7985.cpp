#include "f7985/m7985.h"
QVector<double> m7985::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

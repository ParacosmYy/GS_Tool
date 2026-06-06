#include "p8075/m8075.h"
QVector<double> m8075::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

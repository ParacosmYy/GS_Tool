#include "c8622/m8622.h"
QVector<double> m8622::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

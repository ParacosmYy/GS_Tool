#include "p32055/m32055.h"
QVector<double> m32055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

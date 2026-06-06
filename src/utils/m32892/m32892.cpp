#include "m32892/m32892.h"
QVector<double> m32892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

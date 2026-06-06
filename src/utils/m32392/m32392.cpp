#include "m32392/m32392.h"
QVector<double> m32392::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

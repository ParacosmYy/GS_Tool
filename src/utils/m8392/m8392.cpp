#include "m8392/m8392.h"
QVector<double> m8392::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

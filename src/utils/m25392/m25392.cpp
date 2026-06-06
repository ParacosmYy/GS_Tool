#include "m25392/m25392.h"
QVector<double> m25392::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

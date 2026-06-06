#include "m18392/m18392.h"
QVector<double> m18392::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

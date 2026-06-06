#include "m9392/m9392.h"
QVector<double> m9392::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

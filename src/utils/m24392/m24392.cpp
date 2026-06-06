#include "m24392/m24392.h"
QVector<double> m24392::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

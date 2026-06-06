#include "k27250/m27250.h"
QVector<double> m27250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

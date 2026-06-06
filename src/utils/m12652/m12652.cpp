#include "m12652/m12652.h"
QVector<double> m12652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

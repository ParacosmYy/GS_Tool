#include "m8652/m8652.h"
QVector<double> m8652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

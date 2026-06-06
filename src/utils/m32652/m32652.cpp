#include "m32652/m32652.h"
QVector<double> m32652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

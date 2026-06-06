#include "m13652/m13652.h"
QVector<double> m13652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

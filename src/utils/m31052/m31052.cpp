#include "m31052/m31052.h"
QVector<double> m31052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

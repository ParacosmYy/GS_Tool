#include "l18351/m18351.h"
QVector<double> m18351::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

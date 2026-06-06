#include "i29128/m29128.h"
QVector<double> m29128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

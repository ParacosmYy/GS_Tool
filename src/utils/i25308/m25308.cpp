#include "i25308/m25308.h"
QVector<double> m25308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

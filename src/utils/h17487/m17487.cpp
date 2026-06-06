#include "h17487/m17487.h"
QVector<double> m17487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

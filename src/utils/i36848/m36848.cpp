#include "i36848/m36848.h"
QVector<double> m36848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

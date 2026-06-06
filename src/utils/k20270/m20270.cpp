#include "k20270/m20270.h"
QVector<double> m20270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

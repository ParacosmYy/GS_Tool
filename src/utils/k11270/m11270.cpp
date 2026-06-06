#include "k11270/m11270.h"
QVector<double> m11270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

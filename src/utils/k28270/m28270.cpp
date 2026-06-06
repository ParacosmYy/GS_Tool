#include "k28270/m28270.h"
QVector<double> m28270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

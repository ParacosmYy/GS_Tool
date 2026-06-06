#include "g28306/m28306.h"
QVector<double> m28306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

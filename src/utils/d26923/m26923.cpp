#include "d26923/m26923.h"
QVector<double> m26923::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "a26000/m26000.h"
QVector<double> m26000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

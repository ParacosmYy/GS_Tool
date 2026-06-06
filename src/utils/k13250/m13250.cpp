#include "k13250/m13250.h"
QVector<double> m13250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

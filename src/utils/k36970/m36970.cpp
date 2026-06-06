#include "k36970/m36970.h"
QVector<double> m36970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

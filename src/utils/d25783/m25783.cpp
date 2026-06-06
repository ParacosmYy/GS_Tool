#include "d25783/m25783.h"
QVector<double> m25783::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

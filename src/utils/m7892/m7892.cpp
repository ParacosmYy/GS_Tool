#include "m7892/m7892.h"
QVector<double> m7892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "o24114/m24114.h"
QVector<double> m24114::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

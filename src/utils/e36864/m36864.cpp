#include "e36864/m36864.h"
QVector<double> m36864::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

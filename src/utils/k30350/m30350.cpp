#include "k30350/m30350.h"
QVector<double> m30350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

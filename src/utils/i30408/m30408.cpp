#include "i30408/m30408.h"
QVector<double> m30408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

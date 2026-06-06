#include "l37111/m37111.h"
QVector<double> m37111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

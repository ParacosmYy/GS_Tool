#include "t35039/m35039.h"
QVector<double> m35039::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

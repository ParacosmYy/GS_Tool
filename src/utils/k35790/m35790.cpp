#include "k35790/m35790.h"
QVector<double> m35790::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

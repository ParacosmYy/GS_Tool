#include "a25620/m25620.h"
QVector<double> m25620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

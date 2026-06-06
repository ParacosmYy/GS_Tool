#include "o35254/m35254.h"
QVector<double> m35254::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

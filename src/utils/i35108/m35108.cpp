#include "i35108/m35108.h"
QVector<double> m35108::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

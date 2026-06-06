#include "b35301/m35301.h"
QVector<double> m35301::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

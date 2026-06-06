#include "f25365/m25365.h"
QVector<double> m25365::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

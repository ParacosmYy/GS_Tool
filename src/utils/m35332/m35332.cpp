#include "m35332/m35332.h"
QVector<double> m35332::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

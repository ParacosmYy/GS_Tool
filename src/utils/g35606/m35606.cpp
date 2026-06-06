#include "g35606/m35606.h"
QVector<double> m35606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

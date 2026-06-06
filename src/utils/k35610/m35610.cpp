#include "k35610/m35610.h"
QVector<double> m35610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

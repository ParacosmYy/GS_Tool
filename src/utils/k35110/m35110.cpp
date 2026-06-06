#include "k35110/m35110.h"
QVector<double> m35110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

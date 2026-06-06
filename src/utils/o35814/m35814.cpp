#include "o35814/m35814.h"
QVector<double> m35814::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
